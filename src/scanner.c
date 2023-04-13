/*
 * Introduction
 * ************
 *
 * The following notes assume that you are familiar with the COOKLANG specification
 * (http://cooklang.org/spec/cvs/current.html).  We mostly follow it, although in
 * some cases we are less restrictive that it requires.
 *
 * The process of transforming a COOKLANG stream into a sequence of events is
 * divided on two steps: Scanning and Parsing.
 *
 * The Scanner transforms the input stream into a sequence of tokens, while the
 * parser transform the sequence of tokens produced by the Scanner into a
 * sequence of parsing events.
 *
 * The Scanner is rather clever and complicated. The Parser, on the contrary,
 * is a straightforward implementation of a recursive-descendant parser (or,
 * LL(1) parser, as it is usually called).
 *
 * Actually there are two issues of Scanning that might be called "clever", the
 * rest is quite straightforward.  The issues are "block collection start" and
 * "simple keys".  Both issues are explained below in details.
 *
 * Here the Scanning step is explained and implemented.  We start with the list
 * of all the tokens produced by the Scanner together with short descriptions.
 *
 * Now, tokens:
 *
 *      STREAM-START(encoding)          # The stream start.
 *      STREAM-END                      # The stream end.
 *      VERSION-DIRECTIVE(major,minor)  # The '%COOKLANG' directive.
 *      TAG-DIRECTIVE(handle,prefix)    # The '%TAG' directive.
 *      DOCUMENT-START                  # '---'
 *      DOCUMENT-END                    # '...'
 *      BLOCK-SEQUENCE-START            # Indentation increase denoting a block
 *      BLOCK-MAPPING-START             # sequence or a block mapping.
 *      BLOCK-END                       # Indentation decrease.
 *      FLOW-SEQUENCE-START             # '['
 *      FLOW-SEQUENCE-END               # ']'
 *      FLOW-MAPPING-START              # '{'
 *      FLOW-MAPPING-END                # '}'
 *      BLOCK-ENTRY                     # '-'
 *      FLOW-ENTRY                      # ','
 *      KEY                             # '?' or nothing (simple keys).
 *      VALUE                           # ':'
 *      ALIAS(anchor)                   # '*anchor'
 *      ANCHOR(anchor)                  # '&anchor'
 *      TAG(handle,suffix)              # '!handle!suffix'
 *      SCALAR(value,style)             # A scalar.
 *
 * The following two tokens are "virtual" tokens denoting the beginning and the
 * end of the stream:
 *
 *      STREAM-START(encoding)
 *      STREAM-END
 *
 * We pass the information about the input stream encoding with the
 * STREAM-START token.
 *
 */

#include "cooklang_private.h"

/*
 * Ensure that the buffer contains the required number of characters.
 * Return 1 on success, 0 on failure (reader error or memory error).
 */

#define CACHE(parser,length)                                                    \
    (parser->unread >= (length)                                                 \
        ? 1                                                                     \
        : cooklang_parser_update_buffer(parser, (length)))

/*
 * Advance the buffer pointer.
 */

#define SKIP(parser)                                                            \
     (parser->mark.index ++,                                                    \
      parser->mark.column ++,                                                   \
      parser->unread --,                                                        \
      parser->buffer.pointer += WIDTH(parser->buffer))

#define SKIP_LINE(parser)                                                       \
     (IS_CRLF(parser->buffer) ?                                                 \
      (parser->mark.index += 2,                                                 \
       parser->mark.column = 0,                                                 \
       parser->mark.line ++,                                                    \
       parser->unread -= 2,                                                     \
       parser->buffer.pointer += 2) :                                           \
      IS_BREAK(parser->buffer) ?                                                \
      (parser->mark.index ++,                                                   \
       parser->mark.column = 0,                                                 \
       parser->mark.line ++,                                                    \
       parser->unread --,                                                       \
       parser->buffer.pointer += WIDTH(parser->buffer)) : 0)

/*
 * Copy a character to a string buffer and advance pointers.
 */

#define READ(parser,string)                                                     \
     (STRING_EXTEND(parser,string) ?                                            \
         (COPY(string,parser->buffer),                                          \
          parser->mark.index ++,                                                \
          parser->mark.column ++,                                               \
          parser->unread --,                                                    \
          1) : 0)

/*
 * Copy a line break character to a string buffer and advance pointers.
 */

#define READ_LINE(parser,string)                                                \
    (STRING_EXTEND(parser,string) ?                                             \
    (((CHECK_AT(parser->buffer,'\r',0)                                          \
       && CHECK_AT(parser->buffer,'\n',1)) ?        /* CR LF -> LF */           \
     (*((string).pointer++) = (cooklang_char_t) '\n',                               \
      parser->buffer.pointer += 2,                                              \
      parser->mark.index += 2,                                                  \
      parser->mark.column = 0,                                                  \
      parser->mark.line ++,                                                     \
      parser->unread -= 2) :                                                    \
     (CHECK_AT(parser->buffer,'\r',0)                                           \
      || CHECK_AT(parser->buffer,'\n',0)) ?         /* CR|LF -> LF */           \
     (*((string).pointer++) = (cooklang_char_t) '\n',                               \
      parser->buffer.pointer ++,                                                \
      parser->mark.index ++,                                                    \
      parser->mark.column = 0,                                                  \
      parser->mark.line ++,                                                     \
      parser->unread --) :                                                      \
     (CHECK_AT(parser->buffer,'\xC2',0)                                         \
      && CHECK_AT(parser->buffer,'\x85',1)) ?       /* NEL -> LF */             \
     (*((string).pointer++) = (cooklang_char_t) '\n',                               \
      parser->buffer.pointer += 2,                                              \
      parser->mark.index ++,                                                    \
      parser->mark.column = 0,                                                  \
      parser->mark.line ++,                                                     \
      parser->unread --) :                                                      \
     (CHECK_AT(parser->buffer,'\xE2',0) &&                                      \
      CHECK_AT(parser->buffer,'\x80',1) &&                                      \
      (CHECK_AT(parser->buffer,'\xA8',2) ||                                     \
       CHECK_AT(parser->buffer,'\xA9',2))) ?        /* LS|PS -> LS|PS */        \
     (*((string).pointer++) = *(parser->buffer.pointer++),                      \
      *((string).pointer++) = *(parser->buffer.pointer++),                      \
      *((string).pointer++) = *(parser->buffer.pointer++),                      \
      parser->mark.index ++,                                                    \
      parser->mark.column = 0,                                                  \
      parser->mark.line ++,                                                     \
      parser->unread --) : 0),                                                  \
    1) : 0)

/*
 * Public API declarations.
 */

COOKLANG_DECLARE(int)
cooklang_parser_scan(cooklang_parser_t *parser, cooklang_token_t *token);

/*
 * Error handling.
 */

static int
cooklang_parser_set_scanner_error(cooklang_parser_t *parser, const char *context,
        cooklang_mark_t context_mark, const char *problem);

/*
 * High-level token API.
 */

COOKLANG_DECLARE(int)
cooklang_parser_fetch_more_tokens(cooklang_parser_t *parser);

static int
cooklang_parser_fetch_next_token(cooklang_parser_t *parser);


/*
 * Token fetchers.
 */

static int
cooklang_parser_fetch_stream_start(cooklang_parser_t *parser);

static int
cooklang_parser_fetch_stream_end(cooklang_parser_t *parser);


/*
 * Token scanners.
 */

static int
cooklang_parser_scan_to_next_token(cooklang_parser_t *parser);

/*
 * Get the next token.
 */

COOKLANG_DECLARE(int)
cooklang_parser_scan(cooklang_parser_t *parser, cooklang_token_t *token)
{
    assert(parser); /* Non-NULL parser object is expected. */
    assert(token);  /* Non-NULL token object is expected. */

    /* Erase the token object. */

    memset(token, 0, sizeof(cooklang_token_t));

    /* No tokens after STREAM-END or error. */

    if (parser->stream_end_produced || parser->error) {
        return 1;
    }

    /* Ensure that the tokens queue contains enough tokens. */

    if (!parser->token_available) {
        if (!cooklang_parser_fetch_more_tokens(parser))
            return 0;
    }

    /* Fetch the next token from the queue. */

    *token = DEQUEUE(parser, parser->tokens);
    parser->token_available = 0;
    parser->tokens_parsed ++;

    if (token->type == COOKLANG_STREAM_END_TOKEN) {
        parser->stream_end_produced = 1;
    }

    return 1;
}

/*
 * Set the scanner error and return 0.
 */

static int
cooklang_parser_set_scanner_error(cooklang_parser_t *parser, const char *context,
        cooklang_mark_t context_mark, const char *problem)
{
    parser->error = COOKLANG_SCANNER_ERROR;
    parser->context = context;
    parser->context_mark = context_mark;
    parser->problem = problem;
    parser->problem_mark = parser->mark;

    return 0;
}

/*
 * Ensure that the tokens queue contains at least one token which can be
 * returned to the Parser.
 */

COOKLANG_DECLARE(int)
cooklang_parser_fetch_more_tokens(cooklang_parser_t *parser)
{
    int need_more_tokens;

    /* While we need more tokens to fetch, do it. */

    while (1)
    {
        /*
         * Check if we really need to fetch more tokens.
         */

        need_more_tokens = 0;

        if (parser->tokens.head == parser->tokens.tail)
        {
            /* Queue is empty. */

            need_more_tokens = 1;
        }

        /* We are finished. */

        if (!need_more_tokens)
            break;

        /* Fetch the next token. */

        if (!cooklang_parser_fetch_next_token(parser))
            return 0;
    }

    parser->token_available = 1;

    return 1;
}

/*
 * The dispatcher for token fetchers.
 */

static int
cooklang_parser_fetch_next_token(cooklang_parser_t *parser)
{
    /* Ensure that the buffer is initialized. */

    if (!CACHE(parser, 1))
        return 0;

    /* Check if we just started scanning.  Fetch STREAM-START then. */

    if (!parser->stream_start_produced)
        return cooklang_parser_fetch_stream_start(parser);

    /* Eat whitespaces and comments until we reach the next token. */

    if (!cooklang_parser_scan_to_next_token(parser))
        return 0;

    /* Remove obsolete potential simple keys. */

    if (!cooklang_parser_stale_simple_keys(parser))
        return 0;

    /* Check the indentation level against the current column. */

    if (!cooklang_parser_unroll_indent(parser, parser->mark.column))
        return 0;

    /*
     * Ensure that the buffer contains at least 4 characters.  4 is the length
     * of the longest indicators ('--- ' and '... ').
     */

    if (!CACHE(parser, 4))
        return 0;

    /* Is it the end of the stream? */

    if (IS_Z(parser->buffer))
        return cooklang_parser_fetch_stream_end(parser);

    /* Is it a directive? */

    if (parser->mark.column == 0 && CHECK(parser->buffer, '%'))
        return cooklang_parser_fetch_directive(parser);

    /* Is it the document start indicator? */

    if (parser->mark.column == 0
            && CHECK_AT(parser->buffer, '-', 0)
            && CHECK_AT(parser->buffer, '-', 1)
            && CHECK_AT(parser->buffer, '-', 2)
            && IS_BLANKZ_AT(parser->buffer, 3))
        return cooklang_parser_fetch_document_indicator(parser,
                COOKLANG_DOCUMENT_START_TOKEN);

    /* Is it the document end indicator? */

    if (parser->mark.column == 0
            && CHECK_AT(parser->buffer, '.', 0)
            && CHECK_AT(parser->buffer, '.', 1)
            && CHECK_AT(parser->buffer, '.', 2)
            && IS_BLANKZ_AT(parser->buffer, 3))
        return cooklang_parser_fetch_document_indicator(parser,
                COOKLANG_DOCUMENT_END_TOKEN);

    /* Is it the flow sequence start indicator? */

    if (CHECK(parser->buffer, '['))
        return cooklang_parser_fetch_flow_collection_start(parser,
                COOKLANG_FLOW_SEQUENCE_START_TOKEN);

    /* Is it the flow mapping start indicator? */

    if (CHECK(parser->buffer, '{'))
        return cooklang_parser_fetch_flow_collection_start(parser,
                COOKLANG_FLOW_MAPPING_START_TOKEN);

    /* Is it the flow sequence end indicator? */

    if (CHECK(parser->buffer, ']'))
        return cooklang_parser_fetch_flow_collection_end(parser,
                COOKLANG_FLOW_SEQUENCE_END_TOKEN);

    /* Is it the flow mapping end indicator? */

    if (CHECK(parser->buffer, '}'))
        return cooklang_parser_fetch_flow_collection_end(parser,
                COOKLANG_FLOW_MAPPING_END_TOKEN);

    /* Is it the flow entry indicator? */

    if (CHECK(parser->buffer, ','))
        return cooklang_parser_fetch_flow_entry(parser);

    /* Is it the block entry indicator? */

    if (CHECK(parser->buffer, '-') && IS_BLANKZ_AT(parser->buffer, 1))
        return cooklang_parser_fetch_block_entry(parser);

    /* Is it the key indicator? */

    if (CHECK(parser->buffer, '?')
            && (parser->flow_level || IS_BLANKZ_AT(parser->buffer, 1)))
        return cooklang_parser_fetch_key(parser);

    /* Is it the value indicator? */

    if (CHECK(parser->buffer, ':')
            && (parser->flow_level || IS_BLANKZ_AT(parser->buffer, 1)))
        return cooklang_parser_fetch_value(parser);

    /* Is it an alias? */

    if (CHECK(parser->buffer, '*'))
        return cooklang_parser_fetch_anchor(parser, COOKLANG_ALIAS_TOKEN);

    /* Is it an anchor? */

    if (CHECK(parser->buffer, '&'))
        return cooklang_parser_fetch_anchor(parser, COOKLANG_ANCHOR_TOKEN);

    /* Is it a tag? */

    if (CHECK(parser->buffer, '!'))
        return cooklang_parser_fetch_tag(parser);

    /* Is it a literal scalar? */

    if (CHECK(parser->buffer, '|') && !parser->flow_level)
        return cooklang_parser_fetch_block_scalar(parser, 1);

    /* Is it a folded scalar? */

    if (CHECK(parser->buffer, '>') && !parser->flow_level)
        return cooklang_parser_fetch_block_scalar(parser, 0);

    /* Is it a single-quoted scalar? */

    if (CHECK(parser->buffer, '\''))
        return cooklang_parser_fetch_flow_scalar(parser, 1);

    /* Is it a double-quoted scalar? */

    if (CHECK(parser->buffer, '"'))
        return cooklang_parser_fetch_flow_scalar(parser, 0);

    /*
     * Is it a plain scalar?
     *
     * A plain scalar may start with any non-blank characters except
     *
     *      '-', '?', ':', ',', '[', ']', '{', '}',
     *      '#', '&', '*', '!', '|', '>', '\'', '\"',
     *      '%', '@', '`'.
     *
     * In the block context (and, for the '-' indicator, in the flow context
     * too), it may also start with the characters
     *
     *      '-', '?', ':'
     *
     * if it is followed by a non-space character.
     *
     * The last rule is more restrictive than the specification requires.
     */

    if (!(IS_BLANKZ(parser->buffer) || CHECK(parser->buffer, '-')
                || CHECK(parser->buffer, '?') || CHECK(parser->buffer, ':')
                || CHECK(parser->buffer, ',') || CHECK(parser->buffer, '[')
                || CHECK(parser->buffer, ']') || CHECK(parser->buffer, '{')
                || CHECK(parser->buffer, '}') || CHECK(parser->buffer, '#')
                || CHECK(parser->buffer, '&') || CHECK(parser->buffer, '*')
                || CHECK(parser->buffer, '!') || CHECK(parser->buffer, '|')
                || CHECK(parser->buffer, '>') || CHECK(parser->buffer, '\'')
                || CHECK(parser->buffer, '"') || CHECK(parser->buffer, '%')
                || CHECK(parser->buffer, '@') || CHECK(parser->buffer, '`')) ||
            (CHECK(parser->buffer, '-') && !IS_BLANK_AT(parser->buffer, 1)) ||
            (!parser->flow_level &&
             (CHECK(parser->buffer, '?') || CHECK(parser->buffer, ':'))
             && !IS_BLANKZ_AT(parser->buffer, 1)))
        return cooklang_parser_fetch_plain_scalar(parser);

    /*
     * If we don't determine the token type so far, it is an error.
     */

    return cooklang_parser_set_scanner_error(parser,
            "while scanning for the next token", parser->mark,
            "found character that cannot start any token");
}

/*
 * Initialize the scanner and produce the STREAM-START token.
 */

static int
cooklang_parser_fetch_stream_start(cooklang_parser_t *parser)
{
    cooklang_simple_key_t simple_key = { 0, 0, 0, { 0, 0, 0 } };
    cooklang_token_t token;

    /* Set the initial indentation. */

    parser->indent = -1;

    /* Initialize the simple key stack. */

    if (!PUSH(parser, parser->simple_keys, simple_key))
        return 0;

    /* A simple key is allowed at the beginning of the stream. */

    parser->simple_key_allowed = 1;

    /* We have started. */

    parser->stream_start_produced = 1;

    /* Create the STREAM-START token and append it to the queue. */

    STREAM_START_TOKEN_INIT(token, parser->encoding,
            parser->mark, parser->mark);

    if (!ENQUEUE(parser, parser->tokens, token))
        return 0;

    return 1;
}

/*
 * Produce the STREAM-END token and shut down the scanner.
 */

static int
cooklang_parser_fetch_stream_end(cooklang_parser_t *parser)
{
    cooklang_token_t token;

    /* Force new line. */

    if (parser->mark.column != 0) {
        parser->mark.column = 0;
        parser->mark.line ++;
    }

    /* Reset the indentation level. */

    if (!cooklang_parser_unroll_indent(parser, -1))
        return 0;

    /* Reset simple keys. */

    if (!cooklang_parser_remove_simple_key(parser))
        return 0;

    parser->simple_key_allowed = 0;

    /* Create the STREAM-END token and append it to the queue. */

    STREAM_END_TOKEN_INIT(token, parser->mark, parser->mark);

    if (!ENQUEUE(parser, parser->tokens, token))
        return 0;

    return 1;
}

/*
 * Produce the KEY token.
 */

static int
cooklang_parser_fetch_key(cooklang_parser_t *parser)
{
    cooklang_mark_t start_mark, end_mark;
    cooklang_token_t token;

    /* In the block context, additional checks are required. */

    if (!parser->flow_level)
    {
        /* Check if we are allowed to start a new key (not necessary simple). */

        if (!parser->simple_key_allowed) {
            return cooklang_parser_set_scanner_error(parser, NULL, parser->mark,
                    "mapping keys are not allowed in this context");
        }

        /* Add the BLOCK-MAPPING-START token if needed. */

        if (!cooklang_parser_roll_indent(parser, parser->mark.column, -1,
                    COOKLANG_BLOCK_MAPPING_START_TOKEN, parser->mark))
            return 0;
    }

    /* Reset any potential simple keys on the current flow level. */

    if (!cooklang_parser_remove_simple_key(parser))
        return 0;

    /* Simple keys are allowed after '?' in the block context. */

    parser->simple_key_allowed = (!parser->flow_level);

    /* Consume the token. */

    start_mark = parser->mark;
    SKIP(parser);
    end_mark = parser->mark;

    /* Create the KEY token and append it to the queue. */

    TOKEN_INIT(token, COOKLANG_KEY_TOKEN, start_mark, end_mark);

    if (!ENQUEUE(parser, parser->tokens, token))
        return 0;

    return 1;
}

/*
 * Produce the VALUE token.
 */

static int
cooklang_parser_fetch_value(cooklang_parser_t *parser)
{
    cooklang_mark_t start_mark, end_mark;
    cooklang_token_t token;


    /* Consume the token. */

    start_mark = parser->mark;
    SKIP(parser);
    end_mark = parser->mark;

    /* Create the VALUE token and append it to the queue. */

    TOKEN_INIT(token, COOKLANG_VALUE_TOKEN, start_mark, end_mark);

    if (!ENQUEUE(parser, parser->tokens, token))
        return 0;

    return 1;
}


/*
 * Eat whitespaces and comments until the next token is found.
 */

static int
cooklang_parser_scan_to_next_token(cooklang_parser_t *parser)
{
    /* Until the next token is not found. */

    while (1)
    {
        /* Allow the BOM mark to start a line. */

        if (!CACHE(parser, 1)) return 0;

        if (parser->mark.column == 0 && IS_BOM(parser->buffer))
            SKIP(parser);

        /*
         * Eat whitespaces.
         *
         * Tabs are allowed:
         *
         *  - in the flow context;
         *  - in the block context, but not at the beginning of the line or
         *  after '-', '?', or ':' (complex value).
         */

        if (!CACHE(parser, 1)) return 0;

        while (CHECK(parser->buffer,' ') ||
                (CHECK(parser->buffer, '\t'))) {
            SKIP(parser);
            if (!CACHE(parser, 1)) return 0;
        }

        /* Eat a comment until a line break. */

        if (CHECK(parser->buffer, '#')) {
            while (!IS_BREAKZ(parser->buffer)) {
                SKIP(parser);
                if (!CACHE(parser, 1)) return 0;
            }
        }

        /* If it is a line break, eat it. */

        if (IS_BREAK(parser->buffer))
        {
            if (!CACHE(parser, 2)) return 0;
            SKIP_LINE(parser);

            /* In the block context, a new line may start a simple key. */

            if (!parser->flow_level) {
                parser->simple_key_allowed = 1;
            }
        }
        else
        {
            /* We have found a token. */

            break;
        }
    }

    return 1;
}

/*
 * Scan a COOKLANG-DIRECTIVE or TAG-DIRECTIVE token.
 *
 * Scope:
 *      %COOKLANG    1.1    # a comment \n
 *      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *      %TAG    !cooklang!  tag:cooklang.org,2002:  \n
 *      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 */

int
cooklang_parser_scan_directive(cooklang_parser_t *parser, cooklang_token_t *token)
{
    cooklang_mark_t start_mark, end_mark;
    cooklang_char_t *name = NULL;
    int major, minor;
    cooklang_char_t *handle = NULL, *prefix = NULL;

    /* Eat '%'. */

    start_mark = parser->mark;

    SKIP(parser);

    /* Scan the directive name. */

    if (!cooklang_parser_scan_directive_name(parser, start_mark, &name))
        goto error;

    /* Is it a COOKLANG directive? */

    if (strcmp((char *)name, "COOKLANG") == 0)
    {
        /* Scan the VERSION directive value. */

        if (!cooklang_parser_scan_version_directive_value(parser, start_mark,
                    &major, &minor))
            goto error;

        end_mark = parser->mark;

        /* Create a VERSION-DIRECTIVE token. */

        VERSION_DIRECTIVE_TOKEN_INIT(*token, major, minor,
                start_mark, end_mark);
    }

    /* Is it a TAG directive? */

    else if (strcmp((char *)name, "TAG") == 0)
    {
        /* Scan the TAG directive value. */

        if (!cooklang_parser_scan_tag_directive_value(parser, start_mark,
                    &handle, &prefix))
            goto error;

        end_mark = parser->mark;

        /* Create a TAG-DIRECTIVE token. */

        TAG_DIRECTIVE_TOKEN_INIT(*token, handle, prefix,
                start_mark, end_mark);
    }

    /* Unknown directive. */

    else
    {
        cooklang_parser_set_scanner_error(parser, "while scanning a directive",
                start_mark, "found unknown directive name");
        goto error;
    }

    /* Eat the rest of the line including any comments. */

    if (!CACHE(parser, 1)) goto error;

    while (IS_BLANK(parser->buffer)) {
        SKIP(parser);
        if (!CACHE(parser, 1)) goto error;
    }

    if (CHECK(parser->buffer, '#')) {
        while (!IS_BREAKZ(parser->buffer)) {
            SKIP(parser);
            if (!CACHE(parser, 1)) goto error;
        }
    }

    /* Check if we are at the end of the line. */

    if (!IS_BREAKZ(parser->buffer)) {
        cooklang_parser_set_scanner_error(parser, "while scanning a directive",
                start_mark, "did not find expected comment or line break");
        goto error;
    }

    /* Eat a line break. */

    if (IS_BREAK(parser->buffer)) {
        if (!CACHE(parser, 2)) goto error;
        SKIP_LINE(parser);
    }

    cooklang_free(name);

    return 1;

error:
    cooklang_free(prefix);
    cooklang_free(handle);
    cooklang_free(name);
    return 0;
}

/*
 * Scan the directive name.
 *
 * Scope:
 *      %COOKLANG   1.1     # a comment \n
 *       ^^^^
 *      %TAG    !cooklang!  tag:cooklang.org,2002:  \n
 *       ^^^
 */

static int
cooklang_parser_scan_directive_name(cooklang_parser_t *parser,
        cooklang_mark_t start_mark, cooklang_char_t **name)
{
    cooklang_string_t string = NULL_STRING;

    if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;

    /* Consume the directive name. */

    if (!CACHE(parser, 1)) goto error;

    while (IS_ALPHA(parser->buffer))
    {
        if (!READ(parser, string)) goto error;
        if (!CACHE(parser, 1)) goto error;
    }

    /* Check if the name is empty. */

    if (string.start == string.pointer) {
        cooklang_parser_set_scanner_error(parser, "while scanning a directive",
                start_mark, "could not find expected directive name");
        goto error;
    }

    /* Check for an blank character after the name. */

    if (!IS_BLANKZ(parser->buffer)) {
        cooklang_parser_set_scanner_error(parser, "while scanning a directive",
                start_mark, "found unexpected non-alphabetical character");
        goto error;
    }

    *name = string.start;

    return 1;

error:
    STRING_DEL(parser, string);
    return 0;
}

/*
 * Scan the value of VERSION-DIRECTIVE.
 *
 * Scope:
 *      %COOKLANG   1.1     # a comment \n
 *           ^^^^^^
 */

static int
cooklang_parser_scan_version_directive_value(cooklang_parser_t *parser,
        cooklang_mark_t start_mark, int *major, int *minor)
{
    /* Eat whitespaces. */

    if (!CACHE(parser, 1)) return 0;

    while (IS_BLANK(parser->buffer)) {
        SKIP(parser);
        if (!CACHE(parser, 1)) return 0;
    }

    /* Consume the major version number. */

    if (!cooklang_parser_scan_version_directive_number(parser, start_mark, major))
        return 0;

    /* Eat '.'. */

    if (!CHECK(parser->buffer, '.')) {
        return cooklang_parser_set_scanner_error(parser, "while scanning a %COOKLANG directive",
                start_mark, "did not find expected digit or '.' character");
    }

    SKIP(parser);

    /* Consume the minor version number. */

    if (!cooklang_parser_scan_version_directive_number(parser, start_mark, minor))
        return 0;

    return 1;
}

#define MAX_NUMBER_LENGTH   9

/*
 * Scan the version number of VERSION-DIRECTIVE.
 *
 * Scope:
 *      %COOKLANG   1.1     # a comment \n
 *              ^
 *      %COOKLANG   1.1     # a comment \n
 *                ^
 */

static int
cooklang_parser_scan_version_directive_number(cooklang_parser_t *parser,
        cooklang_mark_t start_mark, int *number)
{
    int value = 0;
    size_t length = 0;

    /* Repeat while the next character is digit. */

    if (!CACHE(parser, 1)) return 0;

    while (IS_DIGIT(parser->buffer))
    {
        /* Check if the number is too long. */

        if (++length > MAX_NUMBER_LENGTH) {
            return cooklang_parser_set_scanner_error(parser, "while scanning a %COOKLANG directive",
                    start_mark, "found extremely long version number");
        }

        value = value*10 + AS_DIGIT(parser->buffer);

        SKIP(parser);

        if (!CACHE(parser, 1)) return 0;
    }

    /* Check if the number was present. */

    if (!length) {
        return cooklang_parser_set_scanner_error(parser, "while scanning a %COOKLANG directive",
                start_mark, "did not find expected version number");
    }

    *number = value;

    return 1;
}

/*
 * Scan the value of a TAG-DIRECTIVE token.
 *
 * Scope:
 *      %TAG    !cooklang!  tag:cooklang.org,2002:  \n
 *          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 */

static int
cooklang_parser_scan_tag_directive_value(cooklang_parser_t *parser,
        cooklang_mark_t start_mark, cooklang_char_t **handle, cooklang_char_t **prefix)
{
    cooklang_char_t *handle_value = NULL;
    cooklang_char_t *prefix_value = NULL;

    /* Eat whitespaces. */

    if (!CACHE(parser, 1)) goto error;

    while (IS_BLANK(parser->buffer)) {
        SKIP(parser);
        if (!CACHE(parser, 1)) goto error;
    }

    /* Scan a handle. */

    if (!cooklang_parser_scan_tag_handle(parser, 1, start_mark, &handle_value))
        goto error;

    /* Expect a whitespace. */

    if (!CACHE(parser, 1)) goto error;

    if (!IS_BLANK(parser->buffer)) {
        cooklang_parser_set_scanner_error(parser, "while scanning a %TAG directive",
                start_mark, "did not find expected whitespace");
        goto error;
    }

    /* Eat whitespaces. */

    while (IS_BLANK(parser->buffer)) {
        SKIP(parser);
        if (!CACHE(parser, 1)) goto error;
    }

    /* Scan a prefix. */

    if (!cooklang_parser_scan_tag_uri(parser, 1, 1, NULL, start_mark, &prefix_value))
        goto error;

    /* Expect a whitespace or line break. */

    if (!CACHE(parser, 1)) goto error;

    if (!IS_BLANKZ(parser->buffer)) {
        cooklang_parser_set_scanner_error(parser, "while scanning a %TAG directive",
                start_mark, "did not find expected whitespace or line break");
        goto error;
    }

    *handle = handle_value;
    *prefix = prefix_value;

    return 1;

error:
    cooklang_free(handle_value);
    cooklang_free(prefix_value);
    return 0;
}

static int
cooklang_parser_scan_anchor(cooklang_parser_t *parser, cooklang_token_t *token,
        cooklang_token_type_t type)
{
    int length = 0;
    cooklang_mark_t start_mark, end_mark;
    cooklang_string_t string = NULL_STRING;

    if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;

    /* Eat the indicator character. */

    start_mark = parser->mark;

    SKIP(parser);

    /* Consume the value. */

    if (!CACHE(parser, 1)) goto error;

    while (IS_ALPHA(parser->buffer)) {
        if (!READ(parser, string)) goto error;
        if (!CACHE(parser, 1)) goto error;
        length ++;
    }

    end_mark = parser->mark;

    /*
     * Check if length of the anchor is greater than 0 and it is followed by
     * a whitespace character or one of the indicators:
     *
     *      '?', ':', ',', ']', '}', '%', '@', '`'.
     */

    if (!length || !(IS_BLANKZ(parser->buffer) || CHECK(parser->buffer, '?')
                || CHECK(parser->buffer, ':') || CHECK(parser->buffer, ',')
                || CHECK(parser->buffer, ']') || CHECK(parser->buffer, '}')
                || CHECK(parser->buffer, '%') || CHECK(parser->buffer, '@')
                || CHECK(parser->buffer, '`'))) {
        cooklang_parser_set_scanner_error(parser, type == COOKLANG_ANCHOR_TOKEN ?
                "while scanning an anchor" : "while scanning an alias", start_mark,
                "did not find expected alphabetic or numeric character");
        goto error;
    }

    /* Create a token. */

    if (type == COOKLANG_ANCHOR_TOKEN) {
        ANCHOR_TOKEN_INIT(*token, string.start, start_mark, end_mark);
    }
    else {
        ALIAS_TOKEN_INIT(*token, string.start, start_mark, end_mark);
    }

    return 1;

error:
    STRING_DEL(parser, string);
    return 0;
}

/*
 * Scan a TAG token.
 */

static int
cooklang_parser_scan_tag(cooklang_parser_t *parser, cooklang_token_t *token)
{
    cooklang_char_t *handle = NULL;
    cooklang_char_t *suffix = NULL;
    cooklang_mark_t start_mark, end_mark;

    start_mark = parser->mark;

    /* Check if the tag is in the canonical form. */

    if (!CACHE(parser, 2)) goto error;

    if (CHECK_AT(parser->buffer, '<', 1))
    {
        /* Set the handle to '' */

        handle = COOKLANG_MALLOC(1);
        if (!handle) goto error;
        handle[0] = '\0';

        /* Eat '!<' */

        SKIP(parser);
        SKIP(parser);

        /* Consume the tag value. */

        if (!cooklang_parser_scan_tag_uri(parser, 1, 0, NULL, start_mark, &suffix))
            goto error;

        /* Check for '>' and eat it. */

        if (!CHECK(parser->buffer, '>')) {
            cooklang_parser_set_scanner_error(parser, "while scanning a tag",
                    start_mark, "did not find the expected '>'");
            goto error;
        }

        SKIP(parser);
    }
    else
    {
        /* The tag has either the '!suffix' or the '!handle!suffix' form. */

        /* First, try to scan a handle. */

        if (!cooklang_parser_scan_tag_handle(parser, 0, start_mark, &handle))
            goto error;

        /* Check if it is, indeed, handle. */

        if (handle[0] == '!' && handle[1] != '\0' && handle[strlen((char *)handle)-1] == '!')
        {
            /* Scan the suffix now. */

            if (!cooklang_parser_scan_tag_uri(parser, 0, 0, NULL, start_mark, &suffix))
                goto error;
        }
        else
        {
            /* It wasn't a handle after all.  Scan the rest of the tag. */

            if (!cooklang_parser_scan_tag_uri(parser, 0, 0, handle, start_mark, &suffix))
                goto error;

            /* Set the handle to '!'. */

            cooklang_free(handle);
            handle = COOKLANG_MALLOC(2);
            if (!handle) goto error;
            handle[0] = '!';
            handle[1] = '\0';

            /*
             * A special case: the '!' tag.  Set the handle to '' and the
             * suffix to '!'.
             */

            if (suffix[0] == '\0') {
                cooklang_char_t *tmp = handle;
                handle = suffix;
                suffix = tmp;
            }
        }
    }

    /* Check the character which ends the tag. */

    if (!CACHE(parser, 1)) goto error;

    if (!IS_BLANKZ(parser->buffer)) {
        if (!parser->flow_level || !CHECK(parser->buffer, ',') ) {
            cooklang_parser_set_scanner_error(parser, "while scanning a tag",
                    start_mark, "did not find expected whitespace or line break");
            goto error;
        }
    }

    end_mark = parser->mark;

    /* Create a token. */

    TAG_TOKEN_INIT(*token, handle, suffix, start_mark, end_mark);

    return 1;

error:
    cooklang_free(handle);
    cooklang_free(suffix);
    return 0;
}

/*
 * Scan a tag handle.
 */

static int
cooklang_parser_scan_tag_handle(cooklang_parser_t *parser, int directive,
        cooklang_mark_t start_mark, cooklang_char_t **handle)
{
    cooklang_string_t string = NULL_STRING;

    if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;

    /* Check the initial '!' character. */

    if (!CACHE(parser, 1)) goto error;

    if (!CHECK(parser->buffer, '!')) {
        cooklang_parser_set_scanner_error(parser, directive ?
                "while scanning a tag directive" : "while scanning a tag",
                start_mark, "did not find expected '!'");
        goto error;
    }

    /* Copy the '!' character. */

    if (!READ(parser, string)) goto error;

    /* Copy all subsequent alphabetical and numerical characters. */

    if (!CACHE(parser, 1)) goto error;

    while (IS_ALPHA(parser->buffer))
    {
        if (!READ(parser, string)) goto error;
        if (!CACHE(parser, 1)) goto error;
    }

    /* Check if the trailing character is '!' and copy it. */

    if (CHECK(parser->buffer, '!'))
    {
        if (!READ(parser, string)) goto error;
    }
    else
    {
        /*
         * It's either the '!' tag or not really a tag handle.  If it's a %TAG
         * directive, it's an error.  If it's a tag token, it must be a part of
         * URI.
         */

        if (directive && !(string.start[0] == '!' && string.start[1] == '\0')) {
            cooklang_parser_set_scanner_error(parser, "while parsing a tag directive",
                    start_mark, "did not find expected '!'");
            goto error;
        }
    }

    *handle = string.start;

    return 1;

error:
    STRING_DEL(parser, string);
    return 0;
}

/*
 * Scan a tag.
 */

static int
cooklang_parser_scan_tag_uri(cooklang_parser_t *parser, int uri_char, int directive,
        cooklang_char_t *head, cooklang_mark_t start_mark, cooklang_char_t **uri)
{
    size_t length = head ? strlen((char *)head) : 0;
    cooklang_string_t string = NULL_STRING;

    if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;

    /* Resize the string to include the head. */

    while ((size_t)(string.end - string.start) <= length) {
        if (!cooklang_string_extend(&string.start, &string.pointer, &string.end)) {
            parser->error = COOKLANG_MEMORY_ERROR;
            goto error;
        }
    }

    /*
     * Copy the head if needed.
     *
     * Note that we don't copy the leading '!' character.
     */

    if (length > 1) {
        memcpy(string.start, head+1, length-1);
        string.pointer += length-1;
    }

    /* Scan the tag. */

    if (!CACHE(parser, 1)) goto error;

    /*
     * The set of characters that may appear in URI is as follows:
     *
     *      '0'-'9', 'A'-'Z', 'a'-'z', '_', '-', ';', '/', '?', ':', '@', '&',
     *      '=', '+', '$', '.', '!', '~', '*', '\'', '(', ')', '%'.
     *
     * If we are inside a verbatim tag <...> (parameter uri_char is true)
     * then also the following flow indicators are allowed:
     *      ',', '[', ']'
     */

    while (IS_ALPHA(parser->buffer) || CHECK(parser->buffer, ';')
            || CHECK(parser->buffer, '/') || CHECK(parser->buffer, '?')
            || CHECK(parser->buffer, ':') || CHECK(parser->buffer, '@')
            || CHECK(parser->buffer, '&') || CHECK(parser->buffer, '=')
            || CHECK(parser->buffer, '+') || CHECK(parser->buffer, '$')
            || CHECK(parser->buffer, '.') || CHECK(parser->buffer, '%')
            || CHECK(parser->buffer, '!') || CHECK(parser->buffer, '~')
            || CHECK(parser->buffer, '*') || CHECK(parser->buffer, '\'')
            || CHECK(parser->buffer, '(') || CHECK(parser->buffer, ')')
            || (uri_char && (
                CHECK(parser->buffer, ',')
                || CHECK(parser->buffer, '[') || CHECK(parser->buffer, ']')
                )
            ))
    {
        /* Check if it is a URI-escape sequence. */

        if (CHECK(parser->buffer, '%')) {
            if (!STRING_EXTEND(parser, string))
                goto error;

            if (!cooklang_parser_scan_uri_escapes(parser,
                        directive, start_mark, &string)) goto error;
        }
        else {
            if (!READ(parser, string)) goto error;
        }

        length ++;
        if (!CACHE(parser, 1)) goto error;
    }

    /* Check if the tag is non-empty. */

    if (!length) {
        if (!STRING_EXTEND(parser, string))
            goto error;

        cooklang_parser_set_scanner_error(parser, directive ?
                "while parsing a %TAG directive" : "while parsing a tag",
                start_mark, "did not find expected tag URI");
        goto error;
    }

    *uri = string.start;

    return 1;

error:
    STRING_DEL(parser, string);
    return 0;
}

/*
 * Decode an URI-escape sequence corresponding to a single UTF-8 character.
 */

static int
cooklang_parser_scan_uri_escapes(cooklang_parser_t *parser, int directive,
        cooklang_mark_t start_mark, cooklang_string_t *string)
{
    int width = 0;

    /* Decode the required number of characters. */

    do {

        unsigned char octet = 0;

        /* Check for a URI-escaped octet. */

        if (!CACHE(parser, 3)) return 0;

        if (!(CHECK(parser->buffer, '%')
                    && IS_HEX_AT(parser->buffer, 1)
                    && IS_HEX_AT(parser->buffer, 2))) {
            return cooklang_parser_set_scanner_error(parser, directive ?
                    "while parsing a %TAG directive" : "while parsing a tag",
                    start_mark, "did not find URI escaped octet");
        }

        /* Get the octet. */

        octet = (AS_HEX_AT(parser->buffer, 1) << 4) + AS_HEX_AT(parser->buffer, 2);

        /* If it is the leading octet, determine the length of the UTF-8 sequence. */

        if (!width)
        {
            width = (octet & 0x80) == 0x00 ? 1 :
                    (octet & 0xE0) == 0xC0 ? 2 :
                    (octet & 0xF0) == 0xE0 ? 3 :
                    (octet & 0xF8) == 0xF0 ? 4 : 0;
            if (!width) {
                return cooklang_parser_set_scanner_error(parser, directive ?
                        "while parsing a %TAG directive" : "while parsing a tag",
                        start_mark, "found an incorrect leading UTF-8 octet");
            }
        }
        else
        {
            /* Check if the trailing octet is correct. */

            if ((octet & 0xC0) != 0x80) {
                return cooklang_parser_set_scanner_error(parser, directive ?
                        "while parsing a %TAG directive" : "while parsing a tag",
                        start_mark, "found an incorrect trailing UTF-8 octet");
            }
        }

        /* Copy the octet and move the pointers. */

        *(string->pointer++) = octet;
        SKIP(parser);
        SKIP(parser);
        SKIP(parser);

    } while (--width);

    return 1;
}

/*
 * Scan a block scalar.
 */

static int
cooklang_parser_scan_block_scalar(cooklang_parser_t *parser, cooklang_token_t *token,
        int literal)
{
    cooklang_mark_t start_mark;
    cooklang_mark_t end_mark;
    cooklang_string_t string = NULL_STRING;
    cooklang_string_t leading_break = NULL_STRING;
    cooklang_string_t trailing_breaks = NULL_STRING;
    int chomping = 0;
    int increment = 0;
    int indent = 0;
    int leading_blank = 0;
    int trailing_blank = 0;

    if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
    if (!STRING_INIT(parser, leading_break, INITIAL_STRING_SIZE)) goto error;
    if (!STRING_INIT(parser, trailing_breaks, INITIAL_STRING_SIZE)) goto error;

    /* Eat the indicator '|' or '>'. */

    start_mark = parser->mark;

    SKIP(parser);

    /* Scan the additional block scalar indicators. */

    if (!CACHE(parser, 1)) goto error;

    /* Check for a chomping indicator. */

    if (CHECK(parser->buffer, '+') || CHECK(parser->buffer, '-'))
    {
        /* Set the chomping method and eat the indicator. */

        chomping = CHECK(parser->buffer, '+') ? +1 : -1;

        SKIP(parser);

        /* Check for an indentation indicator. */

        if (!CACHE(parser, 1)) goto error;

        if (IS_DIGIT(parser->buffer))
        {
            /* Check that the indentation is greater than 0. */

            if (CHECK(parser->buffer, '0')) {
                cooklang_parser_set_scanner_error(parser, "while scanning a block scalar",
                        start_mark, "found an indentation indicator equal to 0");
                goto error;
            }

            /* Get the indentation level and eat the indicator. */

            increment = AS_DIGIT(parser->buffer);

            SKIP(parser);
        }
    }

    /* Do the same as above, but in the opposite order. */

    else if (IS_DIGIT(parser->buffer))
    {
        if (CHECK(parser->buffer, '0')) {
            cooklang_parser_set_scanner_error(parser, "while scanning a block scalar",
                    start_mark, "found an indentation indicator equal to 0");
            goto error;
        }

        increment = AS_DIGIT(parser->buffer);

        SKIP(parser);

        if (!CACHE(parser, 1)) goto error;

        if (CHECK(parser->buffer, '+') || CHECK(parser->buffer, '-')) {
            chomping = CHECK(parser->buffer, '+') ? +1 : -1;

            SKIP(parser);
        }
    }

    /* Eat whitespaces and comments to the end of the line. */

    if (!CACHE(parser, 1)) goto error;

    while (IS_BLANK(parser->buffer)) {
        SKIP(parser);
        if (!CACHE(parser, 1)) goto error;
    }

    if (CHECK(parser->buffer, '#')) {
        while (!IS_BREAKZ(parser->buffer)) {
            SKIP(parser);
            if (!CACHE(parser, 1)) goto error;
        }
    }

    /* Check if we are at the end of the line. */

    if (!IS_BREAKZ(parser->buffer)) {
        cooklang_parser_set_scanner_error(parser, "while scanning a block scalar",
                start_mark, "did not find expected comment or line break");
        goto error;
    }

    /* Eat a line break. */

    if (IS_BREAK(parser->buffer)) {
        if (!CACHE(parser, 2)) goto error;
        SKIP_LINE(parser);
    }

    end_mark = parser->mark;

    /* Set the indentation level if it was specified. */

    if (increment) {
        indent = parser->indent >= 0 ? parser->indent+increment : increment;
    }

    /* Scan the leading line breaks and determine the indentation level if needed. */

    if (!cooklang_parser_scan_block_scalar_breaks(parser, &indent, &trailing_breaks,
                start_mark, &end_mark)) goto error;

    /* Scan the block scalar content. */

    if (!CACHE(parser, 1)) goto error;

    while ((int)parser->mark.column == indent && !(IS_Z(parser->buffer)))
    {
        /*
         * We are at the beginning of a non-empty line.
         */

        /* Is it a trailing whitespace? */

        trailing_blank = IS_BLANK(parser->buffer);

        /* Check if we need to fold the leading line break. */

        if (!literal && (*leading_break.start == '\n')
                && !leading_blank && !trailing_blank)
        {
            /* Do we need to join the lines by space? */

            if (*trailing_breaks.start == '\0') {
                if (!STRING_EXTEND(parser, string)) goto error;
                *(string.pointer ++) = ' ';
            }

            CLEAR(parser, leading_break);
        }
        else {
            if (!JOIN(parser, string, leading_break)) goto error;
            CLEAR(parser, leading_break);
        }

        /* Append the remaining line breaks. */

        if (!JOIN(parser, string, trailing_breaks)) goto error;
        CLEAR(parser, trailing_breaks);

        /* Is it a leading whitespace? */

        leading_blank = IS_BLANK(parser->buffer);

        /* Consume the current line. */

        while (!IS_BREAKZ(parser->buffer)) {
            if (!READ(parser, string)) goto error;
            if (!CACHE(parser, 1)) goto error;
        }

        /* Consume the line break. */

        if (!CACHE(parser, 2)) goto error;

        if (!READ_LINE(parser, leading_break)) goto error;

        /* Eat the following indentation spaces and line breaks. */

        if (!cooklang_parser_scan_block_scalar_breaks(parser,
                    &indent, &trailing_breaks, start_mark, &end_mark)) goto error;
    }

    /* Chomp the tail. */

    if (chomping != -1) {
        if (!JOIN(parser, string, leading_break)) goto error;
    }
    if (chomping == 1) {
        if (!JOIN(parser, string, trailing_breaks)) goto error;
    }

    /* Create a token. */

    SCALAR_TOKEN_INIT(*token, string.start, string.pointer-string.start,
            literal ? COOKLANG_LITERAL_SCALAR_STYLE : COOKLANG_FOLDED_SCALAR_STYLE,
            start_mark, end_mark);

    STRING_DEL(parser, leading_break);
    STRING_DEL(parser, trailing_breaks);

    return 1;

error:
    STRING_DEL(parser, string);
    STRING_DEL(parser, leading_break);
    STRING_DEL(parser, trailing_breaks);

    return 0;
}

/*
 * Scan indentation spaces and line breaks for a block scalar.  Determine the
 * indentation level if needed.
 */

static int
cooklang_parser_scan_block_scalar_breaks(cooklang_parser_t *parser,
        int *indent, cooklang_string_t *breaks,
        cooklang_mark_t start_mark, cooklang_mark_t *end_mark)
{
    int max_indent = 0;

    *end_mark = parser->mark;

    /* Eat the indentation spaces and line breaks. */

    while (1)
    {
        /* Eat the indentation spaces. */

        if (!CACHE(parser, 1)) return 0;

        while ((!*indent || (int)parser->mark.column < *indent)
                && IS_SPACE(parser->buffer)) {
            SKIP(parser);
            if (!CACHE(parser, 1)) return 0;
        }

        if ((int)parser->mark.column > max_indent)
            max_indent = (int)parser->mark.column;

        /* Check for a tab character messing the indentation. */

        if ((!*indent || (int)parser->mark.column < *indent)
                && IS_TAB(parser->buffer)) {
            return cooklang_parser_set_scanner_error(parser, "while scanning a block scalar",
                    start_mark, "found a tab character where an indentation space is expected");
        }

        /* Have we found a non-empty line? */

        if (!IS_BREAK(parser->buffer)) break;

        /* Consume the line break. */

        if (!CACHE(parser, 2)) return 0;
        if (!READ_LINE(parser, *breaks)) return 0;
        *end_mark = parser->mark;
    }

    /* Determine the indentation level if needed. */

    if (!*indent) {
        *indent = max_indent;
        if (*indent < parser->indent + 1)
            *indent = parser->indent + 1;
        if (*indent < 1)
            *indent = 1;
    }

   return 1;
}

/*
 * Scan a quoted scalar.
 */

static int
cooklang_parser_scan_flow_scalar(cooklang_parser_t *parser, cooklang_token_t *token,
        int single)
{
    cooklang_mark_t start_mark;
    cooklang_mark_t end_mark;
    cooklang_string_t string = NULL_STRING;
    cooklang_string_t leading_break = NULL_STRING;
    cooklang_string_t trailing_breaks = NULL_STRING;
    cooklang_string_t whitespaces = NULL_STRING;
    int leading_blanks;

    if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
    if (!STRING_INIT(parser, leading_break, INITIAL_STRING_SIZE)) goto error;
    if (!STRING_INIT(parser, trailing_breaks, INITIAL_STRING_SIZE)) goto error;
    if (!STRING_INIT(parser, whitespaces, INITIAL_STRING_SIZE)) goto error;

    /* Eat the left quote. */

    start_mark = parser->mark;

    SKIP(parser);

    /* Consume the content of the quoted scalar. */

    while (1)
    {
        /* Check that there are no document indicators at the beginning of the line. */

        if (!CACHE(parser, 4)) goto error;

        if (parser->mark.column == 0 &&
            ((CHECK_AT(parser->buffer, '-', 0) &&
              CHECK_AT(parser->buffer, '-', 1) &&
              CHECK_AT(parser->buffer, '-', 2)) ||
             (CHECK_AT(parser->buffer, '.', 0) &&
              CHECK_AT(parser->buffer, '.', 1) &&
              CHECK_AT(parser->buffer, '.', 2))) &&
            IS_BLANKZ_AT(parser->buffer, 3))
        {
            cooklang_parser_set_scanner_error(parser, "while scanning a quoted scalar",
                    start_mark, "found unexpected document indicator");
            goto error;
        }

        /* Check for EOF. */

        if (IS_Z(parser->buffer)) {
            cooklang_parser_set_scanner_error(parser, "while scanning a quoted scalar",
                    start_mark, "found unexpected end of stream");
            goto error;
        }

        /* Consume non-blank characters. */

        if (!CACHE(parser, 2)) goto error;

        leading_blanks = 0;

        while (!IS_BLANKZ(parser->buffer))
        {
            /* Check for an escaped single quote. */

            if (single && CHECK_AT(parser->buffer, '\'', 0)
                    && CHECK_AT(parser->buffer, '\'', 1))
            {
                if (!STRING_EXTEND(parser, string)) goto error;
                *(string.pointer++) = '\'';
                SKIP(parser);
                SKIP(parser);
            }

            /* Check for the right quote. */

            else if (CHECK(parser->buffer, single ? '\'' : '"'))
            {
                break;
            }

            /* Check for an escaped line break. */

            else if (!single && CHECK(parser->buffer, '\\')
                    && IS_BREAK_AT(parser->buffer, 1))
            {
                if (!CACHE(parser, 3)) goto error;
                SKIP(parser);
                SKIP_LINE(parser);
                leading_blanks = 1;
                break;
            }

            /* Check for an escape sequence. */

            else if (!single && CHECK(parser->buffer, '\\'))
            {
                size_t code_length = 0;

                if (!STRING_EXTEND(parser, string)) goto error;

                /* Check the escape character. */

                switch (parser->buffer.pointer[1])
                {
                    case '0':
                        *(string.pointer++) = '\0';
                        break;

                    case 'a':
                        *(string.pointer++) = '\x07';
                        break;

                    case 'b':
                        *(string.pointer++) = '\x08';
                        break;

                    case 't':
                    case '\t':
                        *(string.pointer++) = '\x09';
                        break;

                    case 'n':
                        *(string.pointer++) = '\x0A';
                        break;

                    case 'v':
                        *(string.pointer++) = '\x0B';
                        break;

                    case 'f':
                        *(string.pointer++) = '\x0C';
                        break;

                    case 'r':
                        *(string.pointer++) = '\x0D';
                        break;

                    case 'e':
                        *(string.pointer++) = '\x1B';
                        break;

                    case ' ':
                        *(string.pointer++) = '\x20';
                        break;

                    case '"':
                        *(string.pointer++) = '"';
                        break;

                    case '/':
                        *(string.pointer++) = '/';
                        break;

                    case '\\':
                        *(string.pointer++) = '\\';
                        break;

                    case 'N':   /* NEL (#x85) */
                        *(string.pointer++) = '\xC2';
                        *(string.pointer++) = '\x85';
                        break;

                    case '_':   /* #xA0 */
                        *(string.pointer++) = '\xC2';
                        *(string.pointer++) = '\xA0';
                        break;

                    case 'L':   /* LS (#x2028) */
                        *(string.pointer++) = '\xE2';
                        *(string.pointer++) = '\x80';
                        *(string.pointer++) = '\xA8';
                        break;

                    case 'P':   /* PS (#x2029) */
                        *(string.pointer++) = '\xE2';
                        *(string.pointer++) = '\x80';
                        *(string.pointer++) = '\xA9';
                        break;

                    case 'x':
                        code_length = 2;
                        break;

                    case 'u':
                        code_length = 4;
                        break;

                    case 'U':
                        code_length = 8;
                        break;

                    default:
                        cooklang_parser_set_scanner_error(parser, "while parsing a quoted scalar",
                                start_mark, "found unknown escape character");
                        goto error;
                }

                SKIP(parser);
                SKIP(parser);

                /* Consume an arbitrary escape code. */

                if (code_length)
                {
                    unsigned int value = 0;
                    size_t k;

                    /* Scan the character value. */

                    if (!CACHE(parser, code_length)) goto error;

                    for (k = 0; k < code_length; k ++) {
                        if (!IS_HEX_AT(parser->buffer, k)) {
                            cooklang_parser_set_scanner_error(parser, "while parsing a quoted scalar",
                                    start_mark, "did not find expected hexdecimal number");
                            goto error;
                        }
                        value = (value << 4) + AS_HEX_AT(parser->buffer, k);
                    }

                    /* Check the value and write the character. */

                    if ((value >= 0xD800 && value <= 0xDFFF) || value > 0x10FFFF) {
                        cooklang_parser_set_scanner_error(parser, "while parsing a quoted scalar",
                                start_mark, "found invalid Unicode character escape code");
                        goto error;
                    }

                    if (value <= 0x7F) {
                        *(string.pointer++) = value;
                    }
                    else if (value <= 0x7FF) {
                        *(string.pointer++) = 0xC0 + (value >> 6);
                        *(string.pointer++) = 0x80 + (value & 0x3F);
                    }
                    else if (value <= 0xFFFF) {
                        *(string.pointer++) = 0xE0 + (value >> 12);
                        *(string.pointer++) = 0x80 + ((value >> 6) & 0x3F);
                        *(string.pointer++) = 0x80 + (value & 0x3F);
                    }
                    else {
                        *(string.pointer++) = 0xF0 + (value >> 18);
                        *(string.pointer++) = 0x80 + ((value >> 12) & 0x3F);
                        *(string.pointer++) = 0x80 + ((value >> 6) & 0x3F);
                        *(string.pointer++) = 0x80 + (value & 0x3F);
                    }

                    /* Advance the pointer. */

                    for (k = 0; k < code_length; k ++) {
                        SKIP(parser);
                    }
                }
            }

            else
            {
                /* It is a non-escaped non-blank character. */

                if (!READ(parser, string)) goto error;
            }

            if (!CACHE(parser, 2)) goto error;
        }

        /* Check if we are at the end of the scalar. */

        /* Fix for crash unitialized value crash
         * Credit for the bug and input is to OSS Fuzz
         * Credit for the fix to Alex Gaynor
         */
        if (!CACHE(parser, 1)) goto error;
        if (CHECK(parser->buffer, single ? '\'' : '"'))
            break;

        /* Consume blank characters. */

        if (!CACHE(parser, 1)) goto error;

        while (IS_BLANK(parser->buffer) || IS_BREAK(parser->buffer))
        {
            if (IS_BLANK(parser->buffer))
            {
                /* Consume a space or a tab character. */

                if (!leading_blanks) {
                    if (!READ(parser, whitespaces)) goto error;
                }
                else {
                    SKIP(parser);
                }
            }
            else
            {
                if (!CACHE(parser, 2)) goto error;

                /* Check if it is a first line break. */

                if (!leading_blanks)
                {
                    CLEAR(parser, whitespaces);
                    if (!READ_LINE(parser, leading_break)) goto error;
                    leading_blanks = 1;
                }
                else
                {
                    if (!READ_LINE(parser, trailing_breaks)) goto error;
                }
            }
            if (!CACHE(parser, 1)) goto error;
        }

        /* Join the whitespaces or fold line breaks. */

        if (leading_blanks)
        {
            /* Do we need to fold line breaks? */

            if (leading_break.start[0] == '\n') {
                if (trailing_breaks.start[0] == '\0') {
                    if (!STRING_EXTEND(parser, string)) goto error;
                    *(string.pointer++) = ' ';
                }
                else {
                    if (!JOIN(parser, string, trailing_breaks)) goto error;
                    CLEAR(parser, trailing_breaks);
                }
                CLEAR(parser, leading_break);
            }
            else {
                if (!JOIN(parser, string, leading_break)) goto error;
                if (!JOIN(parser, string, trailing_breaks)) goto error;
                CLEAR(parser, leading_break);
                CLEAR(parser, trailing_breaks);
            }
        }
        else
        {
            if (!JOIN(parser, string, whitespaces)) goto error;
            CLEAR(parser, whitespaces);
        }
    }

    /* Eat the right quote. */

    SKIP(parser);

    end_mark = parser->mark;

    /* Create a token. */

    SCALAR_TOKEN_INIT(*token, string.start, string.pointer-string.start,
            single ? COOKLANG_SINGLE_QUOTED_SCALAR_STYLE : COOKLANG_DOUBLE_QUOTED_SCALAR_STYLE,
            start_mark, end_mark);

    STRING_DEL(parser, leading_break);
    STRING_DEL(parser, trailing_breaks);
    STRING_DEL(parser, whitespaces);

    return 1;

error:
    STRING_DEL(parser, string);
    STRING_DEL(parser, leading_break);
    STRING_DEL(parser, trailing_breaks);
    STRING_DEL(parser, whitespaces);

    return 0;
}

/*
 * Scan a plain scalar.
 */

static int
cooklang_parser_scan_plain_scalar(cooklang_parser_t *parser, cooklang_token_t *token)
{
    cooklang_mark_t start_mark;
    cooklang_mark_t end_mark;
    cooklang_string_t string = NULL_STRING;
    cooklang_string_t leading_break = NULL_STRING;
    cooklang_string_t trailing_breaks = NULL_STRING;
    cooklang_string_t whitespaces = NULL_STRING;
    int leading_blanks = 0;
    int indent = parser->indent+1;

    if (!STRING_INIT(parser, string, INITIAL_STRING_SIZE)) goto error;
    if (!STRING_INIT(parser, leading_break, INITIAL_STRING_SIZE)) goto error;
    if (!STRING_INIT(parser, trailing_breaks, INITIAL_STRING_SIZE)) goto error;
    if (!STRING_INIT(parser, whitespaces, INITIAL_STRING_SIZE)) goto error;

    start_mark = end_mark = parser->mark;

    /* Consume the content of the plain scalar. */

    while (1)
    {
        /* Check for a document indicator. */

        if (!CACHE(parser, 4)) goto error;

        if (parser->mark.column == 0 &&
            ((CHECK_AT(parser->buffer, '-', 0) &&
              CHECK_AT(parser->buffer, '-', 1) &&
              CHECK_AT(parser->buffer, '-', 2)) ||
             (CHECK_AT(parser->buffer, '.', 0) &&
              CHECK_AT(parser->buffer, '.', 1) &&
              CHECK_AT(parser->buffer, '.', 2))) &&
            IS_BLANKZ_AT(parser->buffer, 3)) break;

        /* Check for a comment. */

        if (CHECK(parser->buffer, '#'))
            break;

        /* Consume non-blank characters. */

        while (!IS_BLANKZ(parser->buffer))
        {
            /* Check for "x:" + one of ',?[]{}' in the flow context. TODO: Fix the test "spec-08-13".
             * This is not completely according to the spec
             * See http://cooklang.org/spec/1.1/#id907281 9.1.3. Plain
             */

            if (parser->flow_level
                    && CHECK(parser->buffer, ':')
                    && (
                        CHECK_AT(parser->buffer, ',', 1)
                        || CHECK_AT(parser->buffer, '?', 1)
                        || CHECK_AT(parser->buffer, '[', 1)
                        || CHECK_AT(parser->buffer, ']', 1)
                        || CHECK_AT(parser->buffer, '{', 1)
                        || CHECK_AT(parser->buffer, '}', 1)
                    )
                    ) {
                cooklang_parser_set_scanner_error(parser, "while scanning a plain scalar",
                        start_mark, "found unexpected ':'");
                goto error;
            }

            /* Check for indicators that may end a plain scalar. */

            if ((CHECK(parser->buffer, ':') && IS_BLANKZ_AT(parser->buffer, 1))
                    || (parser->flow_level &&
                        (CHECK(parser->buffer, ',')
                         || CHECK(parser->buffer, '[')
                         || CHECK(parser->buffer, ']') || CHECK(parser->buffer, '{')
                         || CHECK(parser->buffer, '}'))))
                break;

            /* Check if we need to join whitespaces and breaks. */

            if (leading_blanks || whitespaces.start != whitespaces.pointer)
            {
                if (leading_blanks)
                {
                    /* Do we need to fold line breaks? */

                    if (leading_break.start[0] == '\n') {
                        if (trailing_breaks.start[0] == '\0') {
                            if (!STRING_EXTEND(parser, string)) goto error;
                            *(string.pointer++) = ' ';
                        }
                        else {
                            if (!JOIN(parser, string, trailing_breaks)) goto error;
                            CLEAR(parser, trailing_breaks);
                        }
                        CLEAR(parser, leading_break);
                    }
                    else {
                        if (!JOIN(parser, string, leading_break)) goto error;
                        if (!JOIN(parser, string, trailing_breaks)) goto error;
                        CLEAR(parser, leading_break);
                        CLEAR(parser, trailing_breaks);
                    }

                    leading_blanks = 0;
                }
                else
                {
                    if (!JOIN(parser, string, whitespaces)) goto error;
                    CLEAR(parser, whitespaces);
                }
            }

            /* Copy the character. */

            if (!READ(parser, string)) goto error;

            end_mark = parser->mark;

            if (!CACHE(parser, 2)) goto error;
        }

        /* Is it the end? */

        if (!(IS_BLANK(parser->buffer) || IS_BREAK(parser->buffer)))
            break;

        /* Consume blank characters. */

        if (!CACHE(parser, 1)) goto error;

        while (IS_BLANK(parser->buffer) || IS_BREAK(parser->buffer))
        {
            if (IS_BLANK(parser->buffer))
            {
                /* Check for tab characters that abuse indentation. */

                if (leading_blanks && (int)parser->mark.column < indent
                        && IS_TAB(parser->buffer)) {
                    cooklang_parser_set_scanner_error(parser, "while scanning a plain scalar",
                            start_mark, "found a tab character that violates indentation");
                    goto error;
                }

                /* Consume a space or a tab character. */

                if (!leading_blanks) {
                    if (!READ(parser, whitespaces)) goto error;
                }
                else {
                    SKIP(parser);
                }
            }
            else
            {
                if (!CACHE(parser, 2)) goto error;

                /* Check if it is a first line break. */

                if (!leading_blanks)
                {
                    CLEAR(parser, whitespaces);
                    if (!READ_LINE(parser, leading_break)) goto error;
                    leading_blanks = 1;
                }
                else
                {
                    if (!READ_LINE(parser, trailing_breaks)) goto error;
                }
            }
            if (!CACHE(parser, 1)) goto error;
        }

        /* Check indentation level. */

        if (!parser->flow_level && (int)parser->mark.column < indent)
            break;
    }

    /* Create a token. */

    SCALAR_TOKEN_INIT(*token, string.start, string.pointer-string.start,
            COOKLANG_PLAIN_SCALAR_STYLE, start_mark, end_mark);

    /* Note that we change the 'simple_key_allowed' flag. */

    if (leading_blanks) {
        parser->simple_key_allowed = 1;
    }

    STRING_DEL(parser, leading_break);
    STRING_DEL(parser, trailing_breaks);
    STRING_DEL(parser, whitespaces);

    return 1;

error:
    STRING_DEL(parser, string);
    STRING_DEL(parser, leading_break);
    STRING_DEL(parser, trailing_breaks);
    STRING_DEL(parser, whitespaces);

    return 0;
}

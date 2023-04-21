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

static int
cooklang_parser_fetch_eol(cooklang_parser_t *parser);

static int
cooklang_parser_fetch_whitespace(cooklang_parser_t *parser);

static int
cooklang_parser_fetch_number(cooklang_parser_t *parser);

static int
cooklang_parser_fetch_special_symbol(cooklang_parser_t *parser, cooklang_token_type_t type);

static int
cooklang_parser_fetch_word(cooklang_parser_t *parser);

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

    if (!cooklang_parser_scan_to_next_token(parser)) {
        return 0;
    }

    /*
     * Ensure that the buffer contains at least 2 characters.  2 is the length
     * of the longest indicators ('--' and '[-').
     */

    if (!CACHE(parser, 2))
        return 0;

    /* Is it the end of the stream? */

    if (IS_Z(parser->buffer))
        return cooklang_parser_fetch_stream_end(parser);

    /* Is it a line break? */

    if (IS_BREAK(parser->buffer))
        return cooklang_parser_fetch_eol(parser);

    /* Is it a whitespace? */

    if (IS_BLANK(parser->buffer))
        return cooklang_parser_fetch_whitespace(parser);

    /* Is it a number? */

    if (IS_DIGIT(parser->buffer))
        return cooklang_parser_fetch_number(parser);

    /* Is it a colon symbol? */

    if (CHECK(parser->buffer, ':'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_COLON_TOKEN);

    /* Is it "at" symbol? */

    if (CHECK(parser->buffer, '@'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_AT_TOKEN);

    /* Is it a percent symbol? */

    if (CHECK(parser->buffer, '%'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_PERCENT_TOKEN);

    /* Is it an opening brace symbol? */

    if (CHECK(parser->buffer, '{'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_BRACES_LEFT_TOKEN);

    /* Is it a closing brace symbol? */

    if (CHECK(parser->buffer, '}'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_BRACES_RIGHT_TOKEN);

    /* Is it a pipe symbol? */

    if (CHECK(parser->buffer, '|'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_PIPE_TOKEN);

    /* Is it a larger symbol? */

    if (CHECK(parser->buffer, '>'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_CHEVRON_TOKEN);

    /* Is it a tilde symbol? */

    if (CHECK(parser->buffer, '~'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_TILDE_TOKEN);

    /* Is it a hash symbol? */

    if (CHECK(parser->buffer, '#'))
        return cooklang_parser_fetch_special_symbol(parser,
                COOKLANG_HASH_TOKEN);

//     if CharacterSet.punctuationCharacters.contains(currentCharacter) || CharacterSet.symbols.contains(currentCharacter) {
//         return punctuation()
//     }

    if (IS_ALPHA(parser->buffer))
        return cooklang_parser_fetch_word(parser);


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
    cooklang_token_t token;

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
 * Produce the COOKLANG_EOL_TOKEN token.
 */

static int
cooklang_parser_fetch_eol(cooklang_parser_t *parser)
{
    cooklang_mark_t start_mark, end_mark;
    cooklang_token_t token;

    /* Consume the token. */

    start_mark = parser->mark;

    while (IS_BREAK(parser->buffer)) {
        SKIP_LINE(parser);
    }

    end_mark = parser->mark;

    /* Create the COOKLANG_EOL_TOKEN token. */

    TOKEN_INIT(token, COOKLANG_EOL_TOKEN, start_mark, end_mark);

    /* Append the token to the queue. */

    if (!ENQUEUE(parser, parser->tokens, token))
        return 0;

    return 1;
}



/*
 * Produce the COOKLANG_WHITESPACE_TOKEN token.
 */

static int
cooklang_parser_fetch_whitespace(cooklang_parser_t *parser)
{
    cooklang_mark_t start_mark, end_mark;
    cooklang_token_t token;

    /* Consume the token. */

    start_mark = parser->mark;

    while (CHECK(parser->buffer,' ') ||
            (CHECK(parser->buffer, '\t'))) {
        SKIP(parser);
    }

    end_mark = parser->mark;

    /* Create the COOKLANG_WHITESPACE_TOKEN token. */

    TOKEN_INIT(token, COOKLANG_WHITESPACE_TOKEN, start_mark, end_mark);

    /* Append the token to the queue. */

    if (!ENQUEUE(parser, parser->tokens, token))
        return 0;

    return 1;
}


/*
 * Produce the COOKLANG_INTEGER_TOKEN token.
 */

static int
cooklang_parser_fetch_number(cooklang_parser_t *parser)
{
    cooklang_mark_t start_mark, end_mark;
    cooklang_token_t token;

    /* Consume the token. */

    start_mark = parser->mark;

    while (IS_DIGIT(parser->buffer)) {
        SKIP(parser);
    }

    end_mark = parser->mark;

    /* Create the COOKLANG_INTEGER_TOKEN token. */

    TOKEN_INIT(token, COOKLANG_INTEGER_TOKEN, start_mark, end_mark);

    /* Append the token to the queue. */

    if (!ENQUEUE(parser, parser->tokens, token))
        return 0;

    return 1;
}



/*
 * Produce the COOKLANG_WORD_TOKEN token.
 */

static int
cooklang_parser_fetch_word(cooklang_parser_t *parser)
{
    cooklang_mark_t start_mark, end_mark;
    cooklang_token_t token;

    /* Consume the token. */

    start_mark = parser->mark;

    while (IS_ALPHA(parser->buffer)) {
        SKIP(parser);
    }

    end_mark = parser->mark;

    /* Create the COOKLANG_WORD_TOKEN token. */

    TOKEN_INIT(token, COOKLANG_WORD_TOKEN, start_mark, end_mark);

    /* Append the token to the queue. */

    if (!ENQUEUE(parser, parser->tokens, token))
        return 0;

    return 1;
}
/*
 * Produce the COOKLANG_COLON_TOKEN, COOKLANG_AT_TOKEN, COOKLANG_PERCENT_TOKEN,
 *             COOKLANG_BRACES_LEFT_TOKEN, COOKLANG_BRACES_RIGHT_TOKEN,
 *             COOKLANG_PIPE_TOKEN, COOKLANG_CHEVRON_TOKEN, COOKLANG_TILDE_TOKEN,
 *             COOKLANG_HASH_TOKEN tokens.
 */

static int
cooklang_parser_fetch_special_symbol(cooklang_parser_t *parser, cooklang_token_type_t type)
{
    cooklang_mark_t start_mark, end_mark;
    cooklang_token_t token;

    /* Consume the token. */

    start_mark = parser->mark;
    SKIP(parser);

    end_mark = parser->mark;

    /* Create the COOKLANG_AT_TOKEN token. */

    TOKEN_INIT(token, type, start_mark, end_mark);

    /* Append the token to the queue. */

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

    /* Create the STREAM-END token and append it to the queue. */

    STREAM_END_TOKEN_INIT(token, parser->mark, parser->mark);

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

        /* Eat a comment until a line break. */

        if (!CACHE(parser, 2)) return 0;

        if (CHECK_AT(parser->buffer, '-', 0) && CHECK_AT(parser->buffer, '-', 1)) {
            while (!IS_BREAKZ(parser->buffer)) {
                SKIP(parser);
                if (!CACHE(parser, 1)) return 0;
            }
        }

        /* Eat a comment until a line break. */

        if (!CACHE(parser, 2)) return 0;

        if (CHECK_AT(parser->buffer, '[', 0) && CHECK_AT(parser->buffer, '-', 1)) {
            while (!(CHECK_AT(parser->buffer, '-', 0) && CHECK_AT(parser->buffer, ']', 1))) {

                if (IS_Z_AT(parser->buffer, 0))
                    break;

                if (IS_Z_AT(parser->buffer, 1)) {
                    SKIP(parser);
                    break;
                }

                SKIP(parser);
                if (!CACHE(parser, 2)) return 0;
            }
            /* Eat final "-" and "]". */
            if (CHECK(parser->buffer, '-'))
                SKIP(parser);
            if (CHECK(parser->buffer, ']'))
                SKIP(parser);
        }

        /* We have found a token. */

        break;

    }

    return 1;
}



#include "cooklang_private.h"

/*
 * Peek the next token in the token queue.
 */

#define PEEK_TOKEN(parser)                                                      \
    ((parser->token_available || cooklang_parser_fetch_more_tokens(parser)) ?       \
        parser->tokens.head : NULL)

/*
 * Remove the next token from the queue (must be called after PEEK_TOKEN).
 */

#define SKIP_TOKEN(parser)                                                      \
    (parser->token_available = 0,                                               \
     parser->tokens_parsed ++,                                                  \
     parser->stream_end_produced =                                              \
        (parser->tokens.head->type == COOKLANG_STREAM_END_TOKEN),                   \
     parser->tokens.head ++)

/*
 * Public API declarations.
 */

COOKLANG_DECLARE(int)
cooklang_parser_parse(cooklang_parser_t *parser, cooklang_event_t *event);

/*
 * Error handling.
 */

static int
cooklang_parser_set_parser_error(cooklang_parser_t *parser,
        const char *problem, cooklang_mark_t problem_mark);

static int
cooklang_parser_set_parser_error_context(cooklang_parser_t *parser,
        const char *context, cooklang_mark_t context_mark,
        const char *problem, cooklang_mark_t problem_mark);

/*
 * State functions.
 */

static int
cooklang_parser_state_machine(cooklang_parser_t *parser, cooklang_event_t *event);

static int
cooklang_parser_parse_stream_start(cooklang_parser_t *parser, cooklang_event_t *event);

static int
cooklang_parser_parse_recipe_start(cooklang_parser_t *parser, cooklang_event_t *event,
        int implicit);

static int
cooklang_parser_parse_recipe_content(cooklang_parser_t *parser, cooklang_event_t *event);

static int
cooklang_parser_parse_recipe_end(cooklang_parser_t *parser, cooklang_event_t *event);

static int
cooklang_parser_parse_node(cooklang_parser_t *parser, cooklang_event_t *event,
        int block, int indentless_sequence);

/*
 * Utility functions.
 */


/*
 * Get the next event.
 */

COOKLANG_DECLARE(int)
cooklang_parser_parse(cooklang_parser_t *parser, cooklang_event_t *event)
{
    assert(parser);     /* Non-NULL parser object is expected. */
    assert(event);      /* Non-NULL event object is expected. */

    /* Erase the event object. */

    memset(event, 0, sizeof(cooklang_event_t));

    /* No events after the end of the stream or error. */

    if (parser->stream_end_produced || parser->error ||
            parser->state == COOKLANG_PARSE_END_STATE) {
        return 1;
    }

    /* Generate the next event. */

    return cooklang_parser_state_machine(parser, event);
}

/*
 * Set parser error.
 */

static int
cooklang_parser_set_parser_error(cooklang_parser_t *parser,
        const char *problem, cooklang_mark_t problem_mark)
{
    parser->error = COOKLANG_PARSER_ERROR;
    parser->problem = problem;
    parser->problem_mark = problem_mark;

    return 0;
}

static int
cooklang_parser_set_parser_error_context(cooklang_parser_t *parser,
        const char *context, cooklang_mark_t context_mark,
        const char *problem, cooklang_mark_t problem_mark)
{
    parser->error = COOKLANG_PARSER_ERROR;
    parser->context = context;
    parser->context_mark = context_mark;
    parser->problem = problem;
    parser->problem_mark = problem_mark;

    return 0;
}


/*
 * State dispatcher.
 */

static int
cooklang_parser_state_machine(cooklang_parser_t *parser, cooklang_event_t *event)
{
    switch (parser->state)
    {
        case COOKLANG_PARSE_STREAM_START_STATE:
            return cooklang_parser_parse_stream_start(parser, event);

        case COOKLANG_PARSE_IMPLICIT_RECIPE_START_STATE:
            return cooklang_parser_parse_recipe_start(parser, event, 1);

        case COOKLANG_PARSE_RECIPE_START_STATE:
            return cooklang_parser_parse_recipe_start(parser, event, 0);

        case COOKLANG_PARSE_RECIPE_CONTENT_STATE:
            return cooklang_parser_parse_recipe_content(parser, event);

        case COOKLANG_PARSE_RECIPE_END_STATE:
            return cooklang_parser_parse_recipe_end(parser, event);

        case COOKLANG_PARSE_BLOCK_NODE_STATE:
            return cooklang_parser_parse_node(parser, event, 1, 0);

        case COOKLANG_PARSE_BLOCK_NODE_OR_INDENTLESS_SEQUENCE_STATE:
            return cooklang_parser_parse_node(parser, event, 1, 1);

        case COOKLANG_PARSE_FLOW_NODE_STATE:
            return cooklang_parser_parse_node(parser, event, 0, 0);

        default:
            assert(1);      /* Invalid state. */
    }

    return 0;
}

/*
 * Parse the production:
 * stream   ::= STREAM-START implicit_recipe? explicit_recipe* STREAM-END
 *              ************
 */

static int
cooklang_parser_parse_stream_start(cooklang_parser_t *parser, cooklang_event_t *event)
{
    cooklang_token_t *token;

    token = PEEK_TOKEN(parser);
    if (!token) return 0;

    if (token->type != COOKLANG_STREAM_START_TOKEN) {
        return cooklang_parser_set_parser_error(parser,
                "did not find expected <stream-start>", token->start_mark);
    }

    parser->state = COOKLANG_PARSE_IMPLICIT_RECIPE_START_STATE;
    STREAM_START_EVENT_INIT(*event, token->data.stream_start.encoding,
            token->start_mark, token->start_mark);
    SKIP_TOKEN(parser);

    return 1;
}

/*
 * Parse the productions:
 * implicit_recipe    ::= block_node RECIPE-END*
 *                          *
 * explicit_recipe    ::= DIRECTIVE* RECIPE-START block_node? RECIPE-END*
 *                          *************************
 */

static int
cooklang_parser_parse_recipe_start(cooklang_parser_t *parser, cooklang_event_t *event,
        int implicit)
{
    cooklang_token_t *token;
    cooklang_version_directive_t *version_directive = NULL;
    struct {
        cooklang_tag_directive_t *start;
        cooklang_tag_directive_t *end;
    } tag_directives = { NULL, NULL };

    token = PEEK_TOKEN(parser);
    if (!token) return 0;

    /* Parse extra recipe end indicators. */

    if (!implicit)
    {
        while (token->type == COOKLANG_RECIPE_END_TOKEN) {
            SKIP_TOKEN(parser);
            token = PEEK_TOKEN(parser);
            if (!token) return 0;
        }
    }

    /* Parse an implicit recipe. */

    if (implicit && token->type != COOKLANG_VERSION_DIRECTIVE_TOKEN &&
            token->type != COOKLANG_TAG_DIRECTIVE_TOKEN &&
            token->type != COOKLANG_RECIPE_START_TOKEN &&
            token->type != COOKLANG_STREAM_END_TOKEN)
    {
        if (!cooklang_parser_process_directives(parser, NULL, NULL, NULL))
            return 0;
        if (!PUSH(parser, parser->states, COOKLANG_PARSE_RECIPE_END_STATE))
            return 0;
        parser->state = COOKLANG_PARSE_BLOCK_NODE_STATE;
        RECIPE_START_EVENT_INIT(*event, NULL, NULL, NULL, 1,
                token->start_mark, token->start_mark);
        return 1;
    }

    /* Parse an explicit recipe. */

    else if (token->type != COOKLANG_STREAM_END_TOKEN)
    {
        cooklang_mark_t start_mark, end_mark;
        start_mark = token->start_mark;
        if (!cooklang_parser_process_directives(parser, &version_directive,
                    &tag_directives.start, &tag_directives.end))
            return 0;
        token = PEEK_TOKEN(parser);
        if (!token) goto error;
        if (token->type != COOKLANG_RECIPE_START_TOKEN) {
            cooklang_parser_set_parser_error(parser,
                    "did not find expected <recipe start>", token->start_mark);
            goto error;
        }
        if (!PUSH(parser, parser->states, COOKLANG_PARSE_RECIPE_END_STATE))
            goto error;
        parser->state = COOKLANG_PARSE_RECIPE_CONTENT_STATE;
        end_mark = token->end_mark;
        RECIPE_START_EVENT_INIT(*event, version_directive,
                tag_directives.start, tag_directives.end, 0,
                start_mark, end_mark);
        SKIP_TOKEN(parser);
        version_directive = NULL;
        tag_directives.start = tag_directives.end = NULL;
        return 1;
    }

    /* Parse the stream end. */

    else
    {
        parser->state = COOKLANG_PARSE_END_STATE;
        STREAM_END_EVENT_INIT(*event, token->start_mark, token->end_mark);
        SKIP_TOKEN(parser);
        return 1;
    }

error:
    cooklang_free(version_directive);
    while (tag_directives.start != tag_directives.end) {
        cooklang_free(tag_directives.end[-1].handle);
        cooklang_free(tag_directives.end[-1].prefix);
        tag_directives.end --;
    }
    cooklang_free(tag_directives.start);
    return 0;
}

/*
 * Parse the productions:
 * explicit_recipe    ::= DIRECTIVE* RECIPE-START block_node? RECIPE-END*
 *                                                    ***********
 */

static int
cooklang_parser_parse_recipe_content(cooklang_parser_t *parser, cooklang_event_t *event)
{
    cooklang_token_t *token;

    token = PEEK_TOKEN(parser);
    if (!token) return 0;

    if (token->type == COOKLANG_VERSION_DIRECTIVE_TOKEN ||
            token->type == COOKLANG_TAG_DIRECTIVE_TOKEN ||
            token->type == COOKLANG_RECIPE_START_TOKEN ||
            token->type == COOKLANG_RECIPE_END_TOKEN ||
            token->type == COOKLANG_STREAM_END_TOKEN) {
        parser->state = POP(parser, parser->states);
        return cooklang_parser_process_empty_scalar(parser, event,
                token->start_mark);
    }
    else {
        return cooklang_parser_parse_node(parser, event, 1, 0);
    }
}

/*
 * Parse the productions:
 * implicit_recipe    ::= block_node RECIPE-END*
 *                                     *************
 * explicit_recipe    ::= DIRECTIVE* RECIPE-START block_node? RECIPE-END*
 *                                                                *************
 */

static int
cooklang_parser_parse_recipe_end(cooklang_parser_t *parser, cooklang_event_t *event)
{
    cooklang_token_t *token;
    cooklang_mark_t start_mark, end_mark;
    int implicit = 1;

    token = PEEK_TOKEN(parser);
    if (!token) return 0;

    start_mark = end_mark = token->start_mark;

    if (token->type == COOKLANG_RECIPE_END_TOKEN) {
        end_mark = token->end_mark;
        SKIP_TOKEN(parser);
        implicit = 0;
    }

    while (!STACK_EMPTY(parser, parser->tag_directives)) {
        cooklang_tag_directive_t tag_directive = POP(parser, parser->tag_directives);
        cooklang_free(tag_directive.handle);
        cooklang_free(tag_directive.prefix);
    }

    parser->state = COOKLANG_PARSE_RECIPE_START_STATE;
    RECIPE_END_EVENT_INIT(*event, implicit, start_mark, end_mark);

    return 1;
}



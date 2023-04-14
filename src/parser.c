
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

    // parser->state = COOKLANG_PARSE_IMPLICIT_RECIPE_START_STATE;
    STREAM_START_EVENT_INIT(*event, token->data.stream_start.encoding,
            token->start_mark, token->start_mark);
    SKIP_TOKEN(parser);

    return 1;
}





#include "cooklang_private.h"

/*
 * API functions.
 */

COOKLANG_DECLARE(int)
cooklang_parser_load(cooklang_parser_t *parser, cooklang_recipe_t *recipe);

/*
 * Error handling.
 */

static int
cooklang_parser_set_composer_error(cooklang_parser_t *parser,
        const char *problem, cooklang_mark_t problem_mark);

static int
cooklang_parser_set_composer_error_context(cooklang_parser_t *parser,
        const char *context, cooklang_mark_t context_mark,
        const char *problem, cooklang_mark_t problem_mark);


/*
 * Document loading context.
 */
struct loader_ctx {
    int *start;
    int *end;
    int *top;
};


/*
 * Load the next recipe of the stream.
 */

COOKLANG_DECLARE(int)
cooklang_parser_load(cooklang_parser_t *parser, cooklang_recipe_t *recipe)
{
    cooklang_event_t event;

    assert(parser);     /* Non-NULL parser object is expected. */
    assert(recipe);   /* Non-NULL recipe object is expected. */

    memset(recipe, 0, sizeof(cooklang_recipe_t));

    if (!parser->stream_start_produced) {
        if (!cooklang_parser_parse(parser, &event)) goto error;
        assert(event.type == COOKLANG_STREAM_START_EVENT);
                        /* STREAM-START is expected. */
    }

    if (parser->stream_end_produced) {
        return 1;
    }

    if (!cooklang_parser_parse(parser, &event)) goto error;
    if (event.type == COOKLANG_STREAM_END_EVENT) {
        return 1;
    }

    parser->recipe = recipe;

    parser->recipe = NULL;

    return 1;

error:

    parser->recipe = NULL;

    return 0;
}

/*
 * Set composer error.
 */

static int
cooklang_parser_set_composer_error(cooklang_parser_t *parser,
        const char *problem, cooklang_mark_t problem_mark)
{
    parser->error = COOKLANG_COMPOSER_ERROR;
    parser->problem = problem;
    parser->problem_mark = problem_mark;

    return 0;
}

/*
 * Set composer error with context.
 */

static int
cooklang_parser_set_composer_error_context(cooklang_parser_t *parser,
        const char *context, cooklang_mark_t context_mark,
        const char *problem, cooklang_mark_t problem_mark)
{
    parser->error = COOKLANG_COMPOSER_ERROR;
    parser->context = context;
    parser->context_mark = context_mark;
    parser->problem = problem;
    parser->problem_mark = problem_mark;

    return 0;
}


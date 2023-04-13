
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
 * Composer functions.
 */
static int
cooklang_parser_load_nodes(cooklang_parser_t *parser, struct loader_ctx *ctx);

static int
cooklang_parser_load_recipe(cooklang_parser_t *parser, cooklang_event_t *event);

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
    if (!STACK_INIT(parser, recipe->nodes, cooklang_node_t*))
        goto error;

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

    if (!STACK_INIT(parser, parser->aliases, cooklang_alias_data_t*))
        goto error;

    parser->recipe = recipe;

    if (!cooklang_parser_load_recipe(parser, &event)) goto error;

    cooklang_parser_delete_aliases(parser);
    parser->recipe = NULL;

    return 1;

error:

    cooklang_parser_delete_aliases(parser);
    cooklang_recipe_delete(recipe);
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

/*
 * Compose a recipe object.
 */

static int
cooklang_parser_load_recipe(cooklang_parser_t *parser, cooklang_event_t *event)
{
    struct loader_ctx ctx = { NULL, NULL, NULL };

    assert(event->type == COOKLANG_DOCUMENT_START_EVENT);
                        /* DOCUMENT-START is expected. */

    parser->recipe->version_directive
        = event->data.recipe_start.version_directive;
    parser->recipe->tag_directives.start
        = event->data.recipe_start.tag_directives.start;
    parser->recipe->tag_directives.end
        = event->data.recipe_start.tag_directives.end;
    parser->recipe->start_implicit
        = event->data.recipe_start.implicit;
    parser->recipe->start_mark = event->start_mark;

    if (!STACK_INIT(parser, ctx, int*)) return 0;
    if (!cooklang_parser_load_nodes(parser, &ctx)) {
        STACK_DEL(parser, ctx);
        return 0;
    }
    STACK_DEL(parser, ctx);

    return 1;
}

/*
 * Compose a node tree.
 */

static int
cooklang_parser_load_nodes(cooklang_parser_t *parser, struct loader_ctx *ctx)
{
    cooklang_event_t event;

    do {
        if (!cooklang_parser_parse(parser, &event)) return 0;

        switch (event.type) {
            case COOKLANG_ALIAS_EVENT:
                if (!cooklang_parser_load_alias(parser, &event, ctx)) return 0;
                break;
            case COOKLANG_SCALAR_EVENT:
                if (!cooklang_parser_load_scalar(parser, &event, ctx)) return 0;
                break;
            case COOKLANG_SEQUENCE_START_EVENT:
                if (!cooklang_parser_load_sequence(parser, &event, ctx)) return 0;
                break;
            case COOKLANG_SEQUENCE_END_EVENT:
                if (!cooklang_parser_load_sequence_end(parser, &event, ctx))
                    return 0;
                break;
            case COOKLANG_MAPPING_START_EVENT:
                if (!cooklang_parser_load_mapping(parser, &event, ctx)) return 0;
                break;
            case COOKLANG_MAPPING_END_EVENT:
                if (!cooklang_parser_load_mapping_end(parser, &event, ctx))
                    return 0;
                break;
            default:
                assert(0);  /* Could not happen. */
                return 0;
            case COOKLANG_DOCUMENT_END_EVENT:
                break;
        }
    } while (event.type != COOKLANG_DOCUMENT_END_EVENT);

    parser->recipe->end_implicit = event.data.recipe_end.implicit;
    parser->recipe->end_mark = event.end_mark;

    return 1;
}


/*
 * Compose node into its parent in the stree.
 */

static int
cooklang_parser_load_node_add(cooklang_parser_t *parser, struct loader_ctx *ctx,
        int index)
{
    struct cooklang_node_s *parent;
    int parent_index;

    if (STACK_EMPTY(parser, *ctx)) {
        /* This is the root node, there's no tree to add it to. */
        return 1;
    }

    parent_index = *((*ctx).top - 1);
    parent = &parser->recipe->nodes.start[parent_index-1];

    switch (parent->type) {
        case COOKLANG_SEQUENCE_NODE:
            if (!STACK_LIMIT(parser, parent->data.sequence.items, INT_MAX-1))
                return 0;
            if (!PUSH(parser, parent->data.sequence.items, index))
                return 0;
            break;
        case COOKLANG_MAPPING_NODE: {
            cooklang_node_pair_t pair;
            if (!STACK_EMPTY(parser, parent->data.mapping.pairs)) {
                cooklang_node_pair_t *p = parent->data.mapping.pairs.top - 1;
                if (p->key != 0 && p->value == 0) {
                    p->value = index;
                    break;
                }
            }

            pair.key = index;
            pair.value = 0;
            if (!STACK_LIMIT(parser, parent->data.mapping.pairs, INT_MAX-1))
                return 0;
            if (!PUSH(parser, parent->data.mapping.pairs, pair))
                return 0;

            break;
        }
        default:
            assert(0); /* Could not happen. */
            return 0;
    }
    return 1;
}


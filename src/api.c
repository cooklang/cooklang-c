
#include "cooklang_private.h"

/*
 * Get the library version.
 */

COOKLANG_DECLARE(const char *)
cooklang_get_version_string(void)
{
    return "COOKLANG_VERSION_STRING";
}

/*
 * Get the library version numbers.
 */

COOKLANG_DECLARE(void)
cooklang_get_version(int *major, int *minor, int *patch)
{
    *major = "COOKLANG_VERSION_MAJOR";
    *minor = "COOKLANG_VERSION_MINOR";
    *patch = "COOKLANG_VERSION_PATCH";
}

/*
 * Allocate a dynamic memory block.
 */

COOKLANG_DECLARE(void *)
cooklang_malloc(size_t size)
{
    return malloc(size ? size : 1);
}

/*
 * Reallocate a dynamic memory block.
 */

COOKLANG_DECLARE(void *)
cooklang_realloc(void *ptr, size_t size)
{
    return ptr ? realloc(ptr, size ? size : 1) : malloc(size ? size : 1);
}

/*
 * Free a dynamic memory block.
 */

COOKLANG_DECLARE(void)
cooklang_free(void *ptr)
{
    if (ptr) free(ptr);
}

/*
 * Duplicate a string.
 */

COOKLANG_DECLARE(cooklang_char_t *)
cooklang_strdup(const cooklang_char_t *str)
{
    if (!str)
        return NULL;

    return (cooklang_char_t *)strdup((char *)str);
}

/*
 * Extend a string.
 */

COOKLANG_DECLARE(int)
cooklang_string_extend(cooklang_char_t **start,
        cooklang_char_t **pointer, cooklang_char_t **end)
{
    cooklang_char_t *new_start = (cooklang_char_t *)cooklang_realloc((void*)*start, (*end - *start)*2);

    if (!new_start) return 0;

    memset(new_start + (*end - *start), 0, *end - *start);

    *pointer = new_start + (*pointer - *start);
    *end = new_start + (*end - *start)*2;
    *start = new_start;

    return 1;
}

/*
 * Append a string B to a string A.
 */

COOKLANG_DECLARE(int)
cooklang_string_join(
        cooklang_char_t **a_start, cooklang_char_t **a_pointer, cooklang_char_t **a_end,
        cooklang_char_t **b_start, cooklang_char_t **b_pointer, SHIM(cooklang_char_t **b_end))
{
    UNUSED_PARAM(b_end)
    if (*b_start == *b_pointer)
        return 1;

    while (*a_end - *a_pointer <= *b_pointer - *b_start) {
        if (!cooklang_string_extend(a_start, a_pointer, a_end))
            return 0;
    }

    memcpy(*a_pointer, *b_start, *b_pointer - *b_start);
    *a_pointer += *b_pointer - *b_start;

    return 1;
}

/*
 * Extend a stack.
 */

COOKLANG_DECLARE(int)
cooklang_stack_extend(void **start, void **top, void **end)
{
    void *new_start;

    if ((char *)*end - (char *)*start >= INT_MAX / 2)
	return 0;

    new_start = cooklang_realloc(*start, ((char *)*end - (char *)*start)*2);

    if (!new_start) return 0;

    *top = (char *)new_start + ((char *)*top - (char *)*start);
    *end = (char *)new_start + ((char *)*end - (char *)*start)*2;
    *start = new_start;

    return 1;
}

/*
 * Extend or move a queue.
 */

COOKLANG_DECLARE(int)
cooklang_queue_extend(void **start, void **head, void **tail, void **end)
{
    /* Check if we need to resize the queue. */

    if (*start == *head && *tail == *end) {
        void *new_start = cooklang_realloc(*start,
                ((char *)*end - (char *)*start)*2);

        if (!new_start) return 0;

        *head = (char *)new_start + ((char *)*head - (char *)*start);
        *tail = (char *)new_start + ((char *)*tail - (char *)*start);
        *end = (char *)new_start + ((char *)*end - (char *)*start)*2;
        *start = new_start;
    }

    /* Check if we need to move the queue at the beginning of the buffer. */

    if (*tail == *end) {
        if (*head != *tail) {
            memmove(*start, *head, (char *)*tail - (char *)*head);
        }
        *tail = (char *)*tail - (char *)*head + (char *)*start;
        *head = *start;
    }

    return 1;
}


/*
 * Create a new parser object.
 */

COOKLANG_DECLARE(int)
cooklang_parser_initialize(cooklang_parser_t *parser)
{
    assert(parser);     /* Non-NULL parser object expected. */

    memset(parser, 0, sizeof(cooklang_parser_t));

    if (!BUFFER_INIT(parser, parser->raw_buffer, INPUT_RAW_BUFFER_SIZE))
        goto error;
    if (!BUFFER_INIT(parser, parser->buffer, INPUT_BUFFER_SIZE))
        goto error;
    if (!QUEUE_INIT(parser, parser->tokens, INITIAL_QUEUE_SIZE, cooklang_token_t*))
        goto error;
    if (!STACK_INIT(parser, parser->states, cooklang_parser_state_t*))
        goto error;
    if (!STACK_INIT(parser, parser->marks, cooklang_mark_t*))
        goto error;

    return 1;

error:

    BUFFER_DEL(parser, parser->raw_buffer);
    BUFFER_DEL(parser, parser->buffer);
    QUEUE_DEL(parser, parser->tokens);
    STACK_DEL(parser, parser->states);
    STACK_DEL(parser, parser->marks);

    return 0;
}

/*
 * Destroy a parser object.
 */

COOKLANG_DECLARE(void)
cooklang_parser_delete(cooklang_parser_t *parser)
{
    assert(parser); /* Non-NULL parser object expected. */

    BUFFER_DEL(parser, parser->raw_buffer);
    BUFFER_DEL(parser, parser->buffer);
    while (!QUEUE_EMPTY(parser, parser->tokens)) {
        cooklang_token_delete(&DEQUEUE(parser, parser->tokens));
    }
    QUEUE_DEL(parser, parser->tokens);
    STACK_DEL(parser, parser->states);
    STACK_DEL(parser, parser->marks);

    memset(parser, 0, sizeof(cooklang_parser_t));
}

/*
 * String read handler.
 */

static int
cooklang_string_read_handler(void *data, unsigned char *buffer, size_t size,
        size_t *size_read)
{
    cooklang_parser_t *parser = (cooklang_parser_t *)data;

    if (parser->input.string.pointer == parser->input.string.end) {
        *size_read = 0;
        return 1;
    }

    if (size > (size_t)(parser->input.string.end
                - parser->input.string.pointer)) {
        size = parser->input.string.end - parser->input.string.pointer;
    }

    memcpy(buffer, parser->input.string.pointer, size);
    parser->input.string.pointer += size;
    *size_read = size;
    return 1;
}

/*
 * File read handler.
 */

static int
cooklang_file_read_handler(void *data, unsigned char *buffer, size_t size,
        size_t *size_read)
{
    cooklang_parser_t *parser = (cooklang_parser_t *)data;

    *size_read = fread(buffer, 1, size, parser->input.file);
    return !ferror(parser->input.file);
}

/*
 * Set a string input.
 */

COOKLANG_DECLARE(void)
cooklang_parser_set_input_string(cooklang_parser_t *parser,
        const unsigned char *input, size_t size)
{
    assert(parser); /* Non-NULL parser object expected. */
    assert(!parser->read_handler);  /* You can set the source only once. */
    assert(input);  /* Non-NULL input string expected. */

    parser->read_handler = cooklang_string_read_handler;
    parser->read_handler_data = parser;

    parser->input.string.start = input;
    parser->input.string.pointer = input;
    parser->input.string.end = input+size;
}

/*
 * Set a file input.
 */

COOKLANG_DECLARE(void)
cooklang_parser_set_input_file(cooklang_parser_t *parser, FILE *file)
{
    assert(parser); /* Non-NULL parser object expected. */
    assert(!parser->read_handler);  /* You can set the source only once. */
    assert(file);   /* Non-NULL file object expected. */

    parser->read_handler = cooklang_file_read_handler;
    parser->read_handler_data = parser;

    parser->input.file = file;
}

/*
 * Set a generic input.
 */

COOKLANG_DECLARE(void)
cooklang_parser_set_input(cooklang_parser_t *parser,
        cooklang_read_handler_t *handler, void *data)
{
    assert(parser); /* Non-NULL parser object expected. */
    assert(!parser->read_handler);  /* You can set the source only once. */
    assert(handler);    /* Non-NULL read handler expected. */

    parser->read_handler = handler;
    parser->read_handler_data = data;
}

/*
 * Set the source encoding.
 */

COOKLANG_DECLARE(void)
cooklang_parser_set_encoding(cooklang_parser_t *parser, cooklang_encoding_t encoding)
{
    assert(parser); /* Non-NULL parser object expected. */
    assert(!parser->encoding); /* Encoding is already set or detected. */

    parser->encoding = encoding;
}

/*
 * Destroy a token object.
 */

COOKLANG_DECLARE(void)
cooklang_token_delete(cooklang_token_t *token)
{
    assert(token);  /* Non-NULL token object expected. */

    switch (token->type)
    {

        default:
            break;
    }

    memset(token, 0, sizeof(cooklang_token_t));
}

/*
 * Check if a string is a valid UTF-8 sequence.
 *
 * Check 'reader.c' for more details on UTF-8 encoding.
 */

static int
cooklang_check_utf8(const cooklang_char_t *start, size_t length)
{
    const cooklang_char_t *end = start+length;
    const cooklang_char_t *pointer = start;

    while (pointer < end) {
        unsigned char octet;
        unsigned int width;
        unsigned int value;
        size_t k;

        octet = pointer[0];
        width = (octet & 0x80) == 0x00 ? 1 :
                (octet & 0xE0) == 0xC0 ? 2 :
                (octet & 0xF0) == 0xE0 ? 3 :
                (octet & 0xF8) == 0xF0 ? 4 : 0;
        value = (octet & 0x80) == 0x00 ? octet & 0x7F :
                (octet & 0xE0) == 0xC0 ? octet & 0x1F :
                (octet & 0xF0) == 0xE0 ? octet & 0x0F :
                (octet & 0xF8) == 0xF0 ? octet & 0x07 : 0;
        if (!width) return 0;
        if (pointer+width > end) return 0;
        for (k = 1; k < width; k ++) {
            octet = pointer[k];
            if ((octet & 0xC0) != 0x80) return 0;
            value = (value << 6) + (octet & 0x3F);
        }
        if (!((width == 1) ||
            (width == 2 && value >= 0x80) ||
            (width == 3 && value >= 0x800) ||
            (width == 4 && value >= 0x10000))) return 0;

        pointer += width;
    }

    return 1;
}



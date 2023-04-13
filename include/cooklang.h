/**
 * @file cooklang.h
 * @brief Public interface for libcooklang.
 *
 * Include the header file with the code:
 * @code
 * #include <cooklang.h>
 * @endcode
 */

#ifndef COOKLANG_H
#define COOKLANG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @defgroup export Export Definitions
 * @{
 */

/** The public API declaration. */

#if defined(__MINGW32__)
#   define  COOKLANG_DECLARE(type)  type
#elif defined(_WIN32)
#   if defined(COOKLANG_DECLARE_STATIC)
#       define  COOKLANG_DECLARE(type)  type
#   elif defined(COOKLANG_DECLARE_EXPORT)
#       define  COOKLANG_DECLARE(type)  __declspec(dllexport) type
#   else
#       define  COOKLANG_DECLARE(type)  __declspec(dllimport) type
#   endif
#else
#   define  COOKLANG_DECLARE(type)  type
#endif

/** @} */


/**
 * @defgroup version Version Information
 * @{
 */

/**
 * Get the library version as a string.
 *
 * @returns The function returns the pointer to a static string of the form
 * @c "X.Y.Z", where @c X is the major version number, @c Y is a minor version
 * number, and @c Z is the patch version number.
 */

COOKLANG_DECLARE(const char *)
cooklang_get_version_string(void);

/**
 * Get the library version numbers.
 *
 * @param[out]      major   Major version number.
 * @param[out]      minor   Minor version number.
 * @param[out]      patch   Patch version number.
 */

COOKLANG_DECLARE(void)
cooklang_get_version(int *major, int *minor, int *patch);

/** @} */



/**
 * @defgroup basic Basic Types
 * @{
 */

/** The character type (UTF-8 octet). */
typedef unsigned char cooklang_char_t;

/** The stream encoding. */
typedef enum cooklang_encoding_e {
    /** Let the parser choose the encoding. */
    COOKLANG_ANY_ENCODING,
    /** The default UTF-8 encoding. */
    COOKLANG_UTF8_ENCODING,
    /** The UTF-16-LE encoding with BOM. */
    COOKLANG_UTF16LE_ENCODING,
    /** The UTF-16-BE encoding with BOM. */
    COOKLANG_UTF16BE_ENCODING
} cooklang_encoding_t;

/** Line break types. */

typedef enum cooklang_break_e {
    /** Let the parser choose the break type. */
    COOKLANG_ANY_BREAK,
    /** Use CR for line breaks (Mac style). */
    COOKLANG_CR_BREAK,
    /** Use LN for line breaks (Unix style). */
    COOKLANG_LN_BREAK,
    /** Use CR LN for line breaks (DOS style). */
    COOKLANG_CRLN_BREAK
} cooklang_break_t;


/** Many bad things could happen with the parser and emitter. */
typedef enum cooklang_error_type_e {
    /** No error is produced. */
    COOKLANG_NO_ERROR,

    /** Cannot allocate or reallocate a block of memory. */
    COOKLANG_MEMORY_ERROR,

    /** Cannot read or decode the input stream. */
    COOKLANG_READER_ERROR,
    /** Cannot scan the input stream. */
    COOKLANG_SCANNER_ERROR,
    /** Cannot parse the input stream. */
    COOKLANG_PARSER_ERROR,
    /** Cannot compose a Cooklang recipe. */
    COOKLANG_COMPOSER_ERROR,

    /** Cannot write to the output stream. */
    COOKLANG_WRITER_ERROR,
    /** Cannot emit a Cooklang stream. */
    COOKLANG_EMITTER_ERROR
} cooklang_error_type_t;


/** The pointer position. */
typedef struct cooklang_mark_s {
    /** The position index. */
    size_t index;

    /** The position line. */
    size_t line;

    /** The position column. */
    size_t column;
} cooklang_mark_t;

/** @} */



/**
 * @defgroup tokens Tokens
 * @{
 */

typedef enum cooklang_token_type_e {
    COOKLANG_WORD_TOKEN,
    COOKLANG_INTEGER_TOKEN,
    COOKLANG_DECIMAL_TOKEN,
    COOKLANG_FRACTIONAL_TOKEN,
    COOKLANG_WHOLE_FRACTIONAL_TOKEN,
    COOKLANG_WHITESPACE_TOKEN,
    COOKLANG_COLON_TOKEN,
    COOKLANG_AT_TOKEN,
    COOKLANG_PERCENT_TOKEN,
    COOKLANG_BRACES_LEFT_TOKEN,
    COOKLANG_BRACES_RIGHT_TOKEN,
    COOKLANG_PIPE_TOKEN,
    COOKLANG_CHEVRON_TOKEN,
    COOKLANG_TILDE_TOKEN,
    COOKLANG_HASH_TOKEN,
    COOKLANG_EOL_TOKEN,
    COOKLANG_EOF_TOKEN,
    COOKLANG_COUNT_TOKEN
} cooklang_token_type_t;


/** The token structure. */
typedef struct cooklang_token_s {

    /** The token type. */
    cooklang_token_type_t type;

    /** The token data. */
    union {

        /** The stream start (for @c COOKLANG_STREAM_START_TOKEN). */
        struct {
            /** The stream encoding. */
            cooklang_encoding_t encoding;
        } stream_start;

    } data;

    /** The beginning of the token. */
    cooklang_mark_t start_mark;
    /** The end of the token. */
    cooklang_mark_t end_mark;

} cooklang_token_t;



/**
 * Convert a COOKLANG type into a human readable string.
 *
 * @param[in]  type  The state to convert.
 * @return String representing state.
 */

static inline const char * cooklang_token_type_to_string(cooklang_token_type_t type)
{
    static const char * const strings[COOKLANG_COUNT_TOKEN] = {
        [COOKLANG_WORD_TOKEN]             = "WORD_TOKEN",
        [COOKLANG_INTEGER_TOKEN]          = "INTEGER_TOKEN",
        [COOKLANG_DECIMAL_TOKEN]          = "DECIMAL_TOKEN",
        [COOKLANG_FRACTIONAL_TOKEN]       = "FRACTIONAL_TOKEN",
        [COOKLANG_WHOLE_FRACTIONAL_TOKEN] = "WHOLE_FRACTIONAL_TOKEN",
        [COOKLANG_WHITESPACE_TOKEN]       = "WHITESPACE_TOKEN",
        [COOKLANG_COLON_TOKEN]            = "COLON_TOKEN",
        [COOKLANG_AT_TOKEN]               = "AT_TOKEN",
        [COOKLANG_PERCENT_TOKEN]          = "PERCENT_TOKEN",
        [COOKLANG_BRACES_LEFT_TOKEN]      = "BRACES_LEFT_TOKEN",
        [COOKLANG_BRACES_RIGHT_TOKEN]     = "BRACES_RIGHT_TOKEN",
        [COOKLANG_PIPE_TOKEN]             = "PIPE_TOKEN",
        [COOKLANG_CHEVRON_TOKEN]          = "CHEVRON_TOKEN",
        [COOKLANG_TILDE_TOKEN]            = "TILDE_TOKEN",
        [COOKLANG_HASH_TOKEN]             = "HASH_TOKEN",
        [COOKLANG_EOL_TOKEN]              = "EOL_TOKEN",
        [COOKLANG_EOF_TOKEN]              = "EOF_TOKEN",
    };
    if ((int)type >= COOKLANG_COUNT_TOKEN) {
        return "<invalid>";
    }
    return strings[type];
}


/**
 * Free any memory allocated for a token object.
 *
 * @param[in,out]   token   A token object.
 */

COOKLANG_DECLARE(void)
cooklang_token_delete(cooklang_token_t *token);

/** @} */



/**
 * @defgroup events Events
 * @{
 */

/** Event types. */
typedef enum cooklang_event_type_e {
    /** An empty event. */
    COOKLANG_NO_EVENT,

    /** A STREAM-START event. */
    COOKLANG_STREAM_START_EVENT,
    /** A STREAM-END event. */
    COOKLANG_STREAM_END_EVENT,

    /** A DOCUMENT-START event. */
    COOKLANG_DOCUMENT_START_EVENT,
    /** A DOCUMENT-END event. */
    COOKLANG_DOCUMENT_END_EVENT,

    /** An ALIAS event. */
    COOKLANG_ALIAS_EVENT,
    /** A SCALAR event. */
    COOKLANG_SCALAR_EVENT,

    /** A SEQUENCE-START event. */
    COOKLANG_SEQUENCE_START_EVENT,
    /** A SEQUENCE-END event. */
    COOKLANG_SEQUENCE_END_EVENT,

    /** A MAPPING-START event. */
    COOKLANG_MAPPING_START_EVENT,
    /** A MAPPING-END event. */
    COOKLANG_MAPPING_END_EVENT
} cooklang_event_type_t;

/** The event structure. */
typedef struct cooklang_event_s {

    /** The event type. */
    cooklang_event_type_t type;

    /** The event data. */
    union {

        /** The stream parameters (for @c COOKLANG_STREAM_START_EVENT). */
        struct {
            /** The recipe encoding. */
            cooklang_encoding_t encoding;
        } stream_start;

        /** The recipe parameters (for @c COOKLANG_DOCUMENT_START_EVENT). */
        struct {

            /** Is the recipe indicator implicit? */
            int implicit;
        } recipe_start;

        /** The recipe end parameters (for @c COOKLANG_DOCUMENT_END_EVENT). */
        struct {
            /** Is the recipe end indicator implicit? */
            int implicit;
        } recipe_end;

    } data;

    /** The beginning of the event. */
    cooklang_mark_t start_mark;
    /** The end of the event. */
    cooklang_mark_t end_mark;
} cooklang_event_t;

/** @} */



/**
 * @defgroup parser Parser Definitions
 * @{
 */

/**
 * The prototype of a read handler.
 *
 * The read handler is called when the parser needs to read more bytes from the
 * source.  The handler should write not more than @a size bytes to the @a
 * buffer.  The number of written bytes should be set to the @a length variable.
 *
 * @param[in,out]   data        A pointer to an application data specified by
 *                              cooklang_parser_set_input().
 * @param[out]      buffer      The buffer to write the data from the source.
 * @param[in]       size        The size of the buffer.
 * @param[out]      size_read   The actual number of bytes read from the source.
 *
 * @returns On success, the handler should return @c 1.  If the handler failed,
 * the returned value should be @c 0.  On EOF, the handler should set the
 * @a size_read to @c 0 and return @c 1.
 */

typedef int cooklang_read_handler_t(void *data, unsigned char *buffer, size_t size,
        size_t *size_read);


/**
 * The states of the parser.
 */
typedef enum cooklang_parser_state_e {
    /** Expect STREAM-START. */
    COOKLANG_PARSE_STREAM_START_STATE,
    /** Expect the beginning of an implicit recipe. */
    COOKLANG_PARSE_IMPLICIT_DOCUMENT_START_STATE,
    /** Expect DOCUMENT-START. */
    COOKLANG_PARSE_DOCUMENT_START_STATE,
    /** Expect the content of a recipe. */
    COOKLANG_PARSE_DOCUMENT_CONTENT_STATE,
    /** Expect DOCUMENT-END. */
    COOKLANG_PARSE_DOCUMENT_END_STATE,

    /** Expect a block node. */
    COOKLANG_PARSE_BLOCK_NODE_STATE,
    /** Expect a block node or indentless sequence. */
    COOKLANG_PARSE_BLOCK_NODE_OR_INDENTLESS_SEQUENCE_STATE,
    /** Expect a flow node. */
    COOKLANG_PARSE_FLOW_NODE_STATE,
    /** Expect the first entry of a block sequence. */
    COOKLANG_PARSE_BLOCK_SEQUENCE_FIRST_ENTRY_STATE,
    /** Expect an entry of a block sequence. */
    COOKLANG_PARSE_BLOCK_SEQUENCE_ENTRY_STATE,

    /** Expect an entry of an indentless sequence. */
    COOKLANG_PARSE_INDENTLESS_SEQUENCE_ENTRY_STATE,
    /** Expect the first key of a block mapping. */
    COOKLANG_PARSE_BLOCK_MAPPING_FIRST_KEY_STATE,
    /** Expect a block mapping key. */
    COOKLANG_PARSE_BLOCK_MAPPING_KEY_STATE,
    /** Expect a block mapping value. */
    COOKLANG_PARSE_BLOCK_MAPPING_VALUE_STATE,
    /** Expect the first entry of a flow sequence. */
    COOKLANG_PARSE_FLOW_SEQUENCE_FIRST_ENTRY_STATE,

    /** Expect an entry of a flow sequence. */
    COOKLANG_PARSE_FLOW_SEQUENCE_ENTRY_STATE,
    /** Expect a key of an ordered mapping. */
    COOKLANG_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_KEY_STATE,
    /** Expect a value of an ordered mapping. */
    COOKLANG_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_VALUE_STATE,
    /** Expect the and of an ordered mapping entry. */
    COOKLANG_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_END_STATE,
    /** Expect the first key of a flow mapping. */
    COOKLANG_PARSE_FLOW_MAPPING_FIRST_KEY_STATE,
    /** Expect a key of a flow mapping. */

    COOKLANG_PARSE_FLOW_MAPPING_KEY_STATE,
    /** Expect a value of a flow mapping. */
    COOKLANG_PARSE_FLOW_MAPPING_VALUE_STATE,
    /** Expect an empty value of a flow mapping. */
    COOKLANG_PARSE_FLOW_MAPPING_EMPTY_VALUE_STATE,
    /** Expect nothing. */
    COOKLANG_PARSE_END_STATE
} cooklang_parser_state_t;



/** The recipe structure. */
typedef struct cooklang_recipe_s {

    // /** The recipe nodes. */
    // struct {
    //     /** The beginning of the stack. */
    //     cooklang_node_t *start;
    //     /** The end of the stack. */
    //     cooklang_node_t *end;
    //     /** The top of the stack. */
    //     cooklang_node_t *top;
    // } nodes;



    /** The beginning of the recipe. */
    cooklang_mark_t start_mark;
    /** The end of the recipe. */
    cooklang_mark_t end_mark;

} cooklang_recipe_t;


/**
 * The parser structure.
 *
 * All members are internal.  Manage the structure using the @c cooklang_parser_
 * family of functions.
 */

typedef struct cooklang_parser_s {

    /**
     * @name Error handling
     * @{
     */

    /** Error type. */
    cooklang_error_type_t error;
    /** Error description. */
    const char *problem;
    /** The byte about which the problem occured. */
    size_t problem_offset;
    /** The problematic value (@c -1 is none). */
    int problem_value;
    /** The problem position. */
    cooklang_mark_t problem_mark;
    /** The error context. */
    const char *context;
    /** The context position. */
    cooklang_mark_t context_mark;

    /**
     * @}
     */

    /**
     * @name Reader stuff
     * @{
     */

    /** Read handler. */
    cooklang_read_handler_t *read_handler;

    /** A pointer for passing to the read handler. */
    void *read_handler_data;

    /** Standard (string or file) input data. */
    union {
        /** String input data. */
        struct {
            /** The string start pointer. */
            const unsigned char *start;
            /** The string end pointer. */
            const unsigned char *end;
            /** The string current position. */
            const unsigned char *current;
        } string;

        /** File input data. */
        FILE *file;
    } input;

    /** EOF flag */
    int eof;

    /** The working buffer. */
    struct {
        /** The beginning of the buffer. */
        cooklang_char_t *start;
        /** The end of the buffer. */
        cooklang_char_t *end;
        /** The current position of the buffer. */
        cooklang_char_t *pointer;
        /** The last filled position of the buffer. */
        cooklang_char_t *last;
    } buffer;

    /* The number of unread characters in the buffer. */
    size_t unread;

    /** The raw buffer. */
    struct {
        /** The beginning of the buffer. */
        unsigned char *start;
        /** The end of the buffer. */
        unsigned char *end;
        /** The current position of the buffer. */
        unsigned char *pointer;
        /** The last filled position of the buffer. */
        unsigned char *last;
    } raw_buffer;

    /** The input encoding. */
    cooklang_encoding_t encoding;

    /** The offset of the current position (in bytes). */
    size_t offset;

    /** The mark of the current position. */
    cooklang_mark_t mark;

    /**
     * @}
     */

    /**
     * @name Scanner stuff
     * @{
     */

    /** Have we started to scan the input stream? */
    int stream_start_produced;

    /** Have we reached the end of the input stream? */
    int stream_end_produced;

    /** The number of unclosed '[' and '{' indicators. */
    int flow_level;

    /** The tokens queue. */
    struct {
        /** The beginning of the tokens queue. */
        cooklang_token_t *start;
        /** The end of the tokens queue. */
        cooklang_token_t *end;
        /** The head of the tokens queue. */
        cooklang_token_t *head;
        /** The tail of the tokens queue. */
        cooklang_token_t *tail;
    } tokens;

    /** The number of tokens fetched from the queue. */
    size_t tokens_parsed;

    /** Does the tokens queue contain a token ready for dequeueing. */
    int token_available;

    /**
     * @}
     */

    /**
     * @name Parser stuff
     * @{
     */

    /** The parser states stack. */
    struct {
        /** The beginning of the stack. */
        cooklang_parser_state_t *start;
        /** The end of the stack. */
        cooklang_parser_state_t *end;
        /** The top of the stack. */
        cooklang_parser_state_t *top;
    } states;

    /** The current parser state. */
    cooklang_parser_state_t state;

    /** The stack of marks. */
    struct {
        /** The beginning of the stack. */
        cooklang_mark_t *start;
        /** The end of the stack. */
        cooklang_mark_t *end;
        /** The top of the stack. */
        cooklang_mark_t *top;
    } marks;

    /**
     * @}
     */

    /**
     * @name Dumper stuff
     * @{
     */

    /** The currently parsed recipe. */
    cooklang_recipe_t *recipe;

    /**
     * @}
     */

} cooklang_parser_t;

/**
 * Initialize a parser.
 *
 * This function creates a new parser object.  An application is responsible
 * for destroying the object using the cooklang_parser_delete() function.
 *
 * @param[out]      parser  An empty parser object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

COOKLANG_DECLARE(int)
cooklang_parser_initialize(cooklang_parser_t *parser);

/**
 * Destroy a parser.
 *
 * @param[in,out]   parser  A parser object.
 */

COOKLANG_DECLARE(void)
cooklang_parser_delete(cooklang_parser_t *parser);

/**
 * Set a string input.
 *
 * Note that the @a input pointer must be valid while the @a parser object
 * exists.  The application is responsible for destroing @a input after
 * destroying the @a parser.
 *
 * @param[in,out]   parser  A parser object.
 * @param[in]       input   A source data.
 * @param[in]       size    The length of the source data in bytes.
 */

COOKLANG_DECLARE(void)
cooklang_parser_set_input_string(cooklang_parser_t *parser,
        const unsigned char *input, size_t size);

/**
 * Set a file input.
 *
 * @a file should be a file object open for reading.  The application is
 * responsible for closing the @a file.
 *
 * @param[in,out]   parser  A parser object.
 * @param[in]       file    An open file.
 */

COOKLANG_DECLARE(void)
cooklang_parser_set_input_file(cooklang_parser_t *parser, FILE *file);

/**
 * Set a generic input handler.
 *
 * @param[in,out]   parser  A parser object.
 * @param[in]       handler A read handler.
 * @param[in]       data    Any application data for passing to the read
 *                          handler.
 */

COOKLANG_DECLARE(void)
cooklang_parser_set_input(cooklang_parser_t *parser,
        cooklang_read_handler_t *handler, void *data);

/**
 * Set the source encoding.
 *
 * @param[in,out]   parser      A parser object.
 * @param[in]       encoding    The source encoding.
 */

COOKLANG_DECLARE(void)
cooklang_parser_set_encoding(cooklang_parser_t *parser, cooklang_encoding_t encoding);

/**
 * Scan the input stream and produce the next token.
 *
 * Call the function subsequently to produce a sequence of tokens corresponding
 * to the input stream.  The initial token has the type
 * @c COOKLANG_STREAM_START_TOKEN while the ending token has the type
 * @c COOKLANG_STREAM_END_TOKEN.
 *
 * An application is responsible for freeing any buffers associated with the
 * produced token object using the @c cooklang_token_delete function.
 *
 * An application must not alternate the calls of cooklang_parser_scan() with the
 * calls of cooklang_parser_parse() or cooklang_parser_load(). Doing this will break
 * the parser.
 *
 * @param[in,out]   parser      A parser object.
 * @param[out]      token       An empty token object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

COOKLANG_DECLARE(int)
cooklang_parser_scan(cooklang_parser_t *parser, cooklang_token_t *token);

/**
 * Parse the input stream and produce the next parsing event.
 *
 * Call the function subsequently to produce a sequence of events corresponding
 * to the input stream.  The initial event has the type
 * @c COOKLANG_STREAM_START_EVENT while the ending event has the type
 * @c COOKLANG_STREAM_END_EVENT.
 *
 * An application is responsible for freeing any buffers associated with the
 * produced event object using the cooklang_event_delete() function.
 *
 * An application must not alternate the calls of cooklang_parser_parse() with the
 * calls of cooklang_parser_scan() or cooklang_parser_load(). Doing this will break the
 * parser.
 *
 * @param[in,out]   parser      A parser object.
 * @param[out]      event       An empty event object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

COOKLANG_DECLARE(int)
cooklang_parser_parse(cooklang_parser_t *parser, cooklang_event_t *event);

/**
 * Parse the input stream and produce the next COOKLANG recipe.
 *
 * Call this function subsequently to produce a sequence of recipes
 * constituting the input stream.
 *
 * If the produced recipe has no root node, it means that the recipe
 * end has been reached.
 *
 * An application is responsible for freeing any data associated with the
 * produced recipe object using the cooklang_recipe_delete() function.
 *
 * An application must not alternate the calls of cooklang_parser_load() with the
 * calls of cooklang_parser_scan() or cooklang_parser_parse(). Doing this will break
 * the parser.
 *
 * @param[in,out]   parser      A parser object.
 * @param[out]      recipe    An empty recipe object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

COOKLANG_DECLARE(int)
cooklang_parser_load(cooklang_parser_t *parser, cooklang_recipe_t *recipe);

/** @} */




#ifdef __cplusplus
}
#endif

#endif

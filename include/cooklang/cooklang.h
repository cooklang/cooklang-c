/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2022 Alexey Dubovskoy
 */

/**
 * \file
 * \brief COOKLANG library public header.
 *
 * COOKLANG is a C library for parsing and serialising recipes in Cooklang format.
 */

#ifndef COOKLANG_H
#define COOKLANG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>


/**
 * COOKLANG library version string.
 */
extern const char *cooklang_version_str;

/**
 * COOKLANG library version number suitable for comparisons.
 *
 * Version number binary composition is `0bRUUUUUUUJJJJJJJJNNNNNNNNPPPPPPPP`.
 *
 * | Character | Meaning                                |
 * | --------- | -------------------------------------- |
 * | `R`       | Release flag.  If set, it's a release. |
 * | `U`       | Unused / reserved.                     |
 * | `J`       | Major component of version.            |
 * | `N`       | Minor component of version.            |
 * | `P`       | Patch component of version.            |
 */
extern const uint32_t cooklang_version;

/**
 * CYAML value types.
 *
 * These are the fundamental data types that apply to "values" in CYAML.
 *
 * CYAML "values" are represented in by \ref cyaml_schema_value.
 */
typedef enum {
    TOKEN_WORD,
    TOKEN_INTEGER,
    TOKEN_DECIMAL,
    TOKEN_FRACTIONAL,
    TOKEN_WHOLE_FRACTIONAL,
    TOKEN_WHITESPACE,
    TOKEN_COLON,
    TOKEN_AT,
    TOKEN_PERCENT,
    TOKEN_BRACES_LEFT,
    TOKEN_BRACES_RIGHT,
    TOKEN_PIPE,
    TOKEN_CHEVRON,
    TOKEN_TILDE,
    TOKEN_HASH,
    TOKEN_EOL,
    TOKEN_EOF,
    TOKEN_COUNT
} token_type_t;

/**
 * CYAML value types.
 *
 * These are the fundamental data types that apply to "values" in CYAML.
 *
 * CYAML "values" are represented in by \ref cyaml_schema_value.
 */
typedef struct {
    const uint8_t *start, *end;
    token_type_t type;
} token_t;

/**
 * Convert a COOKLANG type into a human readable string.
 *
 * \param[in]  type  The state to convert.
 * \return String representing state.
 */
static inline const char * token_type_to_string(token_type_t type)
{
    static const char * const strings[TOKEN_COUNT] = {
        [TOKEN_WORD]             = "TOKEN_WORD",
        [TOKEN_INTEGER]          = "TOKEN_INTEGER",
        [TOKEN_DECIMAL]          = "TOKEN_DECIMAL",
        [TOKEN_FRACTIONAL]       = "TOKEN_FRACTIONAL",
        [TOKEN_WHOLE_FRACTIONAL] = "TOKEN_WHOLE_FRACTIONAL",
        [TOKEN_WHITESPACE]       = "TOKEN_WHITESPACE",
        [TOKEN_COLON]            = "TOKEN_COLON",
        [TOKEN_AT]               = "TOKEN_AT",
        [TOKEN_PERCENT]          = "TOKEN_PERCENT",
        [TOKEN_BRACES_LEFT]      = "TOKEN_BRACES_LEFT",
        [TOKEN_BRACES_RIGHT]     = "TOKEN_BRACES_RIGHT",
        [TOKEN_PIPE]             = "TOKEN_PIPE",
        [TOKEN_CHEVRON]          = "TOKEN_CHEVRON",
        [TOKEN_TILDE]            = "TOKEN_TILDE",
        [TOKEN_HASH]             = "TOKEN_HASH",
        [TOKEN_EOL]              = "TOKEN_EOL",
        [TOKEN_EOF]              = "TOKEN_EOF",
    };
    if ((int)type >= TOKEN_COUNT) {
        return "<invalid>";
    }
    return strings[type];
}



#ifdef __cplusplus
}
#endif

#endif

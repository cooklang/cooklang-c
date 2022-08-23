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



#ifdef __cplusplus
}
#endif

#endif

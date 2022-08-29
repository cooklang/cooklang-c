/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2022 Alexey Dubovskoy
 */

/**
 * \file
 * \brief COOKLANG lexer.
 */

#ifndef COOKLANG_LEXER_H
#define COOKLANG_LEXER_H

#include <cooklang/cooklang.h>


void init_lexer(const wchar_t *input);
token_t next_token(void);

#endif

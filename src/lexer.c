/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2022 Alexey Dubovskoy
 */

/**
 * \file
 * \brief COOKLANG lexer.
 */


#include "lexer.h"

typedef struct {
  const wchar_t* start;
  const wchar_t* current;
  int line;
} lexer_t;

lexer_t lexer;

void init_lexer(const wchar_t *input) {
    lexer.start = input;
    lexer.current = input;
    lexer.line = 1;
}

token_t next_token(){
    token_t token;
    token.type = TOKEN_EOF;
    return token;
}

/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2022 Alexey Dubovskoy
 */

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include <cooklang.h>

#include "ttest.h"
#include "test.h"

/**
 * Unit test context data.
 */
typedef struct test_data {
    const cooklang_char_t *input;
    cooklang_parser_t parser;
    cooklang_token_t token;
} test_data_t;


#define EXPECT_NEXT_TOKEN(tc, td, expected_token_type) do { \
    if (!cooklang_parser_scan(&td.parser, &td.token)) \
        return ttest_fail(&tc, "Unexpected exit from scan"); \
    if (td.token.type != expected_token_type) { \
        cleanup(&td); \
        return ttest_fail(&tc, "Line %d: expected %s, got %s", __LINE__, cooklang_token_type_to_string(expected_token_type), cooklang_token_type_to_string(td.token.type)); \
    } \
    } while(0)


static void init_test_data(test_data_t *data, const cooklang_char_t *input) {
    data->input = input;
    cooklang_parser_initialize(&data->parser);
    cooklang_parser_set_input_string(&data->parser, data->input, strlen((char *)data->input));
}

static void cleanup(test_data_t *data) {
    cooklang_token_delete(&data->token);
    cooklang_parser_delete(&data->parser);
    memset(data, 0, sizeof(test_data_t));
}

/**
 * Test hello.
 *
 * \param[in]  report  The test report context.
 */
static bool test_hello(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    init_test_data(&td, (cooklang_char_t *)"hello");
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}


/**
 * Test empty input.
 *
 * \param[in]  report  The test report context.
 */
static bool test_empty_input(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    init_test_data(&td, (cooklang_char_t *)"");
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test whitespace only input.
 *
 * \param[in]  report  The test report context.
 */
static bool test_whitespace_only_input(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    init_test_data(&td, (cooklang_char_t *)" ");
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test new line input.
 *
 * \param[in]  report  The test report context.
 */
static bool test_new_line_input(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"   \n    ";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_EOL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test windows new line input.
 *
 * \param[in]  report  The test report context.
 */
static bool test_windows_new_line_input(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc\r\ndef\r\nghi";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_EOL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_EOL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test multiple new lines input.
 *
 * \param[in]  report  The test report context.
 */
static bool test_multiple_new_lines_input(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"   \n\n    ";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_EOL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test basic string.
 *
 * \param[in]  report  The test report context.
 */
static bool test_basic_string(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test emoji.
 *
 * \param[in]  report  The test report context.
 */
static bool test_emoji(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"🍚";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test concessive strings.
 *
 * \param[in]  report  The test report context.
 */
static bool test_concessive_strings(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc xyz";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test string with numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_string_with_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc 70770 xyz";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test string with decimal numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_string_with_decimal_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc 0.70770 xyz";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_DECIMAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test string with decimal like numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_string_with_decimal_like_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc 01.70770 xyz";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test decimal numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_decimal_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"0.70770";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_DECIMAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test integer numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_integer_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"70770";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test integer like numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_integer_like_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"70770hehe";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}


/**
 * Test fractional numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_fractional_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"1/2";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_FRACTIONAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test fractional numbers with spaces1.
 *
 * \param[in]  report  The test report context.
 */
static bool test_fractional_numbers_with_spaces1(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"1 / 2";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_FRACTIONAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test fractional numbers with spaces2.
 *
 * \param[in]  report  The test report context.
 */
static bool test_fractional_numbers_with_spaces2(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)" 1 / 2";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_FRACTIONAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test fractional numbers with spaces after.
 *
 * \param[in]  report  The test report context.
 */
static bool test_fractional_numbers_with_spaces_after(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)" 1 / 2 ";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_FRACTIONAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test almost fractional numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_almost_fractional_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"01/2";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test string with leading zero numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_string_with_leading_zero_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc 0777 xyz";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test string with like numbers.
 *
 * \param[in]  report  The test report context.
 */
static bool test_string_with_like_numbers(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc 7peppers xyz";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test string with punctuation.
 *
 * \param[in]  report  The test report context.
 */
static bool test_string_with_punctuation(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc – xyz: lol,";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_COLON_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test string with punctuation repeated.
 *
 * \param[in]  report  The test report context.
 */
static bool test_string_with_punctuation_repeated(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"abc – ...,,xyz: lol,";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_COLON_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients one liner.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_one_liner(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    cooklang_char_t* input = (cooklang_char_t *)"Add @onions{3%medium} chopped finely";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_PERCENT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients decimal.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_decimal(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"@onions{3.5%medium}";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_DECIMAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_PERCENT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients fractions.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_fractions(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"@onions{1/4%medium}";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_FRACTIONAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_PERCENT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients fractions with spaces.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_fractions_with_spaces(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"@onions{1 / 4 %medium}";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_FRACTIONAL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_PERCENT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test comments.
 *
 * \param[in]  report  The test report context.
 */
static bool test_comments(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"-- testing comments";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test block comments.
 *
 * \param[in]  report  The test report context.
 */
static bool test_block_comments(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"visible [- hidden -] visible";
    init_test_data(&td, input);
    printf("\nhere %s\n", td.parser.raw_buffer.start);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    printf("\nhere %s\n", td.parser.buffer.pointer);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    printf("\nhere %s\n", td.parser.buffer.pointer);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    printf("\nhere %s\n", td.parser.buffer.pointer);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test block comments multiline.
 *
 * \param[in]  report  The test report context.
 */
static bool test_block_comments_multiline(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"visible [- hidden"
                            "hidden"
                            "hidden -] visible";

    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test block comments multiline unfinished.
 *
 * \param[in]  report  The test report context.
 */
static bool test_block_comments_multiline_unfinished(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"visible [- hidden"
                            "hidden"
                            "hidden";

    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test block comments unfinished.
 *
 * \param[in]  report  The test report context.
 */
static bool test_block_comments_unfinished(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"visible [- hidden";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test dash last.
 *
 * \param[in]  report  The test report context.
 */
static bool test_dash_last(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"onions -";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test dash in text.
 *
 * \param[in]  report  The test report context.
 */
static bool test_dash_in_text(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"Preheat the oven to 200℃-Fan 180°C.";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test square in text.
 *
 * \param[in]  report  The test report context.
 */
static bool test_square_in_text(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"Preheat the oven to 200℃[Fan] 180°C.";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients no units.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_no_units(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"@onions{3}";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients no amount.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_no_amount(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"@onions";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients multi word no amount.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_multi_word_no_amount(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"@red onions{}";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients multi word with punctuation.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_multi_word_with_punctuation(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"@onions, chopped";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients word no amount.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_word_no_amount(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"an @onion finely chopped";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test equipment.
 *
 * \param[in]  report  The test report context.
 */
static bool test_equipment(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"put into #oven";

    init_test_data(&td, input);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_HASH_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test equipment multi word.
 *
 * \param[in]  report  The test report context.
 */
static bool test_equipment_multi_word(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"fry on #frying pan{}";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_HASH_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test timer.
 *
 * \param[in]  report  The test report context.
 */
static bool test_timer(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"cook for ~{10%minutes}";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_TILDE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_PERCENT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test broken timer.
 *
 * \param[in]  report  The test report context.
 */
static bool test_broken_timer(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)"cook for ~{10 minutes}";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_TILDE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test ingridients multi liner.
 *
 * \param[in]  report  The test report context.
 */
static bool test_ingridients_multi_liner(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)
                            "Add @onions{3%medium} chopped finely\n"
                            "Bonne appetite!";

    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_AT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_LEFT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_PERCENT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_BRACES_RIGHT_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_EOL_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}

/**
 * Test metadata.
 *
 * \param[in]  report  The test report context.
 */
static bool test_metadata(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;
    test_data_t td;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    const cooklang_char_t* input = (cooklang_char_t *)">> servings: 4|5|6";
    init_test_data(&td, input);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_START_TOKEN);

    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_CHEVRON_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_CHEVRON_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WORD_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_COLON_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_WHITESPACE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_PIPE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_PIPE_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_INTEGER_TOKEN);
    EXPECT_NEXT_TOKEN(tc, td, COOKLANG_STREAM_END_TOKEN);

    cleanup(&td);

    return ttest_pass(&tc);
}


/**
 * Run the COOKLANG scanner tests.
 *
 * \param[in]  rc         The ttest report context.
 * \return true iff all unit tests pass, otherwise false.
 */
bool scanner_tests(ttest_report_ctx_t *rc)
{
    bool pass = true;
    ttest_heading(rc, "Lexer tests");

    pass &= test_hello(rc);
    pass &= test_empty_input(rc);
    pass &= test_whitespace_only_input(rc);
    pass &= test_new_line_input(rc);
    pass &= test_windows_new_line_input(rc);
    pass &= test_multiple_new_lines_input(rc);
    pass &= test_basic_string(rc);
    pass &= test_emoji(rc);
    pass &= test_concessive_strings(rc);
    pass &= test_string_with_numbers(rc);
    pass &= test_string_with_decimal_numbers(rc);
    pass &= test_string_with_decimal_like_numbers(rc);
    pass &= test_decimal_numbers(rc);
    pass &= test_integer_numbers(rc);
    pass &= test_integer_like_numbers(rc);
    pass &= test_fractional_numbers(rc);
    pass &= test_fractional_numbers_with_spaces1(rc);
    pass &= test_fractional_numbers_with_spaces2(rc);
    pass &= test_fractional_numbers_with_spaces_after(rc);
    pass &= test_almost_fractional_numbers(rc);
    pass &= test_string_with_leading_zero_numbers(rc);
    pass &= test_string_with_like_numbers(rc);
    pass &= test_string_with_punctuation(rc);
    pass &= test_string_with_punctuation_repeated(rc);
    pass &= test_ingridients_one_liner(rc);
    pass &= test_ingridients_decimal(rc);
    pass &= test_ingridients_fractions(rc);
    pass &= test_ingridients_fractions_with_spaces(rc);
    pass &= test_comments(rc);
    pass &= test_block_comments(rc);
    pass &= test_block_comments_multiline(rc);
    pass &= test_block_comments_multiline_unfinished(rc);
    pass &= test_block_comments_unfinished(rc);
    pass &= test_dash_last(rc);
    pass &= test_dash_in_text(rc);
    pass &= test_square_in_text(rc);
    pass &= test_ingridients_no_units(rc);
    pass &= test_ingridients_no_amount(rc);
    pass &= test_ingridients_multi_word_no_amount(rc);
    pass &= test_ingridients_multi_word_with_punctuation(rc);
    pass &= test_ingridients_word_no_amount(rc);
    pass &= test_equipment(rc);
    pass &= test_equipment_multi_word(rc);
    pass &= test_timer(rc);
    pass &= test_broken_timer(rc);
    pass &= test_ingridients_multi_liner(rc);
    pass &= test_metadata(rc);

    return pass;
}

#undef IF_MATCHED_WITH_EXPECTED_TOKENS

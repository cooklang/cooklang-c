/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2022 Alexey Dubovskoy
 */

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>


#include "ttest.h"
#include "test.h"


/**
 * Test hello.
 *
 * \param[in]  report  The test report context.
 */
static bool test_hello(ttest_report_ctx_t *report)
{
    ttest_ctx_t tc;

    if (!ttest_start(report, __func__, NULL, NULL, &tc)) {
        return true;
    }

    return ttest_pass(&tc);
}

/**
 * Run the COOKLANG lexer tests.
 *
 * \param[in]  rc         The ttest report context.
 * \return true iff all unit tests pass, otherwise false.
 */
bool lexer_tests(ttest_report_ctx_t *rc)
{
    bool pass = true;
    ttest_heading(rc, "File loading tests");

    pass &= test_hello(rc);

    return pass;
}

/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2022 Alexey Dubovskoy
 */

#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "ttest.h"
#include "test.h"

/**
 * Print program usage
 *
 * \param[in]  prog_name  Name of program.
 */
static void usage(const char *prog_name)
{
	fprintf(stderr, "Usage: %s [-q|-v|-d]\n", prog_name);
}

/**
 * Main entry point from OS.
 *
 * \param[in]  argc  Argument count.
 * \param[in]  argv  Vector of string arguments.
 * \return Program return code.
 */
int main(int argc, char *argv[])
{
	bool pass = true;
	bool quiet = false;
	ttest_report_ctx_t rc;
	const char *test_list = NULL;

	enum {
		ARG_PROG_NAME,
		ARG_VERBOSE,
		ARG_TEST_LIST,
		ARG__COUNT,
	};

	if (argc > ARG__COUNT) {
		usage(argv[ARG_PROG_NAME]);
		return EXIT_FAILURE;

	}

	for (int i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "-q") == 0) ||
		    (strcmp(argv[i], "--quiet") == 0)) {
			quiet = true;
			// log_fn = NULL;
		} else if ((strcmp(argv[i], "-v") == 0) ||
		           (strcmp(argv[i], "--verbose") == 0)) {
			// log_level = COOKLANG_LOG_INFO;
		} else if ((strcmp(argv[i], "-d") == 0) ||
		           (strcmp(argv[i], "--debug") == 0)) {
			// log_level = COOKLANG_LOG_DEBUG;
		} else {
			test_list = argv[i];
		}
	}

	rc = ttest_init(test_list, quiet);

	pass &= scanner_tests(&rc);

	ttest_report(&rc);

	return (pass) ? EXIT_SUCCESS : EXIT_FAILURE;
}

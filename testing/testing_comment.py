import unittest

import cooklang


class TestComment(unittest.TestCase):
    def validate(self, test_cases: list) -> None:
        for test_case in test_cases:
            output = cooklang.parseRecipe(test_case["input"])
            print(output)
            self.assertEqual(output["steps"][0][0]["value"], test_case["output_steps"])

    def test_basic(self) -> None:
        test_cases = [
            {
                "input": "meal for 4 people [- scale linearly-]",
                "output_steps": "meal for 4 people ",
            },
        ]

        self.validate(test_cases)

    def test_close_twice(self) -> None:
        test_cases = [
            {
                "input": "meal for 4 people [- scale linearly -] and take 20 minutes to prepare [- does not scale-]",
                "output_steps": "meal for 4 people  and take 20 minutes to prepare ",
            },
        ]

        self.validate(test_cases)

    def test_multiline(self) -> None:
        test_cases = [
            {
                "input": "meal for 4 people [- scale linearly  \n for people \n but time does not scale-]",
                "output_steps": "meal for 4 people ",
            },
        ]
        for test_case in test_cases:
            output = cooklang.parseRecipe(test_case["input"])
            print(output)
            self.assertEqual(len(output["steps"]), 1)

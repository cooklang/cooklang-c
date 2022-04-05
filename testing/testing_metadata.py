import unittest

import cooklang


class TestMetadata(unittest.TestCase):
    def test_basic(self) -> None:
        test_cases = [
            {
                "input": ">> sourced: babooshka\n",
                "output_metadata": {"sourced": "babooshka"},
            },
            {
                "input": ">> source: https://www.gimmesomeoven.com/baked-potato/\n",
                "output_metadata": {"source": "https://www.gimmesomeoven.com/baked-potato/"},
            },
        ]

        for test_case in test_cases:
            output = cooklang.parseRecipe(test_case["input"])
            self.assertEqual(output["metadata"], test_case["output_metadata"])

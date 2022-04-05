import unittest

import cooklang


class TestWords(unittest.TestCase):
    def test_basic(self) -> None:
        test_cases = [
            {
                "input": "recette de croque-monsieur",
                "output_steps": "recette de croque-monsieur",
            },
            {
                "input": "recette de cm -- croque-monsieur\n",
                "output_steps": "recette de cm ",
            },
            {
                "input": "recette de croque->monsieur\n",
                "output_steps": "recette de croque->monsieur",
            },
            {
                "input": "recette de [croque]-monsieur\n",
                "output_steps": "recette de [croque]-monsieur",
            },
        ]

        for test_case in test_cases:
            output = cooklang.parseRecipe(test_case["input"])
            self.assertEqual(output["steps"][0][0]["value"], test_case["output_steps"])

    def test_unicode(self) -> None:
        test_cases = [
            {
                "input": "recette française 😃",
                "output_steps": "recette française 😃",
            },
        ]
        for test_case in test_cases:
            output = cooklang.parseRecipe(test_case["input"])
            print(output)
            self.assertEqual(output["steps"][0][0]["value"], test_case["output_steps"])

import ctypes
import json
from ctypes import *
from typing import Dict

import cooklang
import yaml

POSSIBLE_FIELDS = [
    "type",
    "name",
    "quantity",
    "units",
]


def prettyPrintResult(result: Dict) -> None:
    j = 0
    for i, step in enumerate(result["steps"]):
        print("\n  Step " + str(i) + ":")

        for thing in step:
            j += 1
            print("   Item " + str(j) + ":")
            print("    Type: " + thing["type"])

            if "value" in thing:
                print("    Value: " + thing["value"])
            elif "name" in thing:
                print("    Name: " + thing["name"])
            elif "quantity" in thing:
                print("    Quantity: " + str(thing["quantity"]))
            elif "units" in thing:
                print("    Units: " + thing["units"])

    print("\n  Metadata:")
    print("\n".join(result["metadata"]))


def jsonPrintResult(result: Dict) -> str:
    stepString = ""
    for step in result["steps"]:
        for thing in step:
            stepString += '{ "type": "' + thing["type"] + '"'

            if "value" in thing:
                stepString += ', "value": "' + thing["value"] + '"'

            if "name" in thing:
                stepString += ', "name": "' + thing["name"] + '"'

            if "quantity" in thing:
                stepString += ', "quantity": "' + str(thing["quantity"]) + '"'

            if "units" in thing:
                stepString += ', "units": "' + thing["units"] + '"'

            stepString += "}"

    for meta in result["metadata"]:
        stepString += '{ "Identifier": "' + meta + '", "Content": "' + result["metadata"][meta] + '"}'

    return stepString


def compareQuantities(inputExpectedQuantity: any, inputActualQuantity: any) -> int:
    # convert both to float
    try:
        expectedQuantity = float(inputExpectedQuantity)
        actualQuantity = float(inputActualQuantity)
        # compare the floats
        if expectedQuantity == actualQuantity:
            return 1
        return 0

    except:
        # if fails, it is a string, compare them
        if inputExpectedQuantity == inputActualQuantity:
            return 1
        return 0


def compareTest(expectedInput, actualInput) -> int:
    """
    Returns:
        0 if the test pass, else 1
    """
    print()

    # test steps
    if expectedInput["steps"] != actualInput["steps"]:

        # check length of the list, then iterate on both of them
        if len(expectedInput["steps"]) != len(actualInput["steps"]):
            return 1
        for expected_step, actual_step in zip(expectedInput["steps"], actualInput["steps"]):

            # check again length of the step lists and iterate on direc
            if len(expected_step) != len(actual_step):
                return 1
            for expected_direc, actual_direc in zip(expected_step, actual_step):
                # if they're not identical check each item
                # dev note: in python, dict are ordered
                if expected_direc != actual_direc:
                    if len(expected_direc) != len(actual_direc):
                        return 1
                    for item in expected_direc.keys():
                        if item not in POSSIBLE_FIELDS:
                            return 1
                        if item == "quantity":
                            if not compareQuantities(expected_direc[item], actual_direc[item]):
                                print("  Test failed. Quantities did not match")
                                return 1

                        # otherwise check which item is different
                        elif expected_direc[item] != actual_direc[item]:
                            print("Item: " + item + " did not match in actual output:")
                            print("  Expected: " + str(expected_direc[item]) + "   Actual: " + (actual_direc[item]))
                            return 1

    # test meta data
    if expectedInput["metadata"] != actualInput["metadata"]:
        print("  Test failed. Metadata did not match")
        return 1

    print("  Test Passed.")
    return 0


def main():
    # parse input test file and get each test
    with open("testing/tests.yaml") as tests_input_file:
        tests_input = yaml.safe_load(tests_input_file)

    passed = 0
    total = 0
    unpassed = []

    # for each test found, put the test in the file
    for test in tests_input["tests"]:
        # write test to file
        with open("test_file.cook", "w") as test_file:
            test_file.write(str(tests_input["tests"][test]["source"]))

        print("\n\n____  Test: " + test)
        print("____           Expected Output           ____")
        expectedResult = tests_input["tests"][test]["result"]
        prettyPrintResult(expectedResult)

        print("____            Actual Output            ____")
        # parse the file
        actualResult = cooklang.parseRecipe("test_file.cook")

        prettyPrintResult(actualResult)
        print("____           Compare Results           ____")

        final = compareTest(expectedResult, actualResult)
        total += 1
        if final == 0:
            passed += 1
        else:
            unpassed.append(test)

    print("\n\nTests passed: " + str(passed) + "/" + str(total))
    print("Unpassed tests: ")
    print(unpassed)


if __name__ == "__main__":
    main()

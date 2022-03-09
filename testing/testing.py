#!python3

import ctypes
from ctypes import *

import json
import yaml
import cooklang


def prettyPrintResult(result):
  i = 0
  j = 0
  for step in result['steps']:
    i += 1
    print('\n  Step ' + str(i) + ':')

    for thing in step:
      j += 1
      print('   Item ' + str(j) + ':')
      print('    Type: ' + thing['type'])

      if( 'value' in thing ):
        print('    Value: ' + thing['value'])

      if( 'name' in thing ):
        print('    Name: ' + thing['name'])

      if( 'quantity' in thing ):
        print('    Quantity: ' + str(thing['quantity']))

      if( 'units' in thing ):
        print('    Units: ' + thing['units'])

  print('\n  Metadata:')
  for meta in result['metadata']:
    print(meta)  


def jsonPrintResult(result):
  stepString = ''
  for step in result['steps']:
    for thing in step:
      stepString += '{ \"type\": \"' + thing['type'] + '\"'

      if( 'value' in thing ):
        stepString += ', \"value\": \"' + thing['value'] + '\"'

      if( 'name' in thing ):
        stepString += ', \"name\": \"' + thing['name'] + '\"'

      if( 'quantity' in thing ):
        stepString += ', \"quantity\": \"' + str(thing['quantity']) + '\"'

      if( 'units' in thing ):
        stepString += ', \"units\": \"' + thing['units'] + '\"'

      stepString += '}'

  for meta in result['metadata']:
    stepString += '{ \"Identifier\": \"' + meta + '\", \"Content\": \"' + result['metadata'][meta] + '\"}'

  return stepString



def compareQuantities(inputExpectedQuantity, inputActualQuantity):
  # convert both to float
  try:
    expectedQuantity = float(inputExpectedQuantity);
    actualQuantity = float(inputActualQuantity);
    #compare the floats
    if( expectedQuantity == actualQuantity):
      return 1;
    return 0;

  except:
    # if fails, it is a string, compare them
    if( inputExpectedQuantity == inputActualQuantity ):
      return 1;
    return 0;




def compareTest(expectedInput, actualInput):
  print('')

  # test steps
  if expectedInput['steps'] and actualInput['steps']:
    if expectedInput['steps'] != actualInput['steps']:
      
      # check each step
      for step in range(0, len(expectedInput['steps'])):
        for direc in range(0, len(expectedInput['steps'][step])):
          
          # if they're not identical check each item
          if(expectedInput['steps'][step][direc] != actualInput['steps'][step][direc]):

            for item in expectedInput['steps'][step][direc]:
              # check if each item is found
              if item not in actualInput['steps'][step][direc]:
                print('Item: ' + item + ' could not be found in actual output.')
                return 1
              
              # special compare function for quantities
              elif item == 'quantity':
                if not compareQuantities( expectedInput['steps'][step][direc][item], actualInput['steps'][step][direc][item]):
                  print('  Test failed. Quantities did not match')
                  return 1

              # otherwise check which item is different
              elif expectedInput['steps'][step][direc][item] != actualInput['steps'][step][direc][item]:
                print('Item: ' + item + ' did not match in actual output:')
                print('  Expected: ' + str(expectedInput['steps'][step][direc][item]) + '   Actual: ' + (actualInput['steps'][step][direc][item]))
                return 1

  # test meta data
  if expectedInput['metadata'] and actualInput['metadata']:

    if expectedInput['metadata'] != actualInput['metadata']:
      print('  Test failed. Metadata did not match')
      return 1

  print('  Test Passed.')
  return 0




# main

#parse input test file and get each test
tests_input_file = open('testing/tests.yaml')
tests_input = yaml.safe_load(tests_input_file)


passed = 0
total = 0
unpassed = []

# for each test found, put the test in the file
for test in tests_input['tests']:
  # write test to file
  test_file = open('test_file.cook', 'w')
  test_file.write(str(tests_input['tests'][test]['source']))
  test_file.close()

  print("\n\n____  Test: " + test)
  print("____           Expected Output           ____")
  expectedResult = tests_input['tests'][test]['result']
  prettyPrintResult(expectedResult)

  print("____            Actual Output            ____")
  # parse the file
  actualResult = cooklangC.parseRecipe('test_file.cook')

  prettyPrintResult(actualResult)
  print("____           Compare Results           ____")
  
  final = compareTest(expectedResult, actualResult)
  total += 1
  if final == 0:
    passed += 1
  else:
    unpassed.append(test)


print( '\n\nTests passed: ' + str(passed) + '/' + str(total))
print( 'Unpassed tests: ')
print(unpassed)

import ctypes
from ctypes import *

import json


def format_number(num):
    try:
        dec = decimal.Decimal(num)
    except:
        return 'bad'
    tup = dec.as_tuple()
    delta = len(tup.digits) + tup.exponent
    digits = ''.join(str(d) for d in tup.digits)
    if delta <= 0:
        zeros = abs(tup.exponent) - len(tup.digits)
        val = '0.' + ('0'*zeros) + digits
    else:
        val = digits[:delta] + ('0'*tup.exponent) + '.' + digits[delta:]
    val = val.rstrip('0')
    if val[-1] == '.':
        val = val[:-1]
    if tup.sign:
        return '-' + val
    return val

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


def secondPrettyPrint(result):
  for line in result.split('\n'):
    if line.strip():
      if line.strip() != "Empty step}":
        print(line)



def noneEmptyString(string):
  if string.strip():
    if string.strip() == "Empty step":
      return False;
    else: 
      return True;
  return False;


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
  try:
    expectedLines = expectedInput.split('}')
    actualLines = actualInput.split('}')

  except:
    print('  Test failed. Input were not string type')
    return 1;

  expectedLines = filter(noneEmptyString, expectedLines)
  actualLines = filter(noneEmptyString, actualLines)

  for i in range(0, len(expectedLines)):
    expectedLines[i] += '}'
    expectedLines[i] = expectedLines[i].strip()
  for i in range(0, len(actualLines)):
    actualLines[i] += '}'
    actualLines[i] = actualLines[i].strip()
    
  if(len(expectedLines) != len(actualLines)):
    print('  Test failed. Inequal line count')
    return

  for i in range(0, len(expectedLines)):
    try:
      expected = json.loads(expectedLines[i])
      actual = json.loads(actualLines[i])
      for item in expected:
        if( item not in actual ):
          print('  Test failed. Could not find \"' + item + '\" in Actual Result' )
          return 1

        if( item == "quantity"):
          if not compareQuantities( expected[item], actual[item]):
            print('  Test failed. Quantities did not match')
            return 1
          
        else:
          if( expected[item] != actual[item]):
            print('  Test failed. \"' + item + '\" did not match' )
            return 1

    except:
      print('  Test failed. Input could not be converted to json')
      return 1

  print('  Test Passed.')
  return 0


# main

so = '../Cooklang.so'
cooklang = CDLL(so)

#parse input test file and get each test
tests_input_file = open('tests.json')
tests_input = json.load(tests_input_file)

testFunc = cooklang.testFile
testFunc.restype = c_char_p


passed = 0
total = 0
unpassed = []

# for each test found, put the test in the file
for test in tests_input['tests']:
  # write test to file
  test_file = open('test_file.cook', 'w')
  test_file.write(tests_input['tests'][test]['source'].encode('utf-8'))
  test_file.close()

  print("\n\n____  Test: " + test)
  print("____           Expected Output           ____")
  expectedResult = jsonPrintResult( tests_input['tests'][test]['result'])
  print(expectedResult)

  print("____            Actual Output            ____")
  # parse the file
  actualResult = testFunc('test_file.cook')

  secondPrettyPrint(actualResult)
  print("____           Compare Results           ____")
  
  final = compareTest(expectedResult, actualResult)
  total += 1
  if final == 0:
    passed += 1
  else:
    unpassed.append(test)


print( 'Tests passed: ' + str(passed) + '/' + str(total))
print( 'Unpassed tests: ')
print(unpassed)

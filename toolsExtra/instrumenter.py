
# FLOW
# 1 Open file
# 2 Scan line by line
# 3 check for match
# 4 if there is a match insert instructions

inputName = "user_test.S"

input = open(inputName, 'r')

cnt = 1
line = input.readline()

while line:    
    newEntry = line
    print(newEntry)
    cnt += 1
    line = input.readline()

input.close()
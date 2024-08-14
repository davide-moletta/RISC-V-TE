import sys, re, os

DIR_JUMP_PATTERN = r'\b(call)\b\s+(\w+)'                                                          # Regex to find direct jump instructions
UNDIR_JUMP_PATTERN = r'\b(jalr)\b\s+(\w+)'                                                        # Regex to find indirect jump instructions
RET_PATTERN = r'\b(jr)\b\s+(\w+)'                                                                 # Regex to find return instructions
FUNC_START_PATTERN = r'^[ \t]*([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*$'                                   # Regex to find the start of a function
REGISTER_USE_PATTERN = r'\b(?:lw|la|li|addi|add|sub|mul|div|and|or|xor|sll|srl|sra)\b\s+(a[0-6])' # Regex to find the use of a registers before call

JUMP_TEMPLATE = "\tla  a7,{}\n\tecall\n"                                           # Template to substitute jump code
RET_TEMPLATE = "\tmv  s0,{}\n\tmv  a7,{}\n\taddi\ta7,a7,1\n\tecall\n\tmv  {},s0\n" # Template to substitute return code
MV_TEMPLATE = "\tmv  {},{}\n"                                                      # Template to store function parameters

# List of standard C functions
STD_C_FUNCS = set(["memset", "memcpy", "memmove", "scanf", "memcmp", "strcpy", "strncpy", "strcmp", "strncmp", 
                "strlen", "strstr", "bzero", "isalnum", "isalpha", "isascii", "isblank", "iscntrl", 
                "isdigit", "islower", "isgraph", "isprint", "ispunct", "isspace", "isupper", "toupper", 
                "tolower", "toascii", "memccpy", "memchr", "memrchr", "strcasecmp", "strcasestr", "strcat",
                "strdup", "strchr", "strcspn", "strcoll", "strlcat", "strlcpy", "strlwr", "strncasecmp", 
                "strncat", "strndup", "strnlen", "strrchr", "strsep", "strspn", "strtok_r", "strupr", "longjmp", 
                "setjmp", "abs", "div", "labs", "ldiv", "qsort", "rand_r", "rand", "srand", "utoa", "itoa", 
                "atoi", "atol", "strtol", "strtoul", "printf", "__udivdi3", "__umoddi3", "uart_tx_one_char2",
                "uart_rx_one_char", "uart_rx_one_char_block", "uart_rx_readbuff", "__divdi3"])
leaf_functions = set()

# Check if target function is a leaf
def is_leaf(function_name, files):
    print(f"Checking function {function_name} ...")
    target_pattern = rf'^[ \t]*{function_name}\s*:\s*$'  # Regex to find the start of the desired function

    for file in files:  # For each file, read its content and check
        with open(file, 'r') as f:
            lines = f.readlines()

        function_found = False
        for line in lines:
            if re.search(target_pattern, line):  # If there is a match for the target, set found to true
                function_found = True
                continue  # Continue checking next lines

            if function_found:
                if re.search(FUNC_START_PATTERN, line): # If we match the start of a new function the target function is a leaf
                    leaf_functions.add(function_name)   # Add the function name to the list of leaf functions
                    return True                          
                if re.search(DIR_JUMP_PATTERN, line):   # If we match a jump instruction the target function is not a leaf
                    return False                        ### maybe fot std function this is still considered a leaf ###
    leaf_functions.add(function_name) 
    return True                                         # If no calls found, assume the function is a leaf

def instrument(assembly_files):
    print("Instrumenting files...\n")
    for assembly_file in assembly_files: # For each file read its content and check
        print(f"Instrumenting file {os.path.basename(assembly_file)}...")
        replaced_jump = 0
        replaced_return = 0
        with open(assembly_file, 'r') as f:
            lines = f.readlines()

        new_lines = []
        curr_function = ""
        registers_used = {f'a{i}': False for i in range(7)} # Register tracking
        for line in lines:
            ################################
            # CHECK FOR NEW FUNCTION START #
            ################################
            function_match = re.search(FUNC_START_PATTERN, line)
            if function_match:                          # If we match the start of a new function store its name
                curr_function = function_match.group(1) # Store the current function name

            ##########################
            # CHECK FOR REGISTER USE #
            ##########################
            register_match = re.search(REGISTER_USE_PATTERN, line)
            if register_match:                # If we match an a register use
                reg = register_match.group(1) # Get the used register
                registers_used[reg] = True    # Set it as true in the dictionary
            else:
                ##############################
                # CHECK FOR JUMP INSTRUCTION #
                ##############################
                jump_match = re.search(DIR_JUMP_PATTERN, line) 
                if jump_match:                                                          # If we match a jump instruction
                    instr, label = jump_match.groups()                                  # Get the instruction and the label
                    if label not in STD_C_FUNCS and not is_leaf(label, assembly_files): # Check if target function is a leaf or a std C function. If it is, skip
                        for reg, used in registers_used.items():                        # Add mv instructions for used registers before ecall
                            if used:
                                sx = reg.replace('a', 's')                              # Replace 'a' with 's' to create sx (e.g. if a0 is used, s0 will be used)
                                new_lines.append(MV_TEMPLATE.format(sx, reg))           # Append mv sx,ax

                        new_lines.append(JUMP_TEMPLATE.format(label))                   # If the function is neither a leaf nor a std func replace the instruction
                        
                        for reg, used in registers_used.items():                        # Add mv instructions for used registers after ecall
                            if used:
                                sx = reg.replace('a', 's')                              # Replace 'a' with 's' to create sx (e.g. if a0 is used, s0 will be used)
                                new_lines.append(MV_TEMPLATE.format(reg, sx))           # Append mv ax,sx

                        replaced_jump += 1

                ################################
                # CHECK FOR RETURN INSTRUCTION #
                ################################
                ret_match = re.search(RET_PATTERN, line)                       # If we match a return instruction
                if ret_match and curr_function not in leaf_functions:          # Check if the current function is a leaf. If it is, skip
                    instr, label = ret_match.groups()                          # Get the instruction and the label
                    new_lines.append(RET_TEMPLATE.format(label, label, label)) # Append the label that contains the return address
                    replaced_return += 1
                
                registers_used.update({f'a{i}': False for i in range(7)}) # Reset register tracking

            new_lines.append(line) 
    
        # Write new lines
        with open(assembly_file, 'w') as f:
            f.writelines(new_lines)
        print(f"File {os.path.basename(assembly_file)} had {replaced_jump} jump instruction(s) and {replaced_return} return instruction(s)\n")

def main():
    if len(sys.argv) < 2:
        print("This file is used to instrument the code.\n"
                "Input files should be provided in the following order.\n"
                "python3 instrumenter.py [file_1.s] [file_2.s] [...] [file_N.s]\n"
                "Otherwise, use flasher.py to perform the complete build automatically")
        sys.exit(1)

    instrument(sys.argv[1:])

if __name__ == "__main__":
    main()

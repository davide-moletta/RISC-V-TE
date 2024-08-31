import sys
import re
import os
from pathlib import Path

PATTERNS = {
    "DIR_JUMP": re.compile(r'\b(call)\b\s+(\w+)'),                                                          # Regex to find direct jump instructions
    "UNDIR_JUMP": re.compile(r'\b(jalr)\b\s+(\w+)'),                                                        # Regex to find indirect jump instructions
    "RET": re.compile(r'\b(jr)\b\s+(\w+)'),                                                                 # Regex to find return instructions
    "FUNC_START": re.compile(r'^[ \t]*([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*$'),                                   # Regex to find the start of a function
    "REGISTER_USE": re.compile(r'\b(?:lw|la|li|addi|add|sub|mul|div|and|or|xor|sll|srl|sra)\b\s+(a[0-6])'), # Regex to find the use of a registers before call
    "STACK_OPENING": re.compile(r'addi\s+sp,sp,(-\d+)'),                                                    # Regex to find the opening of the stack
    "SW": re.compile(r'sw\s+\w+,\s*(\d+)\(sp\)'),                                                           # Regex to find the store word operations
    "CSRS": re.compile(r'^\s*csrs\s+mstatus\s*,\s*\w+\s*$')                                                 # Regex to find the end of intr_vector_table function
}

TEMPLATES = {
    "JUMP": "\tla  a7,{}\n\tecall\n",                                           # Template to substitute jump code
    "RET": "\tmv  s0,{}\n\tmv  a7,{}\n\taddi\ta7,a7,1\n\tecall\n\tmv  {},s0\n", # Template to substitute return code
    "MV": "\tmv  {},{}\n",                                                      # Template to store function parameters
    "OPEN_STACK": "\taddi\tsp,sp,{}\n",                                         # Template to open the stack
    "ADDITIONAL_SW": "\tsw  s{},{}(sp)\n"                                       # Template to store a word
}

# List of standard C functions
STD_C_FUNCS = {"memset", "memcpy", "memmove", "scanf", "memcmp", "strcpy", "strncpy", "strcmp", "strncmp",
               "strlen", "strstr", "bzero", "isalnum", "isalpha", "isascii", "isblank", "iscntrl",
               "isdigit", "islower", "isgraph", "isprint", "ispunct", "isspace", "isupper", "toupper",
               "tolower", "toascii", "memccpy", "memchr", "memrchr", "strcasecmp", "strcasestr", "strcat",
               "strdup", "strchr", "strcspn", "strcoll", "strlcat", "strlcpy", "strlwr", "strncasecmp",
               "strncat", "strndup", "strnlen", "strrchr", "strsep", "strspn", "strtok_r", "strupr", "longjmp",
               "setjmp", "abs", "div", "labs", "ldiv", "qsort", "rand_r", "rand", "srand", "utoa", "itoa",
               "atoi", "atol", "strtol", "strtoul", "printf", "__udivdi3", "__umoddi3", "uart_tx_one_char2",
               "uart_rx_one_char", "uart_rx_one_char_block", "uart_rx_readbuff", "__divdi3"}

leaf_functions = set()

# Check if target function is a leaf
def is_leaf(function_name: str) -> bool:
    print(f"Checking function {function_name}...")
    target_pattern = re.compile(rf'^[ \t]*{function_name}\s*:\s*$') # Regex to find the start of the desired function
    
    for filename in Path(".").glob("*.s"):                          # For each file except peculiar ones, read its content and check
        if filename.name in {"boot.s", "main.s", "intr_vector_table.s", "shadow_stack.s"}:
            continue
        
        with filename.open('r') as f:
            lines = f.readlines()

        function_found = False
        for line in lines:                                          # If there is a match for the target, set found to true
            if target_pattern.search(line):
                function_found = True
                continue                                            # Continue checking next lines

            if function_found:
                if PATTERNS["FUNC_START"].search(line):             # If we match the start of a new function the target function is a leaf
                    leaf_functions.add(function_name)               # Add the function name to the list of leaf functions
                    return True
                if PATTERNS["DIR_JUMP"].search(line):               # If we match a jump instruction the target function is not a leaf
                    return False                                    ### maybe fot std function this is still considered a leaf ###

    leaf_functions.add(function_name)
    return True                                                     # If no calls found, assume the function is a leaf                                      


def instrument_vector_table():
    print("Instrumenting interrupt vector table file...\n")
    with Path("intr_vector_table.s").open('r') as f:
        lines = f.readlines()

    new_lines = []
    sw_instructions = []
    function_found = False
    sw_found = True
    sw_added = False
    value = 0
    
    for line in lines:
        if re.search(r'^[ \t]*synchronous_exception_handler\s*:\s*$', line):       # Regex to find the start of the desired function
            function_found = True                                                  # If there is a match for the target, set found to true
            new_lines.append(line)
            continue

        if function_found:
            ####################################
            # CHECK FOR AMOUNT OF STACK OPENED #
            ####################################
            stack_match = PATTERNS["STACK_OPENING"].search(line)
            if stack_match:                                                        # If we match the opening of the stack
                value = int(stack_match.group(1))                                  # Store the value by which the stack is opened
                new_lines.append(TEMPLATES["OPEN_STACK"].format(value - 28))       # Substitute the value with an updated one
                continue

            ###################################
            # CHECK FOR STORE WORD OPERATIONS #
            ###################################
            sw_match = PATTERNS["SW"].search(line)
            if sw_match and sw_found:                                              # If we match the use of a store word operation
                sw_instructions.append(line)                                       # Store it and continue       
            else:                                                                  # As soon as we do not find other sw operations
                sw_found = False    
                if not sw_added:
                    for j, i in enumerate(range(value, value - 28, -4)):
                        instruction = TEMPLATES["ADDITIONAL_SW"].format(j, abs(i)) # Create new instruction
                        new_lines.append(instruction)                              # Add new store word operations for the s registers
                        sw_instructions.append(instruction)                        # Save instruction in the list
                    sw_added = True

            #############################
            # CHECK FOR END OF FUNCTION #
            #############################
            if PATTERNS["CSRS"].search(line):                                      # If we match the end of the function
                function_found = False
                for sw_instruction in reversed(sw_instructions):
                    new_lines.append(sw_instruction.replace('sw', 'lw'))           # Add all the load word instruction in reverse order
                new_lines.append(TEMPLATES["OPEN_STACK"].format(abs(value - 28)))  # Close the stack 

        new_lines.append(line)

    # Write new lines
    with Path("intr_vector_table.s").open('w') as f:
        f.writelines(new_lines)

def instrument(assembly_files):
    print("Instrumenting files...\n")
    for assembly_file in assembly_files:                                           # For each file read its content and check
        print(f"Instrumenting file {os.path.basename(assembly_file)}...")
        replaced_jump = 0
        replaced_return = 0
        curr_function = ""                                                         # Function tracking
        registers_used = {f'a{i}': False for i in range(7)}                        # Register tracking

        with Path(assembly_file).open('r') as f:
            lines = f.readlines()

        new_lines = []
        for line in lines:
            ################################
            # CHECK FOR NEW FUNCTION START #
            ################################
            function_match = PATTERNS["FUNC_START"].search(line)
            if function_match:                                                     # If we match the start of a new function store its name
                curr_function = function_match.group(1)                            # Store the current function name

            ##########################
            # CHECK FOR REGISTER USE #
            ##########################
            register_match = PATTERNS["REGISTER_USE"].search(line)
            if register_match:                                                     # If we match an "a" register use
                reg = register_match.group(1)                                      # Get the used register
                registers_used[reg] = True                                         # Set it as true in the dictionary
            else:
                ##############################
                # CHECK FOR JUMP INSTRUCTION #
                ##############################
                jump_match = PATTERNS["DIR_JUMP"].search(line)
                if jump_match:                                                     # If we match a jump instruction
                    instr, label = jump_match.groups()                             # Get the instruction and the label
                    if label not in STD_C_FUNCS and not is_leaf(label):            # Check if target function is a leaf or a std C function. If it is, skip
                        for reg, used in registers_used.items():                   # Add mv instructions for used registers before ecall
                            if used:
                                sx = reg.replace('a', 's')                         # Replace 'a' with 's' to create sx (e.g. if a0 is used, s0 will be used)
                                new_lines.append(TEMPLATES["MV"].format(sx, reg))  # Append mv sx,ax

                        new_lines.append(TEMPLATES["JUMP"].format(label))          # If the function is neither a leaf nor a std func replace the instruction
                       
                        for reg, used in registers_used.items():                   # Add mv instructions for used registers after ecall
                            if used:
                                sx = reg.replace('a', 's')                         # Replace 'a' with 's' to create sx (e.g. if a0 is used, s0 will be used)
                                new_lines.append(TEMPLATES["MV"].format(reg, sx))  # Append mv ax,sx
                        
                        replaced_jump += 1
                        
                ################################
                # CHECK FOR RETURN INSTRUCTION #
                ################################
                ret_match = PATTERNS["RET"].search(line)                           # If we match a return instruction
                if ret_match and curr_function not in leaf_functions:              # Check if the current function is a leaf. If it is, skip
                    instr, label = ret_match.groups()                              # Get the instruction and the label
                    new_lines.append(TEMPLATES["RET"].format(label, label, label)) # Append the label that contains the return address
                    replaced_return += 1

                registers_used.update({f'a{i}': False for i in range(7)})          # Reset register tracking

            new_lines.append(line)

        # Write new lines
        with Path(assembly_file).open('w') as f:
            f.writelines(new_lines)
        print(f"File {os.path.basename(assembly_file)} had {replaced_jump} jump instruction(s) and {replaced_return} return instruction(s)\n")
    
    instrument_vector_table()

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

import sys
import re
import os
import CFGextractor
from pathlib import Path

PATTERNS = {
    "DIR_JUMP": re.compile(r'\b(call)\b\s+(\w+)'),                        # Regex to find direct jump instructions
    "UNDIR_JUMP": re.compile(r'\b(jalr)\b\s+(\w+)'),                      # Regex to find indirect jump instructions
    "RET": re.compile(r'\b(jr)\b\s+(\w+)'),                               # Regex to find return instructions
    "FUNC_START": re.compile(r'^[ \t]*([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*$'), # Regex to find the start of a function
    "STACK_OPENING": re.compile(r'addi\s+sp,sp,(-\d+)'),                  # Regex to find the opening of the stack
    "SW": re.compile(r'sw\s+\w+,\s*(\d+)\(sp\)'),                         # Regex to find the store word operations
    "CSRS": re.compile(r'^\s*csrs\s+mstatus\s*,\s*\w+\s*$')               # Regex to find the end of intr_vector_table function
}

TEMPLATES = {
    "JUMP": "\tla  a7,{}\n\tecall\n",                     # Template to substitute jump code
    "UNDIR_JUMP": "\tmv  a7,{}\n\tecall\n",               # Template to substitute undirect jump code
    "RET": "\tadd\ta7,{},1\n\tecall\n\taddi\t{},a7,-1\n", # Template to substitute return code
    "OPEN_STACK": "\taddi\tsp,sp,{}\n",                   # Template to open the stack
    "CALL": "\tauipc  a1,0\n\tmv  a0,{}\n\tcall\tprint_reg\n",
    # Template to save the context
    "SAVE_CONTEXT": """\t{}  ra, 124(sp)\n\t{}  t0, 120(sp)\n\t{}  t1, 116(sp)  
                       \n\t{}  t2, 112(sp)\n\t{}  s0, 108(sp)\n\t{}  s1, 104(sp)
                       \n\t{}  a0, 100(sp)\n\t{}  a1, 96(sp)\n\t{}  a2, 92(sp)
                       \n\t{}  a3, 88(sp)\n\t{}  a4, 84(sp)\n\t{}  a5, 80(sp)
                       \n\t{}  a6, 76(sp)\n\t{}  a7, 72(sp)\n\t{}  s2, 68(sp)
                       \n\t{}  s3, 64(sp)\n\t{}  s4, 60(sp)\n\t{}  s5, 56(sp)
                       \n\t{}  s6, 52(sp)\n\t{}  s7, 48(sp)\n\t{}  s8, 44(sp)
                       \n\t{}  s9, 40(sp)\n\t{}  s10, 36(sp)\n\t{}  s11, 32(sp)
                       \n\t{}  t3, 28(sp)\n\t{}  t4, 24(sp)\n\t{}  t5, 20(sp)
                       \n\t{}  t6, 16(sp)\n"""
}

# Lst of standard C functions
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
        if filename.name in {"boot.s", "main.s", "intr_vector_table.s", "shadow_stack.s", "cfg.s"}:
            continue
        
        with filename.open('r') as f:
            lines = f.readlines()

        function_found = False
        for line in lines:                              # If there is a match for the target, set found to true
            if target_pattern.search(line):
                function_found = True
                continue                                # Continue checking next lines

            if function_found:
                if PATTERNS["FUNC_START"].search(line): # If we match the start of a new function the target function is a leaf
                    leaf_functions.add(function_name)   # Add the function name to the list of leaf functions
                    return True
                if PATTERNS["DIR_JUMP"].search(line):   # If we match a jump instruction the target function is not a leaf
                    return False                                   

    leaf_functions.add(function_name)
    return True                                         # If no calls found, assume the function is a leaf                                      

def instrument_vector_table():
    print("Instrumenting interrupt vector table file...\n")
    with Path("intr_vector_table.s").open('r') as f:
        lines = f.readlines()

    new_lines = []
    function_found = False
    sw_found = True
    sw_added = False
    
    for line in lines:
        if re.search(r'^[ \t]*synchronous_exception_handler\s*:\s*$', line): # Regex to find the start of the desired function
            function_found = True                                            # If there is a match for the target, set found to true
            new_lines.append(line)
            continue

        if function_found:
            ####################################
            # CHECK FOR AMOUNT OF STACK OPENED #
            ####################################
            stack_match = PATTERNS["STACK_OPENING"].search(line)
            if stack_match:                                            # If we match the opening of the stack
                new_lines.append(TEMPLATES["OPEN_STACK"].format(-128)) # Substitute the value with an updated one
                continue

            ###################################
            # CHECK FOR STORE WORD OPERATIONS #
            ###################################
            if PATTERNS["SW"].search(line) and sw_found: # If we match the use of a store word operation
                continue
            else:                                        # As soon as we do not find other sw operations
                sw_found = False    
                if not sw_added:
                    new_lines.append(TEMPLATES["SAVE_CONTEXT"].format(*["sw"] * TEMPLATES["SAVE_CONTEXT"].count("{}"))) # Save context
                    sw_added = True

            #############################
            # CHECK FOR END OF FUNCTION #
            #############################
            if PATTERNS["CSRS"].search(line):                                                                       # If we match the end of the function
                function_found = False
                new_lines.append(TEMPLATES["SAVE_CONTEXT"].format(*["lw"] * TEMPLATES["SAVE_CONTEXT"].count("{}"))) # Restore context
                new_lines.append(TEMPLATES["OPEN_STACK"].format(128))                                               # Close the stack 

        new_lines.append(line)

    # Write new lines
    with Path("intr_vector_table.s").open('w') as f:
        f.writelines(new_lines)

def instrument(assembly_files):
    print("Instrumenting files...\n")
    undirect_jumps = False
    for assembly_file in assembly_files:
        print(f"Instrumenting file {os.path.basename(assembly_file)}...")
        replaced_jump = 0
        replaced_return = 0
        curr_function = "" # Function tracking

        with Path(assembly_file).open('r') as f:
            lines = f.readlines()

        new_lines = []
        for line in lines:
            ################################
            # CHECK FOR NEW FUNCTION START #
            ################################
            function_match = PATTERNS["FUNC_START"].search(line)
            if function_match:                          # If we match the start of a new function store its name
                curr_function = function_match.group(1) # Store the current function name

            ##############################
            # CHECK FOR JUMP INSTRUCTION #
            ##############################
            jump_match = PATTERNS["DIR_JUMP"].search(line)
            undir_jump_match = PATTERNS["UNDIR_JUMP"].search(line)
            if jump_match:                                            # If we match a direct jump or an undirect jump instruction
                instr, label = jump_match.groups()                    # Get the instruction and the label
                if label not in STD_C_FUNCS and not is_leaf(label):   # Check if target function is a leaf or a std C function. If it is, skip
                    new_lines.append(TEMPLATES["JUMP"].format(label)) # If the function is neither a leaf nor a std func replace the instruction
                    replaced_jump += 1
            elif undir_jump_match:
                instr, label = undir_jump_match.groups()                # Get the instruction and the label
                #new_lines.append(TEMPLATES["CALL"].format(label))       # Insert the code for the ecall
                new_lines.append(TEMPLATES["UNDIR_JUMP"].format(label)) # Insert the code for the ecall
                replaced_jump += 1
                undirect_jumps = True                    
                    
            ################################
            # CHECK FOR RETURN INSTRUCTION #
            ################################
            ret_match = PATTERNS["RET"].search(line)                           # If we match a return instruction
            if ret_match and curr_function not in leaf_functions:              # Check if the current function is a leaf. If it is, skip
                instr, label = ret_match.groups()                              # Get the instruction and the label
                new_lines.append(TEMPLATES["RET"].format(label, label, label)) # Append the label that contains the return address
                replaced_return += 1

            new_lines.append(line)

        # Write new lines
        with Path(assembly_file).open('w') as f:
            f.writelines(new_lines)
        print(f"File {os.path.basename(assembly_file)} had {replaced_jump} jump instruction(s) and {replaced_return} return instruction(s)\n")
    
    instrument_vector_table()

    if(undirect_jumps == True):
        CFGextractor.extract()

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

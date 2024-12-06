import sys
import re
import os
from pathlib import Path

PATTERNS = {
    "DIR_JUMP": re.compile(r'\b(call)\b\s+(\w+)'),                        # Regex to find direct jump instructions
    "INDIR_JUMP": re.compile(r'\b(jalr)\b\s+(\w+)'),                      # Regex to find indirect jump instructions
    "RET": re.compile(r'\b(jr)\b\s+(\w+)'),                               # Regex to find return instructions
    "FUNC_START": re.compile(r'^[ \t]*([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*$'), # Regex to find the start of a function
    "STACK_OPENING": re.compile(r'addi\s+sp,sp,(-\d+)'),                  # Regex to find the opening of the stack
    "SW": re.compile(r'sw\s+\w+,\s*(\d+)\(sp\)'),                         # Regex to find the store word operations
    "CSRS": re.compile(r'^\s*csrs\s+mstatus\s*,\s*\w+\s*$'),              # Regex to find the end of intr_vector_table function
    "REGISTERS": re.compile(r'\b([tas]\d+)\b')                            # Regex to match t, a, and s register use
}

TEMPLATES = {
    "JUMP": "\tla  a7,{}\n\tecall\n",                     # Template to substitute jump code
    "INDIR_JUMP": "\tmv  a7,{}\n\tecall\n",               # Template to substitute indirect jump code
    "RET": "\tadd\ta7,{},1\n\tecall\n\taddi\t{},a7,-1\n", # Template to substitute return code
    "OPEN_STACK": "\taddi\tsp,sp,{}\n",                   # Template to open the stack
    # Template to call the print_reg function
    "CALL": """\taddi\tsp,sp,-40\n\tsw  a5,4(sp)\n\tsw  a4,8(sp)
                \n\tsw  a2,12(sp)\n\tsw  a1,16(sp)\n\tsw  a0,20(sp)
                \n\tsw  s0,24(sp)\n\tsw  s1,28(sp)\n\tsw  s2,32(sp)
                \n\tsw  s3,36(sp)\n\tmv  a1,{}\n\tauipc  a0,0
                \n\taddi\ta0,a0,38\n\tcall\tprint_reg\n\tlw a5, 4(sp)
                \n\tlw  a4,8(sp)\n\tlw  a2,12(sp)\n\tlw  a1,16(sp)
                \n\tlw  a0,20(sp)\n\tlw  s0,24(sp)\n\tlw  s1,28(sp)
                \n\tlw  s2,32(sp)\n\tlw  s3,36(sp)\n\taddi\tsp,sp,40\n""",
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

# Function to search all leaves functions
def search_leaves():
    print("\nSearching for leaves functions...")
    
    for filename in Path(".").glob("*.s"):                       # For each file except peculiar ones, read content and check
        if filename.name in {"boot.s", "main.s", "intr_vector_table.s", "shadow_stack.s", "cfg.s", "ij_logger.s"}:
            continue
        
        with filename.open('r') as f:
            lines = f.readlines()

        curr_function = ""                                       # Keeps track of the current function name
        func_found = False                                       # Keeps track of whether we are inside a function or not
        curr_leaf = True                                         # Keeps track of whether the current fucntion is a leaf
        for line in lines:                             
            function_match = PATTERNS["FUNC_START"].search(line) 
            if function_match:                                   # If there is a match for the start of a function
                if curr_function != "" and curr_leaf:            # If the function is changing and the last function was a leaf store it
                    leaf_functions.add(curr_function)
                curr_function = function_match.group(1)          # Save new name for current function
                func_found = True                                # Set found to True
                curr_leaf = True                                 # Set leaf to True
                continue

            if func_found:                                       # If we are inside a function
                jump_match = PATTERNS["DIR_JUMP"].search(line)      
                if jump_match:                                   # If we match a direct jump
                    instr, label = jump_match.groups()           # Get the instruction and the label
                    if label not in STD_C_FUNCS:                 # Check if target function is a std C function. If it is, skip
                        curr_leaf = False                        # Set current leaf to false
                        func_found = False                       # Set found to false
                        continue                        
                if PATTERNS["INDIR_JUMP"].search(line):          # If we match an indirect jump
                    func_found = False                           # Set current leaf to false
                    curr_leaf = False                            # Set found to false
                    continue  

        if curr_leaf:                                            # When we reach EoF check if last function was a leaf
            leaf_functions.add(curr_function)                    # Store it 
            func_found = False                                   # Set found to False

# Function to instrument the interrupt_vector_table
def instrument_vector_table(registers):
    print("Instrumenting interrupt vector table...\n")
    with Path("intr_vector_table.s").open('r') as f:
        lines = f.readlines()

    new_lines = []
    func_found = False                                                       # Keeps track of whether we are inside the desired function or not
    sw_found = True                                                          # Keeps track of whether we found a store word operation or not
    sw_added = False                                                         # Keeps track of whether the instrumentation has been made or not

    # Build the save and restore context templates dynamically based on used registers
    stack_sz = (len(registers) + 2) * 4
    stack_offset = stack_sz - 4
    save_template = f"\tsw  ra,{stack_offset}(sp)\n"
    restore_template = f"\tlw  ra,{stack_offset}(sp)\n"
    for reg in sorted(registers):
        stack_offset -= 4                                       # Each register takes 4 bytes
        save_template += f"\tsw  {reg},{stack_offset}(sp)\n"    # Store the current register at a specific offset
        restore_template += f"\tlw  {reg},{stack_offset}(sp)\n" # Load the current register at a specific offset

    for line in lines:
        if re.search(r'^[ \t]*synchronous_exception_handler\s*:\s*$', line): # Regex to find the start of the desired function
            func_found = True                                                # If there is a match for the target, set found to true
            new_lines.append(line)
            continue

        if func_found:                                                       # If we are inside the desired function
            ####################################
            # CHECK FOR AMOUNT OF STACK OPENED #
            ####################################
            stack_match = PATTERNS["STACK_OPENING"].search(line)
            if stack_match:                                                  # If we match the opening of the stack
                new_lines.append(TEMPLATES["OPEN_STACK"].format(-stack_sz))  # Substitute the value with an updated one
                continue

            ###################################
            # CHECK FOR STORE WORD OPERATIONS #
            ###################################
            if PATTERNS["SW"].search(line) and sw_found:                     # If we match the use of a store word operation
                continue                                                     # Skip to erase those lines
            else:                                                            # As soon as we do not find other sw operations
                sw_found = False    
                if not sw_added:                                             # If we did not already updated the code
                    new_lines.append(save_template)                          # Write new save context block
                    sw_added = True

            #############################
            # CHECK FOR END OF FUNCTION #
            #############################
            if PATTERNS["CSRS"].search(line):                                # If we match the end of the function
                func_found = False
                new_lines.append(restore_template)                           # Write new restore context block
                new_lines.append(TEMPLATES["OPEN_STACK"].format(stack_sz))   # Close the stack 

        new_lines.append(line)

    # Write new lines
    with Path("intr_vector_table.s").open('w') as f:
        f.writelines(new_lines)

# Function to inject the CFG
def inject_cfg(src_addresses, dst_addresses):
    print("Injecting CFG...")

    pairs = list(zip(src_addresses, dst_addresses)) # Create pairs of (source, destination)
    unique_pairs = list(set(pairs))                 # Remove duplicates by converting the list to a set and back to a list
    unique_pairs.sort(key=lambda x: (x[0], x[1]))   # Sort pairs based on source first, then destination (for binary search)

    cfg_size = len(unique_pairs)                    # Compute size of the cfg
    new_size_str = f"size_t cfg_size = {cfg_size};" # Build the cfg_size string

    cfg_space = 4 * ((cfg_size - 1) * 2)            # Compute the amount of space occupied by the cfg (-1 is because by default the cfg is {1,1} so we do not count such value)

    # Create new cfg string based on sorted pairs and by adding the space occupied by the cfg
    new_cfg_str = "__attribute__((section(\".cfg\"))) unsigned int cfg[][2] = {"
    new_cfg_str += ", ".join(f"{{{src + cfg_space}, {dst + cfg_space}}}" for src, dst in unique_pairs)
    new_cfg_str += "};\n"

    with open("../src/cfi/cfg.c", 'r') as f:
        lines = f.read()

    cfg_pattern = r'__attribute__\(\(section\("\.cfg"\)\)\) unsigned int cfg\[\]\[2\] = {.*?};' # Pattern for old CFG definition
    size_pattern = r'size_t cfg_size = \d+;'                                                    # Pattern for old size definition

    lines = re.sub(cfg_pattern, new_cfg_str.strip(), lines, flags=re.DOTALL) # Replace the old CFG definition
    lines = re.sub(size_pattern, new_size_str, lines)                        # Replace the old size definition

    # Write the modified content back to the file
    with open("../src/cfi/cfg.c", 'w') as f:
        f.write(lines)

    with open("../src/cfi/intr_vector_table.c", 'r') as f:
        lines = f.read()
    
    # Pattern for the old code block (without forward controls)
    code_block_pattern = (
        r'if\s*\(\s*push\(source\s*\+\s*2\)\s*!=\s*1\s*\)\s*{'
        r'\s*code_termination\(\);\s*}'
    )

    # Replacement code block as a multiline string
    new_code_block = r"""
    if (check(source, ecode_address_encoding)) {
            if (push(source + 2) != 1) {
                code_termination();
            }
        } else {
            code_termination();
        }
    """

    lines = re.sub(code_block_pattern, new_code_block.strip(), lines, flags=re.DOTALL) # Replace the old code block with the forward control one

    # Write the modified content back to the file
    with open("../src/cfi/intr_vector_table.c", 'w') as f:
        f.write(lines)

    print("CFG injected correctly")

def restore_vector_table():

    new_cfg_str = "__attribute__((section(\".cfg\"))) unsigned int cfg[][2] = {{1,1}};" # Replacement string

    with open("../src/cfi/cfg.c", 'r') as f:
        lines = f.read()

    cfg_pattern = r'__attribute__\(\(section\("\.cfg"\)\)\) unsigned int cfg\[\]\[2\] = {.*?};' # Pattern for the old CFG definition to be replaced

    modified_lines = re.sub(cfg_pattern, new_cfg_str, lines, flags=re.DOTALL) # Replace the old CFG definition with the new one

    # Write the modified content back to the file
    with open("../src/cfi/cfg.c", 'w') as f:
        f.write(modified_lines)

    with open("../src/cfi/intr_vector_table.c", 'r') as f:
        lines = f.read()

    # Pattern for the old code block (with forward controls)
    code_block_pattern = (
        r'if\s*\(\s*check\(\s*source,\s*ecode_address_encoding\)\s*\)\s*{'
        r'\s*if\s*\(\s*push\(source\s*\+\s*2\)\s*!=\s*1\s*\)\s*{'
        r'\s*code_termination\(\);\s*}'
        r'\s*}'
        r'\s*else\s*{'
        r'\s*code_termination\(\);\s*}'
    )

    # Replacement code block as a multiline string
    new_code_block = r"""
    if (push(source + 2) != 1) {
            code_termination();
        }
    """

    lines = re.sub(code_block_pattern, new_code_block.strip(), lines, flags=re.DOTALL) # Replace the forward control code block with the normal one

    # Write the modified content back to the file
    with open("../src/cfi/intr_vector_table.c", 'w') as f:
        f.write(lines)

# Function to instrument the code
def instrument(assembly_files, CFGLogging=False):
    search_leaves()                                     # Search for leaves functions

    print("\nInstrumenting files...\n")
    
    ind_jumps = False                                   # Keeps track of whether we have found indirect jumps or not
    registers = set()                                   # Keeps track of all the used registers 
    for assembly_file in assembly_files:
        print(f"Instrumenting file {os.path.basename(assembly_file)}...")
        replaced_jump = 0                               # Keeps track of how many jumps were found
        replaced_return = 0                             # Keeps track of how many return were found
        curr_function = ""                              # Keeps track of the name of the current function

        with Path(assembly_file).open('r') as f:
            lines = f.readlines()

        new_lines = []
        for line in lines:
            ################################
            # CHECK FOR NEW FUNCTION START #
            ################################
            function_match = PATTERNS["FUNC_START"].search(line)
            if function_match:                          # If we match the start of a new function
                curr_function = function_match.group(1) # Store the current function name

            #######################
            # CHECK FOR REGISTERS #
            #######################
            reg_match = PATTERNS["REGISTERS"].findall(line)
            if reg_match:
                registers.update(reg_match)        # Add the matched registers to the set for this file

            ##############################
            # CHECK FOR JUMP INSTRUCTION #
            ##############################
            jump_match = PATTERNS["DIR_JUMP"].search(line)
            ind_jump_match = PATTERNS["INDIR_JUMP"].search(line)
            if jump_match:                                                     # If we match a direct jump instruction
                instr, label = jump_match.groups()                             # Get the instruction and the label
                if label not in STD_C_FUNCS and label not in leaf_functions:   # Check if target function is a leaf or a std C function. If it is, skip
                    new_lines.append(TEMPLATES["JUMP"].format(label))          # If the function is neither a leaf nor a std C function replace the instruction
                    replaced_jump += 1
            elif ind_jump_match:                                               # If we match an indirect jump instruction
                instr, label = ind_jump_match.groups()                         # Get the instruction and the label
                if CFGLogging:                                                 # If we are logging for the simulation
                    new_lines.append(TEMPLATES["CALL"].format(label))          # Insert the code to log indirect jump addresses
                new_lines.append(TEMPLATES["INDIR_JUMP"].format(label))        # Insert the code for the ecall
                replaced_jump += 1
                ind_jumps = True                    
                    
            ################################
            # CHECK FOR RETURN INSTRUCTION #
            ################################
            ret_match = PATTERNS["RET"].search(line)                           # If we match a return instruction
            if ret_match and curr_function not in leaf_functions:              # Check if the current function is a leaf. If it is, skip
                instr, label = ret_match.groups()                              # Get the instruction and the label
                new_lines.append(TEMPLATES["RET"].format(label, label, label)) # Insert the code for the ecall
                replaced_return += 1

            new_lines.append(line)

        # Write new lines
        with Path(assembly_file).open('w') as f:
            f.writelines(new_lines)
            
        print(f"File {os.path.basename(assembly_file)} had {replaced_jump} jump instruction(s) and {replaced_return} return instruction(s)\n")
                    
    instrument_vector_table(registers)
    return ind_jumps

def main():
    print("This file is used to instrument the code.\nPlease use flasher.py to perform the complete build automatically")
    sys.exit(0)

if __name__ == "__main__":
    main()

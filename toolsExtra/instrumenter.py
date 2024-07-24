import sys, re

# When there is a match for:
    # Jump: check that target function is non-leaf, if it's not then add template
    # Return: check that current function is non-leaf, if it's not then add template

jump = r'\b(call)\b\s+(\w+)' # Regex to find jump instructions
ret = r'\b(jr)\b\s+(\w+)'    # Regex to find return instructions
func_str = r'[^.]\b(\w+):\n' # Regex to find start of function

jump_template = ("\blw a7, {}\n"
                "\becall\n") # Template to substitute jump code

ret_template = ("\blw a7, {}\n"
                "\baddi a7, a7, 1\n"
                "\becall\n") # Template to substitute return code

# Check if target function is a leaf
def is_leaf(function_name, files):
    print("Checking...")
    target = r'[^.]\b(' + function_name + r'):\n' # Regex to find the start of the desired function

    for file in files: # For each file read its content and check
        with open(file, 'r') as f:
            lines = f.readlines()
        
        found = False 
        for line in lines:
            target_match = re.search(target, line) # If there is a match for the target set found to true
            if target_match:
                found = True

            if found: # If the function has been found check if is a leaf
                jump_match = re.search(jump, line) # If we match a jump instruction return False since the function is not a leaf
                if jump_match:
                    return False
                
                func_str_match = re.search(func_str, line) # If we match the start of a new function return True since the target function is a leaf
                if func_str_match:
                    return True

def instrument(assembly_files):
    print("Instrumenting files...")
    replaced_jump = 0
    replaced_return = 0
    for assembly_file in assembly_files: # For each file read its content and check
        with open(assembly_file, 'r') as f:
            lines = f.readlines()

        new_lines = []
        curr_leaf = True # Needed to keep track of current function status
        for line in lines:
            func_str_match = re.search(func_str, line) # If we match the start of a new function reset the leaf value
            if func_str_match:
                curr_leaf = True

            jump_match = re.search(jump, line) # If we match a jump instruction set the leaf value to False
            if jump_match:
                curr_leaf = False
                instr, label = jump_match.groups() # Also check that target function is not a leaf
                if not is_leaf(label, assembly_files):
                    new_lines.append(jump_template.format(label)) # If the function is not a leaf replace the instruction
                    replaced_jump += 1

            ret_match = re.search(ret, line) # If we match a return instruction and the current function is not a leaf replace instruction
            if ret_match and not curr_leaf:
                instr, label = ret_match.groups()
                new_lines.append(ret_template.format(label)) 
                replaced_return += 1

            new_lines.append(line) # Write new lines

        with open(assembly_file, 'w') as f:
            f.writelines(new_lines)

        print(f"File {assembly_file} had {replaced_jump} jump instructions and {replaced_return} return instructions")

def main():
    if len(sys.argv) < 2:
        print("This file is used to instrument the code.\n"
                "Input files should be provided in the following order.\n"
                "python3 instrumenter.py [file_1.s] [file_2.s] [...] [file_N.s]")
        exit(1)

    instrument(sys.argv[1:])

if __name__ == "__main__":
    main()
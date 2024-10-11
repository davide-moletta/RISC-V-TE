from pathlib import Path
import sys
import re

src_addresses = []
dst_addresses = []
block_size = 56
print_reg_size = 30

section_pattern = re.compile(r'Disassembly of section \s*.([a-zA-Z0-9_]*):')
jal_pattern = re.compile(r'^\s*([0-9a-fA-F]+):\s+[0-9a-fA-F]+\s+jal\s+([0-9a-fA-F]+)')

# Function to find all logging blocks
def find_blocks(file):
    logging_blocks_starting_addresses = []
    print_reg_address = 0

    # Regex pattern to match the entire block of instructions
    block_pattern = re.compile(r"""^\s*([0-9a-fA-F]+):\s+[0-9a-fA-F]+\s+addi\s+sp,sp,-40\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+a5,4\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+a4,8\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+a2,12\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+a1,16\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+a0,20\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+s0,24\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+s1,28\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+s2,32\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+sw\s+s3,36\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+mv\s+a1,[0-9a-fA-F]+\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+auipc\s+a0,0x0\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+addi\s+a0,a0,38\s+.+\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+jal\s+[0-9a-fA-F]+.*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+a5,4\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+a4,8\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+a2,12\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+a1,16\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+a0,20\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+s0,24\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+s1,28\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+s2,32\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+lw\s+s3,36\(sp\)\s*\n
^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+addi\s+sp,sp,40\s*""", re.VERBOSE | re.MULTILINE)
    
    print_reg_pattern = re.compile(r'^([0-9a-fA-F]+)\s+<print_reg>:', re.MULTILINE)

    with open(file, 'r') as file:
        content = file.read()

    # Find all matches of the block pattern
    matches = block_pattern.findall(content)
    # Find all matches of the print_reg pattern
    pr_matches = print_reg_pattern.findall(content)

    if pr_matches:
        for idx, pr_match in enumerate(pr_matches, start=1):
            print_reg_address = int(pr_match, 16)

    # Output all found blocks
    if matches:
        for idx, match in enumerate(matches, start=1):
            logging_blocks_starting_addresses.append(int(match, 16)) # Append the first address of each logging block

    return print_reg_address, logging_blocks_starting_addresses

def extract(output_string, blocks, print_reg_address, file):
    print("\nExtracting Control Flow Graph...")

    # Extract Source and Destination from the output and store it
    if output_string != "":
        print("Examining output for undirect jumps...")
        lines = output_string.split("\n")
        for line in lines:
            match = re.search(r"Source:\s*([0-9A-Fa-f]+)\s*-\s*Destination:\s*([0-9A-Fa-f]+)", line)
            if match:                               # For each matching line
                src_addr = int(match.group(1), 16)  # Extract source address
                dst_addr = int(match.group(2), 16)  # Extract destination address

                src_num_blocks = 0
                dst_num_blocks = 0
                for block in blocks:                # For each block
                    if src_addr > block:            # If the block was before the source address 
                        src_num_blocks+=1           # Add 1 to the number of blocks before
                    if dst_addr > block:            # If the block was before the destination address 
                        dst_num_blocks+=1           # Add 1 to the number of blocks before

                if src_addr > print_reg_address:
                    src_addr = src_addr - print_reg_size
                if dst_addr > print_reg_address:
                    dst_addr = dst_addr - print_reg_size

                src_addresses.append(src_addr - (src_num_blocks * block_size))
                dst_addresses.append(dst_addr - (dst_num_blocks * block_size))
    
    # Examine firmware.s to extract Source and Destination for direct jumps
    print("Examining .s file for direct jumps...\n")
    with Path(file).open('r') as f:
        lines = f.readlines()

    text_section = False
    for line in lines:
        section_match = section_pattern.match(line)
        if section_match:
            section_name = section_match.group(1)
            if section_name == "text":
                text_section = True
            if section_name != "text" and text_section:
                break
        
        if text_section:
            jal_match = jal_pattern.match(line)
            if jal_match:
                src_addresses.append(int(jal_match.group(1), 16))
                dst_addresses.append(int(jal_match.group(2), 16))

    # print(f"Source addresses: {src_addresses}")
    # print(f"Destination addresses: {dst_addresses}")

def main():
    print("This file is used to extract the Control Flow Graph. Please use the flasher to do so.")
    extract("", [], "firmware.s")
    sys.exit(0)

if __name__ == "__main__":
    main()

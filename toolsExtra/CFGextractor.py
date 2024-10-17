from pathlib import Path
import sys
import re
import instrumenter

# Function to find all logging blocks
def find_blocks(file):
    logging_blocks_starting_addresses = []

    # Regex pattern to match the entire block of instructions for the indirect jump logging
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
    
    with open(file, 'r') as file:
        content = file.read()

    matches = block_pattern.findall(content) # Find all matches of the block pattern
    if matches:
        for idx, match in enumerate(matches, start=1):
            logging_blocks_starting_addresses.append(int(match, 16)) # Append the first address of each logging block

    return logging_blocks_starting_addresses

def extract(output_string, blocks, file):
    print("\nExtracting Control Flow Graph...")
    src_addresses = []
    dst_addresses = []
    block_size = 56         # Size of a logging block in Bytes
    forward_check_size = 84 # Size of the forward control block in Bytes

    # Extract Source and Destination from the output and store it
    if output_string != "":
        print("Examining output for indirect jumps...")
        lines = output_string.split("\n")
        for line in lines:
            match = re.search(r"Source:\s*([0-9A-Fa-f]+)\s*-\s*Destination:\s*([0-9A-Fa-f]+)", line) # Regex to find and extract source and destination address from output
            if match:                                    # For each matching line
                src_addr = int(match.group(1), 16)       # Extract source address
                dst_addr = int(match.group(2), 16)       # Extract destination address

                for block in blocks:                     # For each block
                    if src_addr > block:                 # If the block was before the source address 
                        src_addr -= block_size           # Remove the amount occupied by a block from the source address
                    if dst_addr > block:                 # If the block was before the destination address 
                        dst_addr -= block_size           # Remove the amount occupied by a block from the destination address

                src_addr += forward_check_size           # Add to the source address the amount occupied by the forward control (since it's added after)           
                dst_addr += forward_check_size           # Add to the destination address the amount occupied by the forward control (since it's added after)

                src_addresses.append(src_addr)           # Append the correctly computed source address
                dst_addresses.append(dst_addr)           # Append the correctly computed destination address
    
    # Examine firmware.s to extract Source and Destination for direct jumps
    print("Examining .s file for direct jumps...\n")
    section_pattern = re.compile(r'Disassembly of section \s*.([a-zA-Z0-9_]*):')           # Pattern to find start of section
    jal_pattern = re.compile(r'^\s*([0-9a-fA-F]+):\s+[0-9a-fA-F]+\s+jal\s+([0-9a-fA-F]+)') # Pattern to find a jal instruction

    with Path(file).open('r') as f:
        lines = f.readlines()

    text_section = False
    for line in lines:
        section_match = section_pattern.match(line)               
        if section_match:                                         # If we match the start of a section 
            section_name = section_match.group(1)
            if section_name == "text":                            # If it is the text section
                text_section = True                               # Set text_section to True
            if section_name != "text" and text_section:           # If we are exiting from the text section stop inspecting
                break
        
        if text_section:
            jal_match = jal_pattern.match(line)                   
            if jal_match:                                                              # If we match a jal isntruction and we are inside the text section 
                src_addresses.append(int(jal_match.group(1), 16) + forward_check_size) # Append the source address
                dst_addresses.append(int(jal_match.group(2), 16) + forward_check_size) # Append the destination address

    instrumenter.inject_cfg(src_addresses, dst_addresses) # Inject CFG

def main():
    print("This file is used to extract the Control Flow Graph.\n Please use the flasher to do so.")
    sys.exit(0)

if __name__ == "__main__":
    main()

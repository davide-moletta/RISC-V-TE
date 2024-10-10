from pathlib import Path
import sys
import re

src_addresses = []
dst_addresses = []
block_size = 1

section_pattern = re.compile(r'^\s*\.([a-zA-Z_][a-zA-Z0-9_]*):')
jal_pattern = re.compile(r'^\s*([0-9a-fA-F]+):\s+([0-9a-fA-F]{8})\s+jal\s+([0-9a-fA-F]+)\s+<(?!printf>).*')


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

# Function to find all logging blocks
def find_blocks(file):
    logging_blocks_starting_addresses = []

    # Regex pattern to match the entire block of instructions
    block_pattern = re.compile(
        r"""
        ^\s*([0-9a-fA-F]+):\s+fd810113\s+addi\s+sp,sp,-40\s*\n
        ^\s*([0-9a-fA-F]+):\s+c23e\s+sw\s+a5,4\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+c43a\s+sw\s+a4,8\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+c632\s+sw\s+a2,12\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+c82e\s+sw\s+a1,16\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+ca2a\s+sw\s+a0,20\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+cc22\s+sw\s+s0,24\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+ce26\s+sw\s+s1,28\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+d04a\s+sw\s+s2,32\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+d24e\s+sw\s+s3,36\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+85be\s+mv\s+a1,a5\s*\n
        ^\s*([0-9a-fA-F]+):\s+00000517\s+auipc\s+a0,0x0\s*\n
        ^\s*([0-9a-fA-F]+):\s+3789\s+jal\s+[0-9a-fA-F]+.*\n
        ^\s*([0-9a-fA-F]+):\s+4792\s+lw\s+a5,4\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+4722\s+lw\s+a4,8\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+4632\s+lw\s+a2,12\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+45c2\s+lw\s+a1,16\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+4552\s+lw\s+a0,20\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+4462\s+lw\s+s0,24\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+44f2\s+lw\s+s1,28\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+5902\s+lw\s+s2,32\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+5992\s+lw\s+s3,36\(sp\)\s*\n
        ^\s*([0-9a-fA-F]+):\s+02810113\s+addi\s+sp,sp,40\s*
        """, re.VERBOSE | re.MULTILINE)

    # Read the file content
    with open(file, 'r') as file:
        content = file.read()

    # Find all matches of the block pattern
    matches = block_pattern.findall(content)

    # Output all found blocks
    if matches:
        for idx, match in enumerate(matches, start=1):
            logging_blocks_starting_addresses.append(int(match[0], 16)) # Append the first address of each logging block

    return logging_blocks_starting_addresses

def extract(output_string, blocks, file):
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
                        src_num_blocks+1            # Add 1 to the number of blocks before
                    if dst_addr > block:            # If the block was before the destination address 
                        dst_num_blocks+1            # Add 1 to the number of blocks before

                src_addresses.append(src_addr - (src_num_blocks * block_size))
                dst_addresses.append(dst_addr - (src_num_blocks * block_size))
    
    # Examine firmware.s to extract Source and Destination for direct jumps
    print("Examining .s file for direct jumps...\n")
    with Path(file).open('r') as f:
        lines = f.readlines()

    text_section = False
    for line in lines:
        section_match = section_pattern.match(line)
        if section_match:
            section_name = section_match.group(1)
            if section_name == ".text":
                text_section = True
            if section_name != ".text" and text_section:
                break
        
        if text_section:
            jal_match = jal_pattern.match(line)
            if jal_match:
                src_addresses.append(int(jal_match.group(1), 16))
                dst_addresses.append(int(jal_match.group(3), 16))

    print(f"Source addresses: {src_addresses}")
    print(f"Destination addresses: {dst_addresses}")

def main():
    print("This file is used to extract the Control Flow Graph. Please use the flasher to do so.")
    extract("", [], "firmware.s")
    sys.exit(0)

if __name__ == "__main__":
    main()

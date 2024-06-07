#!/usr/bin/env python
import argparse
from PIL import Image
import os

# Variable to track whether find_next_base_number has been called already
next_base_number_called = False
# Variable to store the current base_number
current_base_number = 10

def find_next_base_number(base_dir):
    """
    Find the next base number to start a new block of ten.
    """
    global next_base_number_called, current_base_number
    if next_base_number_called:
        return current_base_number
    next_base_number_called = True
    
    existing_files = [int(os.path.splitext(f)[0]) for f in os.listdir(base_dir) if f.endswith('.clk')]
    if not existing_files:
        return current_base_number
    max_base_number = max(existing_files)
    current_base_number = ((max_base_number // 10) + 1) * 10
    return current_base_number

def find_available_filename(base_dir):
    """
    Find an available filename by incrementing the number part.
    """
    base_number = find_next_base_number(base_dir)
    used_numbers = set(int(os.path.splitext(f)[0]) for f in os.listdir(base_dir) if f.endswith('.clk'))

    number = base_number
    while True:
        if number not in used_numbers:
            return os.path.join(base_dir, f"{number:02d}.clk")
        
        # If the number is already used, increment it
        number += 1        

def convert_image(input_file_path, output_file_path):
    try:
        # Load the image
        img = Image.open(input_file_path)
    except Exception as e:
        raise ValueError(f"Error loading image file '{input_file_path}': {str(e)}")
    
    # Get original image size
    original_width, original_height = img.size

    # Output original image resolution
    print(f"Original resolution: {original_width}x{original_height}")

    # Resize the image while preserving the aspect ratio
    img.thumbnail((135, 240), resample=Image.NEAREST)

    # Create a new image with a black background of size 240x135
    background = Image.new('RGB', (135, 240), (0, 0, 0))
    
    # Paste the resized image onto the black background
    offset = ((135 - img.width) // 2, (240 - img.height) // 2)
    background.paste(img, offset)

    # Get resized image size
    resized_width, resized_height = background.size
    # Output resized image resolution
    print(f"Resized resolution: {resized_width}x{resized_height}")
    
    # Convert to RGB mode to ensure we can get RGB values
    background = background.convert('RGB')

    W, H = background.size

    # Prepare the header
    header = bytearray([ord('C'), ord('K'), W & 0xFF, (W >> 8) & 0xFF, H & 0xFF, (H >> 8) & 0xFF])

    # Create a memory stream (byte array) and write the header
    memory_stream = bytearray()
    memory_stream.extend(header)

    # Process and write pixel data
    for y in range(H):
        for x in range(W):
            R, G, B = background.getpixel((x, y))
            OutPixel = ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3)
            memory_stream.extend(OutPixel.to_bytes(2, byteorder='little'))

    # Save to file
    with open(output_file_path, 'wb') as file:
        file.write(memory_stream)

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Convert images to CLK format.')
    parser.add_argument('input', nargs='+', help='Input image files or directories to convert')
    parser.add_argument('--output_dir', type=str, default='.', help='Output directory for converted CLK files')
    args = parser.parse_args()

    # Create the output directory if it doesn't exist
    os.makedirs(args.output_dir, exist_ok=True)

    # Convert each input file or all files in input directory
    for input_path in args.input:
        if os.path.isfile(input_path):
            input_files = [input_path]
        elif os.path.isdir(input_path):
            input_files = [os.path.join(input_path, f) for f in os.listdir(input_path) if os.path.isfile(os.path.join(input_path, f))]
        else:
            print(f"Error: Invalid input path '{input_path}'")
            continue

        # Convert each input file
        for input_file in input_files:
            # Generate output file name
            output_file = find_available_filename(args.output_dir)
            
            try:
                convert_image(input_file, output_file)
                print(f"Converted '{input_file}' to '{output_file}'.")
            except Exception as e:
                print(f"Error converting '{input_file}': {str(e)}")
                continue

if __name__ == '__main__':
    main()

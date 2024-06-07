#!/usr/bin/env python
import argparse
from PIL import Image
import os

# Variable, um zu verfolgen, ob find_next_base_number bereits aufgerufen wurde
next_base_number_called = False
# Variable zur Speicherung der aktuellen base_number
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

def convert_image(input_file_path, output_file_path, target_width=None, target_height=None):
    try:
        # Load the image
        img = Image.open(input_file_path)
    except Exception as e:
        raise ValueError(f"Error loading image file '{input_file_path}': {str(e)}")

    # Get original image size
    original_width, original_height = img.size

    # Output original image resolution
    print(f"Original resolution: {original_width}x{original_height}")

    # Resize the image if target width and height are provided
    if target_width is not None and target_height is not None:
        img = img.resize((target_width, target_height), resample=Image.NEAREST)

        # Get resized image size
        resized_width, resized_height = img.size

        # Convert to RGB mode to ensure we can get RGB values
        img = img.convert('RGB')

        # Output resized image resolution
        print(f"Resized resolution: {resized_width}x{resized_height}")
    else:
        # Convert to RGB mode to ensure we can get RGB values
        img = img.convert('RGB')
    
    W, H = img.size

    # Prepare the header
    header = bytearray([ord('C'), ord('K'), W & 0xFF, (W >> 8) & 0xFF, H & 0xFF, (H >> 8) & 0xFF])

    # Create a memory stream (byte array) and write the header
    memory_stream = bytearray()
    memory_stream.extend(header)

    # Process and write pixel data
    for y in range(H):
        for x in range(W):
            R, G, B = img.getpixel((x, y))
            OutPixel = (((R * 31 + 127) // 255) << 11) | (((G * 63 + 127) // 255) << 5) | ((B * 31 + 127) // 255)
            memory_stream.extend(OutPixel.to_bytes(2, byteorder='little'))

    # Save to file
    with open(output_file_path, 'wb') as file:
        file.write(memory_stream)

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Convert images to CLK format.')
    parser.add_argument('input', nargs='+', help='Input image files or directories to convert')
    parser.add_argument('--output_dir', type=str, default='.', help='Output directory for converted CLK files')
    parser.add_argument('--width', type=int, default=None, help='Target width for resizing images')
    parser.add_argument('--height', type=int, default=None, help='Target height for resizing images')
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
                convert_image(input_file, output_file, args.width, args.height)
                print(f"Converted '{input_file}' to '{output_file}'.")
            except Exception as e:
                print(f"Error converting '{input_file}': {str(e)}")

if __name__ == '__main__':
    main()

import tkinter as tk
from PIL import Image, ImageTk
import os
import argparse
import glob

class CLKViewerApp:
    def __init__(self, root, clk_folder):
        # root is the parent window for the CLK Viewer
        self.root = root
        # Set the title of the window
        self.root.title("CLK Viewer")
        # Set a fixed size for the root window
        self.root.geometry("400x400")
    
        # clk_folder is the directory where CLK files are stored
        self.clk_folder = clk_folder
        # Get a list of all CLK files in the clk_folder
        self.clk_files = glob.glob(os.path.join(clk_folder, '*.clk'))
    
        # current_index is used to keep track of the current image being displayed
        self.current_index = 0
    
        # Create a frame to display the image
        self.image_frame = tk.Frame(root)
        self.image_frame.pack(fill=tk.BOTH, expand=True)
    
        # image_label is the label where the CLK image will be displayed
        self.image_label = tk.Label(self.image_frame)
        self.image_label.pack()
    
        # Create a frame for the filename label at the bottom of the window
        self.filename_frame = tk.Frame(root)
        self.filename_frame.pack(side=tk.BOTTOM, fill=tk.X)

        # filename_label is the label where the filename of the CLK image will be displayed
        self.filename_label = tk.Label(self.filename_frame)
        self.filename_label.pack(side=tk.LEFT)
    
        # Create a frame for the controls at the bottom of the window
        self.controls_frame = tk.Frame(root)
        self.controls_frame.pack(side=tk.BOTTOM, fill=tk.X)
    
        # prev_button is the button to show the previous image
        self.prev_button = tk.Button(self.controls_frame, text="Previous", command=self.show_previous_image)
        self.prev_button.pack(side=tk.LEFT)
    
        # next_button is the button to show the next image
        self.next_button = tk.Button(self.controls_frame, text="Next", command=self.show_next_image)
        self.next_button.pack(side=tk.RIGHT)
    
        # Display the first image
        self.show_image()

    def show_image(self):
        clk_file = self.clk_files[self.current_index]
        clk_filename = os.path.basename(clk_file)
        
        # Load the CLK image
        with open(clk_file, 'rb') as file:
            clk_data = file.read()

        # Convert CLK to BMP in memory
        bmp_data = self.convert_clk_to_bmp(clk_data)

        image = Image.frombytes('RGB', (clk_data[2], clk_data[4]), bmp_data[54:], 'raw', 'BGR;16')
        photo = ImageTk.PhotoImage(image)
        self.image_label.configure(image=photo)
        self.image_label.image = photo
        
        # Display filename
        self.filename_label.config(text="Filename: " + clk_filename)

    def show_previous_image(self):
        self.current_index = (self.current_index - 1) % len(self.clk_files)
        self.show_image()

    def show_next_image(self):
        self.current_index = (self.current_index + 1) % len(self.clk_files)
        self.show_image()

    def convert_clk_to_bmp(self, clk_data):
        width = clk_data[2]
        height = clk_data[4]

        bmp_data = bytearray()
        bmp_data.extend(b'BM')  # BMP-Signature
        bmp_data.extend((54 + (width * height * 2)).to_bytes(4, 'little'))  # File size
        bmp_data.extend(b'\x00\x00\x00\x00')  # Reserved
        bmp_data.extend((54).to_bytes(4, 'little'))  # Offset to image data
        bmp_data.extend((40).to_bytes(4, 'little'))  # Size of BITMAPINFO header
        bmp_data.extend(width.to_bytes(4, 'little'))  # Width
        bmp_data.extend(height.to_bytes(4, 'little'))  # Height
        bmp_data.extend(b'\x01\x00')  # Number of color planes
        bmp_data.extend(b'\x10\x00')  # Number of bits per pixel (16)
        bmp_data.extend(b'\x00\x00\x00\x00')  # Compression (none)
        bmp_data.extend((width * height * 2).to_bytes(4, 'little'))  # Image size
        bmp_data.extend(b'\x13\x0B\x00\x00\x13\x0B\x00\x00')  # Horizontal and vertical resolution
        bmp_data.extend(b'\x00\x00\x00\x00')  # Number of colors in the palette
        bmp_data.extend(b'\x00\x00\x00\x00')  # Number of important colors

        # Extract color values and convert to BMP format
        for i in range(6, len(clk_data), 2):
            pixel = ((clk_data[i + 1] & 0xF8) << 8) | ((clk_data[i] & 0xFC) << 3) | (clk_data[i] >> 3)
            bmp_data.extend(pixel.to_bytes(2, 'little'))

        return bmp_data
    # def convert_clk_to_bmp(self, clk_data):
    #     width = clk_data[2]
    #     height = clk_data[4]

    #     bmp_data = bytearray()
    #     bmp_data.extend(b'BM')  # BMP-Signature
    #     bmp_data.extend((54 + (width * height * 6)).to_bytes(4, 'little'))  # File size
    #     bmp_data.extend(b'\x00\x00\x00\x00')  # Reserved
    #     bmp_data.extend((54).to_bytes(4, 'little'))  # Offset to image data
    #     bmp_data.extend((40).to_bytes(4, 'little'))  # Size of BITMAPINFO header
    #     bmp_data.extend(width.to_bytes(4, 'little'))  # Width
    #     bmp_data.extend(height.to_bytes(4, 'little'))  # Height
    #     bmp_data.extend(b'\x01\x00')  # Number of color planes
    #     bmp_data.extend(b'\x30\x00')  # Number of bits per pixel (48)
    #     bmp_data.extend(b'\x00\x00\x00\x00')  # Compression (none)
    #     bmp_data.extend((width * height * 6).to_bytes(4, 'little'))  # Image size
    #     bmp_data.extend(b'\x13\x0B\x00\x00\x13\x0B\x00\x00')  # Horizontal and vertical resolution
    #     bmp_data.extend(b'\x00\x00\x00\x00')  # Number of colors in the palette
    #     bmp_data.extend(b'\x00\x00\x00\x00')  # Number of important colors

    #     # Extract color values and convert to BMP format
    #     for i in range(6, len(clk_data), 2):
    #         R = clk_data[i + 1] & 0xF8
    #         G = clk_data[i] & 0xFC
    #         B = clk_data[i] & 0xF8
    #         pixel = (R << 24) | (G << 16) | (B << 8)
    #         bmp_data.extend(pixel.to_bytes(6, 'little'))

    #     return bmp_data

def main():
    parser = argparse.ArgumentParser(description='View CLK files in a GUI.')
    parser.add_argument('clk_folder', type=str, help='Folder containing CLK files')
    args = parser.parse_args()

    root = tk.Tk()
    app = CLKViewerApp(root, args.clk_folder)
    root.mainloop()

if __name__ == "__main__":
    main()

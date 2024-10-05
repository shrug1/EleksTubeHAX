import os
import re

def rename_bitmaps(directory):
    # List all files in the directory
    files = os.listdir(directory)
    
    # Filter out .bmp files
    bmp_files = [f for f in files if f.endswith('.bmp')]
    
    # Extract numbers from filenames and find the highest one
    numbers = [int(re.search(r'(\d+)', f).group(1)) for f in bmp_files if re.search(r'(\d+)', f)]
    highest_number = max(numbers) if numbers else 0
    
    # Rename files from 0.bmp to 9.bmp
    for i in range(10):
        old_name = f"{i}.bmp"
        if old_name in bmp_files:
            new_name = f"{highest_number + 1 + i}.bmp"
            os.rename(os.path.join(directory, old_name), os.path.join(directory, new_name))
            print(f"Renamed {old_name} to {new_name}")

# Example usage
rename_bitmaps('.')
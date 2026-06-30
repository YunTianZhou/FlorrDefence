#!/usr/bin/env python3
"""
resize_png.py

Resize all 1024x1024 PNG images in the current directory to 256x256, preserving
sharp outlines (ideal for pixel art, text, and sprites). Store the resized images in a 'resized' subdirectory.
"""
import os
from PIL import Image

def resize_image(input_path: str, output_path: str) -> None:
    """Open an image, verify size, resize with NEAREST (crisp) filter, and save."""
    with Image.open(input_path) as im:
        if im.size != (1024, 1024):
            print(f"Skipping '{input_path}' – expected size 1024x1024, got {im.size}")
            return

        # Use NEAREST neighbor to preserve sharp edges and outlines
        resized = im.resize((512, 512), resample=Image.NEAREST)
        # Optionally, you can sharpen the result if needed:
        # from PIL import ImageFilter
        # resized = resized.filter(ImageFilter.SHARPEN)

        resized.save(output_path)
        print(f"Resized and saved: {output_path}")


def main():
    """Process all .png files in the current directory."""
    output_dir = '..'
    os.makedirs(output_dir, exist_ok=True)

    for filename in os.listdir('.'):
        if not filename.lower().endswith('.png'):
            continue

        input_path = filename
        output_path = os.path.join(output_dir, filename)
        resize_image(input_path, output_path)


if __name__ == '__main__':
    main()

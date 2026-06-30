from PIL import Image
import os

# Directory containing images (current directory)
directory = "."

# Process each PNG file in the directory
for filename in os.listdir(directory):
    if filename.lower().endswith(".png"):
        filepath = os.path.join(directory, filename)
        img = Image.open(filepath).convert("RGBA")
        datas = img.getdata()

        new_data = []
        for item in datas:
            # Keep white pixels (R=255, G=255, B=255) fully opaque
            if item[0] == 255 and item[1] == 255 and item[2] == 255:
                new_data.append((255, 255, 255, 255))
            else:
                # Make other pixels transparent
                new_data.append((255, 255, 255, 0))

        img.putdata(new_data)
        img.save(filepath)  # overwrite original
        print(f"Processed {filename}")

print("All images processed.")

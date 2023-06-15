from PIL import Image

def check_image(image_path):
    # Load the image
    image = Image.open(image_path)
    pixels = image.load()
    
    print(pixels[320, 420])

    # Iterate over each pixel
    width, height = image.size
    for x in range(width):
        for y in range(height):
            r, g, b = pixels[x, y]
            
            # Check if blue component is less than 0.5
            if b < 128:
                return False

    return True

# Provide the path to your image
image_path = './custom_scenes/steel-groupers/textures/Fish_Color_normal.png'

# Check the image
result = check_image(image_path)
if result:
    print("All pixels have b >= 128")
else:
    print("Some pixels have b < 128")

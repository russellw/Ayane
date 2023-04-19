import sys

from PIL import Image

width = 100
height = 100
pixels = [(255, 0, 0)] * (width * height)
image = Image.new("RGB", (width, height))
image.putdata(pixels)
image.save("a.png")

from PIL import Image
import numpy as np

ofs = open("bump_circles.c", mode = "w", encoding = "utf-8")
ofs.write("const unsigned char bump_circles[16384]  __attribute__((aligned(4))) = {\n")

img = Image.open("bump_circles2.png")
arr = np.array(img.getdata())

xsize, ysize = img.size
for j in range(0, ysize):
    for i in range(0, xsize):

        rgb = arr[j * ysize + i]
        height = rgb#int((rgb[0] + rgb[1] + rgb[2]) / 3)

        ofs.write('{}'.format(height))
        ofs.write(", ")

        if (i & 15) == 0:
            ofs.write("\n")            

print(arr.size)

ofs.write("};")
ofs.close()
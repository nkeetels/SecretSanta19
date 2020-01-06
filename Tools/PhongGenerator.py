# super fake Blinn-Phong approximation

import math

ofs = open("phong_lut.c", mode = "w", encoding = "utf-8")
ofs.write("const unsigned char phong_lut[16384]  __attribute__((aligned(4))) = {\n")

for j in range(0, 128):
    for i in range(0, 128):
        x = (i - 64) / 64.0
        y = (j - 64) / 64.0

        ilen = 1.0 - math.sqrt(x * x + y * y)

        if ilen < 0.0:
            ilen = 0.0

        ilen *= ilen

        intensity = 255 - int(ilen * 255.0)

        ofs.write('{}'.format(intensity))
        ofs.write(", ")

        if (i & 15) == 0:
            ofs.write("\n")


ofs.write("};")
ofs.close()
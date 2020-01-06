# PaletteShaderr generates shading tables for 32-color palettes
# input binary indexed image
# output palette array containing 8 shades

import struct

file_name = "tiling32"
file_name_pal = file_name + ".pal.bin"

rgb_colors = []

c_output_file = file_name + "_shaded.c"

with open(file_name_pal, mode="rb") as readFile:
    index = 0
    while index < 32:
        # for some reason even though big endianess is forced the bytes are read in the wrong order, so what them manually
        decoded = struct.unpack('>H', readFile.read(2))[0]
        swapped = struct.unpack("<H", struct.pack(">H", decoded))[0]
        #swapped = decoded
        # unpack R, G and B from 15-bit color space
        b = float((swapped & 0x7C00) >> 10)
        g = float((swapped & 0x3E0) >> 5)
        r = float(swapped & 0x1F)

        rgb = (r, g, b)
        rgb_colors.append(rgb)

        index = index + 1


print(len(rgb_colors))

for pal in range(1, 8):
    t = 7 / pal
    for i in range (0, 32):
        rgb = rgb_colors[i]

        r = rgb[0]
        g = rgb[1]
        b = rgb[2]

        r = r / pal
        g = g / pal
        b = b / pal

        new_rgb = (r, g, b)

        rgb_colors.append(new_rgb)

print(len(rgb_colors))

ofs = open(c_output_file, mode="w", encoding="utf-8")
ofs.write("const unsigned short " + file_name + "_palette[256] __attribute__((aligned(4)))= {\n")

for i in range(0, 256):
    rgb = rgb_colors[i]

    # convert to RGB555 range
    r = int(rgb[0])
    g = int(rgb[1])
    b = int(rgb[2])

    # pack into an unsigned 15-bit color value
    color16 = (r  | (g << 5) | (b << 10)) & 0xFFFF
    # color16 = raw_palette[i] # TEST
    ofs.write('0x{:>X}'.format(color16))
    ofs.write(", ")

    if (i & 15) == 0:
        ofs.write("\n")

ofs.write("};")
ofs.close()


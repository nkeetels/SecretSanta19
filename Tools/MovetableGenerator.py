# generates a 320x256 movetable with interleaved UV coordinates encoded in 16 bit unsigned shorts (little endian).

import math

# the table is a bit larger than a GBA screen to be able to move the camera around
XRES = 320
YRES = 256

#effect = "tunnel"
effect = "flower"

ofs = open("lut_effect.c", mode = "w", encoding = "utf-8")
ofs.write("const unsigned short lut_effect[] = {\n")

for j in range(0, YRES):

    ydist = j - (YRES / 2)

    for i in range(0, XRES):

        xdist = i - (XRES / 2)

        # fixed point
        distance = int(math.sqrt(xdist * xdist + ydist * ydist))
        angle = int(math.atan2(float(xdist), float(ydist)) * 64)

        if distance <= 0:
            distance = 1

        if effect == "tunnel":
            u = angle
            v = int(32 * 256 / distance) & 0xFF

        if effect == "flower":
            u = angle
            v = distance + int(math.sin(math.atan2(float(xdist), float(ydist)) * 5) * 90)

        if effect == "swirl":
            u = (2 * angle / math.pi) + distance
            v = int(math.pow(float(distance), 1.2))

        # GBA supports little endian byte-order
        short = ((int(u) << 8) + v) & 0xFFFF
        ofs.write('{}'.format(short))
        ofs.write(", ")

        if (i & 15) == 0:
            ofs.write("\n")

ofs.write("};")
ofs.close()
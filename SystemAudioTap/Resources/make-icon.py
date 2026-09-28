# Draws CabinEQ System's app icon: CabinEQ's blue tile and cabin, with the cabin sitting on an
# EQ curve that runs the full width, since this app EQs everything.
#   python3 make-icon.py   ->   CabinEQSystemIcon.png (1024 x 1024)
import math, os
from PIL import Image, ImageDraw

S = 4                      # draw big, then scale down, for smooth edges
N = 1024 * S
TOP, BOTTOM = (13, 203, 255), (6, 127, 255)
INK = (18, 35, 45, 255)

def px(v): return int(round(v * S))

# The tile: the same size, corner radius and gradient as CabinEQ's icon
gradient = Image.new("RGBA", (N, N))
g = ImageDraw.Draw(gradient)
for y in range(N):
    t = min(1.0, max(0.0, (y / S - 102) / (921 - 102)))
    g.line([(0, y), (N, y)], fill=tuple(int(a + (b - a) * t) for a, b in zip(TOP, BOTTOM)) + (255,))
mask = Image.new("L", (N, N), 0)
ImageDraw.Draw(mask).rounded_rectangle([px(102), px(102), px(921), px(921)], radius=px(200), fill=255)
icon = Image.new("RGBA", (N, N), (0, 0, 0, 0))
icon.paste(gradient, (0, 0), mask)
d = ImageDraw.Draw(icon)

# The EQ curve: a rise under the cabin, a dip, and a lift at the top end
def curve_y(x):
    return (705
            - 105 * math.exp(-((x - 512) / 125) ** 2)  # the hill the cabin stands on
            + 45 * math.exp(-((x - 745) / 60) ** 2)    # a cut
            + 25 * math.exp(-((x - 270) / 70) ** 2))   # a gentle dip in the bass
points = [(px(x), px(curve_y(x))) for x in range(195, 830, 2)]
width = px(46)
d.line(points, fill=INK, width=width, joint="curve")
for end in (points[0], points[-1]):
    d.ellipse([end[0] - width / 2, end[1] - width / 2, end[0] + width / 2, end[1] + width / 2], fill=INK)

# The cabin, standing on the top of the hill
cx, base, half, wall, roof = 512, curve_y(512) - 42, 145, 145, 122
d.polygon([(px(cx - half), px(base)), (px(cx - half), px(base - wall)), (px(cx), px(base - wall - roof)),
           (px(cx + half), px(base - wall)), (px(cx + half), px(base))], fill=INK)
door_w, door_h = 62, 96
d.rectangle([px(cx - door_w / 2), px(base - door_h), px(cx + door_w / 2), px(base)], fill=(0, 0, 0, 0))
# The door has to show the tile behind it, so cut it back out of the tile
cut = Image.new("L", (N, N), 0)
ImageDraw.Draw(cut).rectangle([px(cx - door_w / 2), px(base - door_h), px(cx + door_w / 2), px(base) + 2], fill=255)
icon.paste(gradient, (0, 0), cut)

# Two band handles on the curve: a ring of tile colour between an ink dot and the line
for hx in (270, 745):
    hy = curve_y(hx)
    r_ring, r_dot = 50, 34
    ring = Image.new("L", (N, N), 0)
    ImageDraw.Draw(ring).ellipse([px(hx - r_ring), px(hy - r_ring), px(hx + r_ring), px(hy + r_ring)], fill=255)
    icon.paste(gradient, (0, 0), ring)
    d.ellipse([px(hx - r_dot), px(hy - r_dot), px(hx + r_dot), px(hy + r_dot)], fill=INK)

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "CabinEQSystemIcon.png")
icon.resize((1024, 1024), Image.LANCZOS).save(out)
print(out)

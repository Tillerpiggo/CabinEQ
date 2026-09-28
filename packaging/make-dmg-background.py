# Draws the DMG window's background: an arrow from the app to Applications, and a line of help.
# Finder draws the icon labels in black, so it's light. Makes a 1x and a 2x image, which
# package.sh combines into one TIFF so it's sharp on Retina screens.
#   python3 make-dmg-background.py <out folder>
import os, sys
from PIL import Image, ImageDraw, ImageFont

WIDTH, HEIGHT = 660, 420
APP_X, APPS_X, ICON_Y = 180, 480, 190 # where dmg-settings.py puts the icons (their centres)

def font(size, bold=False):
    for path in ["/System/Library/Fonts/SFNS.ttf", "/System/Library/Fonts/Helvetica.ttc"]:
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except OSError:
                pass
    return ImageFont.load_default()

def draw(scale):
    w, h = WIDTH * scale, HEIGHT * scale
    image = Image.new("RGB", (w, h))
    d = ImageDraw.Draw(image)

    # A soft top-to-bottom wash
    top, bottom = (250, 250, 252), (236, 237, 242)
    for y in range(h):
        t = y / (h - 1)
        d.line([(0, y), (w, y)], fill=tuple(int(a + (b - a) * t) for a, b in zip(top, bottom)))

    # The arrow, between the two icons
    violet = (125, 108, 240)
    y = ICON_Y * scale
    start, end = (APP_X + 88) * scale, (APPS_X - 88) * scale
    thickness = 5 * scale
    d.line([(start, y), (end - 14 * scale, y)], fill=violet, width=thickness)
    head = 16 * scale
    d.polygon([(end, y), (end - head, y - head * 0.7), (end - head, y + head * 0.7)], fill=violet)

    # What to do
    title = "Drag CabinEQ System to Applications"
    subtitle = "Then open it from your Applications folder."
    for text, size, colour, offset in [(title, 17, (40, 42, 48), 312), (subtitle, 13, (110, 114, 124), 340)]:
        f = font(size * scale)
        width = d.textlength(text, font=f)
        d.text(((w - width) / 2, offset * scale), text, font=f, fill=colour)
    return image

out = sys.argv[1] if len(sys.argv) > 1 else "."
draw(1).save(os.path.join(out, "background.png"), dpi=(72, 72))
draw(2).save(os.path.join(out, "background@2x.png"), dpi=(144, 144))

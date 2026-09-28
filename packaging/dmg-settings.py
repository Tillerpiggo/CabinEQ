# dmgbuild's settings for the CabinEQ System disk image. package.sh passes in
# -D app=<path to the app> -D background=<the background TIFF> -D icon=<the volume icon>.
import os

app = defines["app"]
application = os.path.basename(app)

format = "UDZO"
filesystem = "HFS+"
files = [app]
symlinks = {"Applications": "/Applications"}
icon = defines.get("icon")
background = defines["background"]

# Matches make-dmg-background.py
window_rect = ((200, 120), (660, 420))
icon_size = 128
text_size = 13
icon_locations = {application: (180, 190), "Applications": (480, 190)}
default_view = "icon-view"
show_status_bar = False
show_tab_view = False
show_toolbar = False
show_pathbar = False
show_sidebar = False
arrange_by = None

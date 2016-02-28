# Modules #

## Paint

Here is a short, partial and terse description of the Paint module.
Practice first by loading a demo with images in it (e.g. vintage)
and selecting an image from the list.

You can move the red square and then press fire to change the pixels of the
image one by one. On some old phones, moving the cursor can be really slow.
If this is the case, selecting the "(No)Coords" item from the menu should
help.

There are two major modes: image and palette. You can switch between the two
by pressing 0.

There are two sub modes for the image mode: pixel and tile. Tile mode is only
available if the the dimensions of the image are multiple of 8. You can
switch between the two by pressing #.

Image/pixel mode: 2, 4, 6, 8: move the cursor, 5: change the current pixel,
1: cycle foreground color, 3: cycle background color, 7: pick foreground
color, 9: pick background color.

Image/tile mode: 2, 4, 6, 8: move the cursor, 1: cycle foreground color, 3:
cycle background color, 7: copy tile, 5: paste/clear tile. Be careful when
you use tile mode, as no undo is available and it is easy to clear entire
tiles.

You can test how the program looks with the edited images, by pressing *.
When the program ends, control is returned back to Paint.

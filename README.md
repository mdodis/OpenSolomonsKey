# OpenSolomonsKey
Linux and Windows Solomon's Key (1986) port

### Wikipedia Entry:
Solomon's Key (ソロモンの鍵 Soromon no Kagi) is a puzzle game developed by Tecmo in 1986 for arcade release on custom hardware based on the Z80 chipset. It is better known as a 1987 port to the Commodore 64 and the Nintendo Entertainment System, although it also appeared on many other game systems of the time, like the Sega Master System in 1988 and the Famicom Disk System, released in Japan on January 25, 1991.

## TODO:
* Time management on Win32 platform
* Basic Sprites (draw the border stuff that is always there and doesn't change!)
* Input handling
* Time measurement
* Tilemap
* Tilemap / Sprite collision
* Level storage

## Decisions:

### Resolution Independence
Resolution independence can be achieved in two ways, how I see it. You can (1) Render to a texture of a specific resolution and then upscale or
downscale according to the window's dimensions, or (2) use a basic for calculating a specific metric (like a pixel's size)

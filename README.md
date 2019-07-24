# OpenSolomonsKey
Linux and Windows Solomon's Key (1986) port

### Wikipedia Entry:
Solomon's Key (ソロモンの鍵 Soromon no Kagi) is a puzzle game developed by Tecmo in 1986 for arcade release on custom hardware based on the Z80 chipset. It is better known as a 1987 port to the Commodore 64 and the Nintendo Entertainment System, although it also appeared on many other game systems of the time, like the Sega Master System in 1988 and the Famicom Disk System, released in Japan on January 25, 1991.

## TODO:
- [ ] Use actual game assets
- [ ] Sprite animation
- [X] Tilemap / Sprite collision
    - [X] bounding box drawing (for debugging)
    - [X] minkowski aabb (dont forget the penetration vector)
- [X] Level storage
- [X] Removed GLEW
- [X] Time measurement
- [X] Input handling
- [X] Tilemap
- [X] Time management on Win32 platform

## Decisions:

### Resolution Independence
    Resolution independence can be achieved in two ways, how I see it. You can (1) Render to a texture of a specific resolution and then upscale or
    downscale according to the window's dimensions, or (2) use a basic for calculating a specific metric (like a pixel's size).

Adding to that, in order to not have our calculations messed up because of weird scaling issues, we'll do all calculations with a tile size of 64, and scale everything back accordingly. So if the current resolution dependent tile size would be, say 55, then we would scale everything by 55 / 64 = 0.859375.


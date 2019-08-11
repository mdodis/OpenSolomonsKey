# OpenSolomonsKey
Linux and Windows Solomon's Key (1986) port.

### About (Wikipedia):
Solomon's Key (ソロモンの鍵 Soromon no Kagi) is a puzzle game developed by Tecmo in 1986 for arcade release on custom hardware based on the Z80 chipset. It is better known as a 1987 port to the Commodore 64 and the Nintendo Entertainment System, although it also appeared on many other game systems of the time, like the Sega Master System in 1988 and the Famicom Disk System, released in Japan on January 25, 1991.

This is a port of the arcade version of the game, currently on linux and
Windows machines. It's going to include a level file format so you can
make your own levels.

If you are on Windows you can check out a completed port on
[GMS2 by immortalx](https://immortalx74.itch.io/solomonskeyremake).

## TODO:

- [X] Sprite animation
- [X] Use actual game assets
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

### Collision
Right now (Wed 24 Jul 2019), collision is a simple minkowski diff (aabb)
with the penetration vector, maybe if a problem should arise in the
future, I'll add some more checks (swept collision).

## Notes:

* Jump: 2 + 1/3 blocks, goes inside the last block, but without colliding
* Cast on air: retains position

# PlasmaPong Refactor

My attempt to clean up an ancient DOS game a friend and I made in high school.

## Goals

- Rename, rearrange, reformat, and refactor things to be more sane, but still one file
- [X] Get it running under FreeDOS with OpenWatcom compiler
- Fix game-breaking problems
    - [ ] Score display
    - [ ] Address compiler warnings
    - [ ] Crash in non-huge memory model
- Fix QoL problems
    - [ ] Restore video mode upon exit


## Building

Currently using [OpenWactom 1.9](http://openwatcom.org/ftp/install/). Will likely switch to [version 2](https://github.com/open-watcom/open-watcom-v2) soon.

```
> wcl -mh pp.cpp
```

Alternatively use `-ml` or `-mc`. See [known issues](#problems-with-latest-builds-on-freedos)

TODO:

- [ ] makefile

## Running the game


I am using FreeDOS on VMWare with the project directory mounted as a shared folder using vmsmount. After building the program on the host, I can run `pp.exe` inside the VM.

To exit, click both left and right mouse buttons simultaneously.


## Known issues

### Bugs from ancient times

- Score is not displaying correctly. A bunch of glitchy lines appear on the right side of the screen instead.
- Stays in 320x200x256 mode after exiting.

### Problems with latest builds on FreeDOS

- With large/compact memory models, the game usually crashes when the ball leaves the play area. When it doesn't crash, a growing amount of glitchiness starts occurring
- With huge memory model the blurring only affects the top half of the screen
- The game runs significantly slower with huge memory model

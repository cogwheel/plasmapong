# PlasmaPong Refactor

My attempt to clean up an ancient DOS game a friend and I made in high school.

## Goals

- Rename, rearrange, reformat, and refactor things to be more sane, but still one file
    - [ ] Magic numbers -> constants
    - [ ] Deduplicate code
- [X] Get it running under FreeDOS with OpenWatcom compiler
- Fix game-breaking problems
    - [X] Score display
    - [X] Address compiler warnings
    - [X] Crash in non-huge memory model
- Fix QoL problems
    - [X] Restore video mode upon exit


## Building

Currently using [OpenWactom 1.9](http://openwatcom.org/ftp/install/). Will likely switch to [version 2](https://github.com/open-watcom/open-watcom-v2) soon.

```
> wcl -ml -wx -we pp.cpp
```

TODO:

- [ ] makefile

## Running the game


I am using FreeDOS on VMWare with the project directory mounted as a shared folder using vmsmount. After building the program on the host, I can run `pp.exe` inside the VM.

To exit, click both left and right mouse buttons simultaneously.


## Known issues

* Blur effect isn't centered on the screen
* Score is not very legible, especially during countdown
* Ball speed calculation is wonky; there is both a `speed` and `ball_[xy]_delta`

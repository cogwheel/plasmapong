# PlasmaPong Refactor

My attempt to clean up an ancient DOS game a friend and I made in high school.

## Goals

- Rename, rearrange, reformat, and refactor things to be more sane, but still one file
    - [X] Magic numbers -> constants
    - [X] Deduplicate code
    - [X] Globals -> state object
    - [X] Rearrange code to eliminate prototypes
    - [X] Consolidate constants
    - [ ] Defines -> static consts? (not sure if watcom can elide them)
- [ ] Test performance on (simulated) pentium 233 to make sure it still runs well (it's probably fine since I eliminated an entire screen copy)
- [X] Get it running under FreeDOS with OpenWatcom compiler
- Fix game-breaking problems
    - [X] Score display
    - [X] Address compiler warnings
    - [X] Crash in non-huge memory model
- Fix QoL problems
    - [X] Restore video mode upon exit


## Building

Currently using [OpenWactom 2](https://github.com/open-watcom/open-watcom-v2).

For an optimized release build:

```
> wcl -q -mc -wx -we -ox -5 -fp5 -fpi87 -DNDEBUG pp.cpp
```

This game was originally developed on a Pentium MMX 233, hence the `-5 -fp5 -fpi87` options.

For debug builds, add one of the [debug symbol options](https://open-watcom.github.io/open-watcom-v2-wikidocs/cguide.html#DebuggingDProfiling), and remove one or both of `-ox` and `-DNDEBUG`.


TODO:

- [ ] makefile

## Running the game


I am using FreeDOS on VMWare with the project directory mounted as a shared folder using vmsmount. After building the program on the host, I can run `pp.exe` inside the VM.

To exit, click both left and right mouse buttons simultaneously.


## Known issues

* Score is not very legible, especially during countdown
* Ball speed calculation is wonky; there is both a `speed` and `ball_[xy]_delta`

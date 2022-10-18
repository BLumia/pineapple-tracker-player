## Summary

Pineapple Tracker Player is a [modular music](https://en.wikipedia.org/wiki/Modular_music) player.

## Build it manually:

Current state, we need:

- `cmake`: as the build system.
- `qt6` with (currently optional) `Quick` component: since the app is using Qt.
- `libopenmpt`: the modular music playback core library.
- `portaudio`: for audio output.

Building it just requires normal cmake building steps:

``` shell
$ mkdir build && cd build
$ cmake ..
$ cmake --build . # or simply using `make` if you are using Makefile as the cmake generator.
```

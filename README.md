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

## CLA

```
By sending patches in GitHub Pull Request, Issues, email patch 
set, or any other form to this project, it is assumed that you
are offering the Pineapple Tracker Player project and the original
project author (Gary Wang) unlimited, non-exclusive right to
reuse, modify, and relicense the code.
```

This is important because the inability to relicense code has caused devastating problems for other Free Software projects (such as KDE and NASM). Pineapple Tracker Player will always be available in an OSI approved, DFSG-compatible license. If you wish to specify special license conditions of your contributions, just say so when you send them.

## License

The source code of this project is licensed under [**GNU General Public License v3.0 only**](https://spdx.org/licenses/GPL-3.0-only.html) license. Individual files may have a different, but compatible license.

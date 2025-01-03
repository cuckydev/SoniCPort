# NOTICE

This project is being supersceded by [Sonic1C](https://github.com/cuckydev/Sonic1C).

# SoniCPort

Sonic the Hedgehog (1991, Sega Genesis / MegaDrive) C Port.

## Dependencies

* SDL2 (if `BACKEND` is set to `SDL2`)
* pkg-config (for builds that require static-linkage)

## Building

This project uses CMake, allowing it to be built with a range of compilers.

Switch to the terminal and `cd` into this folder.

After that, generate the files for your build system with:
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

MSYS2 users should append `-G"MSYS Makefiles" -DPKG_CONFIG_STATIC_LIBS=ON` to this command, also.

You can also add the following flags:

Name | Function
--------|--------
`-DBACKEND=SDL2` | Use the SDL2 backend (default)
`-DREV01=ON` | Compile a REV01 ROM
`-DJAPANESE=ON` | Compile a Japanese ROM
`-DFIX_BUGS=ON` | Fix bugs that are blatant screw-ups that may harm performance (not gameplay bugs)
`-DLTO=ON` | Enable link-time optimisation
`-DMSVC_LINK_STATIC_RUNTIME=ON` | Link the static MSVC runtime library, to reduce the number of required DLL files (Visual Studio only)

You can pass your own compiler flags with `-DCMAKE_C_FLAGS` and `-DCMAKE_CXX_FLAGS`.

You can then compile the executable with this command:

```
cmake --build build --config Release
```

## Disclaimer

This project is not endorsed by SEGA or Sonic Team.

## Credits

Sonic Team - Original game

Sonic Retro - Sonic 1 Github Disassembly

Clownacy - Additional porting (compression algorithms)

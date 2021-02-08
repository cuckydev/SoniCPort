# SoniCPort

Sonic the Hedgehog (1991, Sega Genesis / MegaDrive) C Port

## Dependencies

* SDL2 (if `COMPILE_SDL2` is set to ON)
* pkg-config (for builds that require static-linkage)
* libvgm (sub-module)

## Building

This project uses CMake, allowing it to be built with a range of compilers.

Switch to the terminal and `cd` into this folder.

Before doing anything else, you must download the libvgm sub-module.

Install the sub-module with:
```
git submodule update --init
```

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
`-DPKG_CONFIG_STATIC_LIBS=ON` | On platforms with pkg-config, static-link the dependencies (good for Windows builds, so you don't need to bundle DLL files)
`-DMSVC_LINK_STATIC_RUNTIME=ON` | Link the static MSVC runtime library, to reduce the number of required DLL files (Visual Studio only)

You can pass your own compiler flags with `-DCMAKE_C_FLAGS` and `-DCMAKE_CXX_FLAGS`.

You can then compile the executable with this command:

```
cmake --build build --config Release
```

## Disclaimer

This project is not endorsed by SEGA or Sonic Team.

## Credits

Sonic Retro - Sonic 1 Disassembly

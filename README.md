shot
====

Make screenshots with friendly CLI.

### Features

- Cross platform:
    - Windows
    - GNU/Linux (X11 + XRandR)
- Versatile region selection methods:
    - the whole desktop
    - currently focused window
    - specific monitor
    - specific rectangle (passed as string)
    - interactive selection with a special window

---

### Compiling for GNU/Linux

1. Install libpng.
2. Run following:

        ./bootstrap
        ./waf configure
        ./waf

3. If you wish to install it globally:

        sudo ./waf install

### Cross compiling for Windows

1. Install [`mxe`](https://github.com/mxe/mxe) and compile libpng:

        git clone https://github.com/mxe/mxe.git
        cd mxe
        make libpng

2. Configure the shell to use `mxe`:

        MXE_PATH=~/src/mxe/
        CROSS=i686-w64-mingw32.static-
        export PATH="$MXE_PATH/usr/bin/:$PATH"
        export CC=${CROSS}gcc
        export AR=${CROSS}ar
        export PKGCONFIG=${CROSS}pkg-config

3. Compile the project the regular way.

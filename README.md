screenshoter
============

Make screenshots with friendly CLI.

**Work in progress.**

### Features

1. Windows support
2. Linux (X11) support

---

### Compiling for GNU/Linux

1. Install libpng.
2. Run following:

        ./bootstrap
        ./waf configure
        ./waf

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

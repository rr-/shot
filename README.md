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
    - currently focused monitor (established by focused window)
    - specific monitor
    - specific window
    - specific rectangle (passed as string)
    - interactive selection with a special window

### Why not scrot?

To have consistent user experience between platforms I use, I wanted it
to work on Windows as well. Additionally, I wanted manual region selection to
be more precise - in scrot, once you select a rectangle, that's it, you can't
correct it. shot's region picker makes it easy to correct offsets even by 1px
before actually taking the screenshot and provides visual feedback of the
screen area.

### Interactive selection in action

![--interactive at its
best](https://cloud.githubusercontent.com/assets/1045476/8808860/5908945e-2fe5-11e5-93bf-ecad1500c35b.png)

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

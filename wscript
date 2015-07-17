# vi: ft=python
from waflib import Logs
import os

APPNAME = 'shot'
try:
    VERSION = os.popen('git describe --tags').read().strip()
    VERSION_LONG = os.popen('git describe --always --dirty --long --tags').read().strip()
except:
    VERSION = '0.0'
    VERSION_LONG = '?'

def options(ctx):
    ctx.load('compiler_c')

    ctx.add_option(
        '-d',
        '--debug',
        dest = 'debug',
        default = False,
        action = 'store_true',
        help = 'enable emitting debug information')

def configure_flags(ctx):
    ctx.load('compiler_c')

    ctx.env.CFLAGS = [
        '-std=c99',
        '-Wall',
        '-Wextra',
        '-pedantic-errors']

    if ctx.options.debug:
        ctx.env.CFLAGS += ['-g']
        Logs.info('Debug information enabled')
    else:
        Logs.info('Debug information disabled, pass -d to enable')

def configure_packages(ctx):
    ctx.check_cfg(
        package = 'libpng',
        args = '--cflags --libs',
        uselib_store = 'LIBPNG',
        mandatory = True)

    ctx.check_cfg(
        package = 'x11',
        args = '--cflags --libs',
        uselib_store = 'LIBX11',
        mandatory = False)

    ctx.check_cfg(
        package = 'xrandr',
        args = '--cflags --libs',
        uselib_store = 'LIBXRANDR',
        mandatory = False)

    ctx.check(
        lib = 'gdi32',
        args = '--cflags --libs',
        uselib_store = 'LIBGDI32',
        define_name = 'HAVE_GDI32',
        mandatory = False)

def configure(ctx):
    configure_flags(ctx)
    configure_packages(ctx)

def build(ctx):
    ctx.env.DEFINES += [ 'SHOT_VERSION="' + VERSION_LONG + '"' ]

    #work around waf inconsistencies (#1600)
    for define in [d for d in ctx.env.DEFINES if d.startswith('HAVE_')]:
        key, value = define.split('=')
        ctx.env[key] = int(value)

    all_sources = ctx.path.ant_glob('src/**/*.c')
    x11_sources = ctx.path.ant_glob('src/**/*x11.c')
    win_sources = ctx.path.ant_glob('src/**/*win.c')
    sources = list(set(all_sources) - set(x11_sources) - set(win_sources))

    path_to_src = ctx.path.find_node('src').abspath()

    if ctx.env.HAVE_GDI32:
        ctx.objects(
            source = win_sources,
            target = 'shot_win',
            use = [ 'LIBGDI32' ])
    elif ctx.env.HAVE_LIBX11:
        ctx.objects(
            source = x11_sources,
            target = 'shot_x11',
            use = [ 'LIBX11', 'LIBXRANDR' ])

    ctx.program(
        source = sources,
        target = 'shot',
        use = [ 'shot_x11', 'shot_win', 'LIBPNG' ])

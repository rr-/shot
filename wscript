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
    ctx.env.CFLAGS = [
        '-Wall',
        '-Wextra',
        '-pedantic']

    if ctx.options.debug:
        ctx.env.CFLAGS += ['-g']
        Logs.info('Debug information enabled')
    else:
        Logs.info('Debug information disabled, pass -d to enable')

    ctx.load('compiler_c')

def configure_packages(ctx):
    ctx.check_cc(
        lib = ['png'],
        header_name = 'png.h',
        uselib_store = 'LIBPNG',
        mandatory = True)

    ctx.check_cc(
        lib = ['X11'],
        header_name = 'X11/Xlib.h',
        uselib_store = 'LIBX11',
        mandatory = True)

def configure(ctx):
    configure_flags(ctx)
    configure_packages(ctx)

def build(ctx):
    ctx.env.DEFINES = [ 'SHOT_VERSION="' + VERSION_LONG + '"' ]

    sources = ctx.path.ant_glob('src/**/*.c')
    path_to_src = ctx.path.find_node('src').abspath()

    ctx.program(
        source = sources,
        target = 'shot',
        cflags = ['-iquote', path_to_src],
        use = [ 'LIBPNG', 'LIBX11' ])

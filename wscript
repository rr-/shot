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
        ctx.env.CFLAGS += ['-ggdb']
        Logs.info('Debug information enabled')
    else:
        Logs.info('Debug information disabled, pass -d to enable')

def configure_packages(ctx):
    ctx.check_cfg(
        package = 'libpng',
        args = '--cflags --libs',
        uselib_store = 'LIBPNG',
        global_define = True,
        mandatory = True)

    ctx.check_cfg(
        package = 'x11',
        args = '--cflags --libs',
        uselib_store = 'LIBX11',
        global_define = True,
        mandatory = False)

    ctx.check_cfg(
        package = 'xrandr',
        args = '--cflags --libs',
        uselib_store = 'LIBXRANDR',
        global_define = True,
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

def chunks(l, n):
    for i in range(0, len(l), n):
        yield l[i:i+n]

def make_help_h(path_to_src):
    help_path = os.path.join(path_to_src, 'help.txt')
    header_path = os.path.join(path_to_src, 'help.h')
    with open(help_path, 'r') as ifh:
        with open(header_path, 'w') as ofh:
            ofh.write('#ifndef HELP_H\n')
            ofh.write('#define HELP_H\n\n')
            ofh.write('const char help_str[] = {\n')
            bytes = list(ifh.read())
            for chunk in chunks(bytes, 16):
                ofh.write('    "')
                for b in chunk:
                    ofh.write('\\x%02x' % ord(b))
                ofh.write('"\n')
            ofh.write('};\n\n')
            ofh.write('#endif\n')

def build(ctx):
    path_to_src = ctx.path.find_node('src').abspath()

    make_help_h(path_to_src)

    ctx.define('SHOT_VERSION', VERSION_LONG)
    ctx.define('_POSIX_C_SOURCE', '200809L', False)

    ctx.env.CFLAGS += ['-iquote', path_to_src]
    if ctx.is_defined('HAVE_GDI32'):
        ctx.env.LINKFLAGS += ['-mwindows']

    all_sources = ctx.path.ant_glob('src/**/*.c')
    x11_sources = ctx.path.ant_glob('src/**/*x11.c')
    win_sources = ctx.path.ant_glob('src/**/*win.c')
    sources = list(set(all_sources) - set(x11_sources) - set(win_sources))

    if ctx.is_defined('HAVE_GDI32'):
        ctx.objects(
            source = win_sources,
            target = 'shot_win',
            use = [ 'LIBGDI32' ])
    elif ctx.is_defined('HAVE_X11'):
        ctx.objects(
            source = x11_sources,
            target = 'shot_x11',
            use = [ 'LIBX11', 'LIBXRANDR' ])

    ctx.program(
        source = sources,
        target = 'shot',
        use = [ 'shot_x11', 'shot_win', 'LIBPNG' ])

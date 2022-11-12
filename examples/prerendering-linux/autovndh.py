#!/usr/bin/env python3

from __future__ import print_function

import argparse
import itertools
import os, os.path
import shutil
import subprocess
import sys
import traceback

import multiprocessing
import multiprocessing.dummy as multi

verbose = 0

def is_exec(p):
    return (p is not None) and os.path.isfile(p) and os.access(p, os.X_OK)

def eprint(*args, **kwargs): print(*args, file=sys.stderr, **kwargs)

def run_proc_get_stdout(cmdlin):
    try:
        if verbose > 1: eprint(cmdlin)
        proc = subprocess.Popen(cmdlin, stdout=subprocess.PIPE)
        output = proc.stdout.read()
        proc.wait()
        if proc.returncode != 0:
            if verbose > 1: eprint("-> " + str(proc.returncode))
            elif verobse > 0:
                eprint(str(cmdlin)+" -> " + str(proc.returncode))
        return None if proc.returncode != 0 else output
    except KeyboardInterrupt:
        exit(1)
    except Exception as e:
        return None


def run_one(binf, opt, input_file):
    return run_proc_get_stdout([binf]+opt+[input_file])

def run_compr(compr, mpool,inp):
    binf, opt, isgz = compr

    return list(filter(lambda x: x[0] is not None,
                       mpool.map(lambda o: (run_one(binf,o,inp), isgz), opt)))

def do_vndh(bin_nasm,vndh_dir,brute_result,vtag,vfork,vunibin,vcheat,vnoargv):
    blob, isgz = brute_result
    vndh_src = os.path.join(vndh_dir, "vondehi.asm")
    if not os.path.isfile(vndh_src):
        eprint("vondehi src file doesn't exist.")
        return None

    cmdlin = [bin_nasm,"-fbin","-o/dev/stdout"]
    if isgz: cmdlin.append("-DUSE_GZIP")
    if vtag is not None: cmdlin.append('-DTAG='+vtag)
    if vfork: cmdlin.append('-DUSE_VFORK')
    if vunibin: cmdlin.append('-DNO_UBUNTU_COMPAT')
    if not vcheat: cmdlin.append('-DNO_CHEATING')
    if not vnoargv: cmdlin.append('-DWANT_ARGV')
    cmdlin.append(vndh_src)

    vbl = run_proc_get_stdout(cmdlin)
    return None if vbl is None else (vbl + blob)


opt_xz_lzma = [
    [ 'dict=8M' ],
    [ 'lc=0,lp=0', 'lc=0,lp=1', 'lc=0,lp=2', 'lc=0,lp=3', 'lc=0,lp=4',
      'lc=1,lp=0', 'lc=1,lp=1', 'lc=1,lp=2', 'lc=1,lp=3',
      'lc=2,lp=0', 'lc=2,lp=1', 'lc=2,lp=2',
      'lc=3,lp=0', 'lc=3,lp=1',
      'lc=4,lp=0' ],
    [ 'pb=0', 'pb=1', 'pb=2', 'pb=3', 'pb=4' ],
    [ 'mf=bt4', 'mf=hc4' ],
    [ 'mode=normal' ],
    [ 'nice=273' ],
    [ 'depth=1000' ]
]


opt_gzip = [["-cnk9"]]
opt_xz   = list(map(lambda x: ['--stdout','--keep']+sum(x,[]),itertools.product(\
    [[],['--x86'],['--delta'],['--x86','--delta'],['--delta','--x86']],\
    map(lambda opts: ['--lzma2='+','.join(opts)], itertools.product(*opt_xz_lzma))
)))
opt_lzma = list(map(lambda opts: ['--stdout','--keep','--lzma1='+','.join(opts)], \
    itertools.product(*opt_xz_lzma)))
opt_zopfli = [['--gzip','--i1024','-c']]

def main(opts):
    global verbose
    if opts.verbose is not None:
        verbose = opts.verbose

    if verbose>1:opt_zopfli[0].append('-v')

    if not is_exec(opts.nasm):
        eprint("nasm binary not found, doesn't exist or is not executable.")
        return

    if not is_exec(opts.gzip  ): opts.gzip   = None
    if not is_exec(opts.xz    ): opts.xz     = None
    if not is_exec(opts.lzma  ): opts.lzma   = None
    if not is_exec(opts.zopfli): opts.zopfli = None

    comprs = sum([ \
        [(opts.gzip  , opt_gzip  , True )] if opts.gzip   is not None else [],
        [(opts.xz    , opt_xz    , False)] if opts.xz     is not None else [],
        [(opts.lzma  , opt_lzma  , False)] if opts.lzma   is not None else [],
        [(opts.zopfli, opt_zopfli, True )] if opts.zopfli is not None else [] \
    ], [])

    if len(comprs) == 0:
        eprint("No compression methods specified.")
        return

    mpool = multi.Pool(processes=opts.jobs)

    allofthem = list(sum(map(lambda x: run_compr(x, mpool, opts.input_file),
                             comprs),[]))

    if len(allofthem) == 0:
        eprint("No useable results available. See error log.")
        return

    allofthem_s = sorted(allofthem, key=lambda x:len(x[0]))

    best = allofthem_s[0]

    if verbose > 0: eprint(len(best[0]), "gzip" if best[1] else "xz")

    res, stubbed = best[0], None
    if not opts.nostub:
        stubbed = do_vndh(opts.nasm, opts.vndh, best,
                      opts.vndh_tag, opts.vndh_vfork, opts.vndh_unibin,
                      opts.vndh_cheat,opts.vndh_no_argv)

    if verbose > 0: eprint("final: "+str(len(res)))

    opts.output_file.write(stubbed or res)
    if stubbed is not None and opts.rawout is not None:
        opts.rawout.write(res)

if __name__=='__main__':
    p = argparse.ArgumentParser(description="""\
Computes the smallest possible compressed size of a binary through
brute-forcing, and then automatically vondehi-packs the result using the
correct flags.
""")

    p.add_argument('input_file', type=str, metavar='input',
                   help='The executable file to compress')
    p.add_argument('output_file', type=argparse.FileType('wb'), metavar='output',
                   default=sys.stdout.buffer, nargs='?',
                   help='Output file (defaults to stdout)')

    p.add_argument('--nostub', '-n', action='store_true',
                   help="Don't prepend a vondehi stub, only compress the input data.")

    p.add_argument('--verbose', '-v', action='count',
                   help='Increase verbosity level (0..2)')
    p.add_argument('--gzip', '-g', nargs='?', const=shutil.which('gzip'),
                   default=None, type=str, help='Enable gzip compression, and'+
                   ' optionally specify which gzip binary to use.')
    p.add_argument('--xz', '-x', nargs='?', const=shutil.which('xz'), type=str,
                   default=None, help='Enable xz (LZMA2) compression, and ' +
                   'optionally specify which xz binary to use.')
    p.add_argument('--lzma', '-l', const=shutil.which('lzma'), type=str,
                   nargs='?', default=None, help='Enable LZMA1 compression '+
                   '(using xz), and optionally specify which lzma binary to '+
                   'use.')
    p.add_argument('--zopfli', '-z', const=shutil.which('zopfli'), nargs='?',
                   type=str, default=None, help='Enable Zopfli-based gzip '+
                   'compression, and optionally specify which zopfli binary to'+
                   ' use.')

    p.add_argument('--nasm', type=str, default=shutil.which('nasm'),
                   help='nasm binary to use')
    p.add_argument('--vndh', type=str, default='ext/vondehi',
                   help='Directory of the vondehi source code')

    p.add_argument('--jobs', '-j', type=int, default=multiprocessing.cpu_count(),
                   help="Number of jobs that run in parallel for the bruteforcing")

    p.add_argument('--vndh_tag', type=str, help="Vanity tag to pass to vondehi")
    p.add_argument('--vndh_vfork', action='store_true',
                   help="Tell vondehi to use vfork(2)")
    p.add_argument('--vndh_unibin', action='store_true', help="Disable "+
                   "compatibility with distributions that keep /bin and "+
                   "/usr/bin separate.")
    p.add_argument('--vndh_cheat', action='store_true', help="Assume file "+
                   "descriptor numbers and hope arguments and environment "+
                   "variables are passed correctly by chance to the payload."+
                   " You cannot use this if you're running on Wayland.")
    p.add_argument('--vndh_no_argv', action='store_true', help="Don't properly"+
                   " keep track of the stack location, makes argv passing bork"+
                   " but saves a few bytes. Only in effect if --vndh_cheat is "+
                   "not enabled.")

    p.add_argument('--rawout', type=argparse.FileType('wb'),
                   help='File to write the raw compressed data to (if --vndh is enabled).')

    main(p.parse_args(sys.argv[1:]))


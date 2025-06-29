# Unix V5 Kernel - Meson Build

# Kernel sources
ken_sources = files(
  'ken/alloc.c',
  'ken/clock.c', 
  'ken/fio.c',
  'ken/iget.c',
  'ken/main.c',
  'ken/malloc.c',
  'ken/nami.c',
  'ken/pipe.c',
  'ken/prf.c',
  'ken/rdwri.c',
  'ken/sig.c',
  'ken/slp.c',
  'ken/subr.c',
  'ken/sys1.c',
  'ken/sys2.c',
  'ken/sys3.c',
  'ken/sys4.c',
  'ken/sysent.c',
  'ken/text.c',
  'ken/trap.c'
)

dmr_sources = files(
  'dmr/bio.c',
  'dmr/cat.c',
  'dmr/dc.c',
  'dmr/dh.c',
  'dmr/dhdm.c',
  'dmr/dhfdm.c',
  'dmr/dn.c',
  'dmr/dp.c',
  'dmr/kl.c',
  'dmr/lp.c',
  'dmr/mem.c',
  'dmr/partab.c',
  'dmr/pc.c',
  'dmr/rf.c',
  'dmr/rk.c',
  'dmr/rp.c',
  'dmr/tc.c',
  'dmr/tm.c',
  'dmr/tty.c',
  'dmr/vs.c',
  'dmr/vt.c'
)

conf_sources = files(
  'conf/conf.c',
  'conf/mkconf.c'
)

# Kernel library for analysis
unix_kernel_lib = static_library('unix_kernel',
  ken_sources + dmr_sources + conf_sources,
  c_args : [
    '-DKERNEL=1',
    '-DUNIX_V5=1',
    '-fno-builtin',
    '-nostdlib', 
    '-ffreestanding'
  ],
  include_directories : inc_sys
)

# Configuration generator
mkconf = executable('mkconf',
  'conf/mkconf.c',
  c_args : ['-Wno-implicit-function-declaration']
)

# Generate kernel configuration
c_conf = custom_target('kernel_config',
  output : 'c.c',
  command : [mkconf],
  capture : true
)

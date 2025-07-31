project('unix-v5', 'c',
  version : '1.0.0',
  license : 'BSD',
  default_options : [
    'warning_level=1',
    'c_std=c99',
    'buildtype=debug',
  ]
)

# Configuration options
kernel_enabled = get_option('kernel')
userland_enabled = get_option('userland') 
compiler_enabled = get_option('compiler')
use_clang = get_option('use_clang')
enable_coverage = get_option('coverage')
enable_analysis = get_option('analysis')
enable_docs = get_option('docs')

# Toolchain configuration
if use_clang
  add_project_arguments('-Wno-implicit-function-declaration', language : 'c')
  add_project_arguments('-Wno-int-conversion', language : 'c')
  add_project_arguments('-Wno-incompatible-pointer-types', language : 'c')
  add_project_arguments('-Wno-main', language : 'c')
  add_project_arguments('-fno-strict-aliasing', language : 'c')
endif

# Coverage support
if enable_coverage
  add_project_arguments('--coverage', language : 'c')
  add_project_link_arguments('--coverage', language : 'c')
endif

# Include directories
inc_sys = include_directories('usr/sys')

# External tools
cppcheck = find_program('cppcheck', required : false)
lizard = find_program('lizard', required : false)
lcov = find_program('lcov', required : false)
genhtml = find_program('genhtml', required : false)
doxygen = find_program('doxygen', required : false)
ctags = find_program('ctags', required : false)

# Analysis targets
if enable_analysis and cppcheck.found()
  run_target('cppcheck',
    command : [
      cppcheck,
      '--enable=all',
      '--std=c99', 
      '--platform=unix32',
      '--suppress=missingIncludeSystem',
      '--quiet',
      '--xml',
      '--xml-version=2',
      '--output-file=' + meson.current_build_dir() + '/cppcheck-report.xml',
      meson.current_source_dir() + '/usr'
    ]
  )
endif

if enable_analysis and lizard.found()
  run_target('complexity',
    command : [
      lizard,
      '--length', '15',
      '--CCN', '10', 
      '--arguments', '5',
      '--xml',
      meson.current_source_dir() + '/usr'
    ]
  )
endif

# Coverage targets
if enable_coverage and lcov.found() and genhtml.found()
  run_target('coverage-clean',
    command : [lcov, '--directory', '.', '--zerocounters']
  )
  
  run_target('coverage-capture',
    command : [
      lcov, '--directory', '.', '--capture', '--output-file', 'coverage.info'
    ]
  )
  
  run_target('coverage-report', 
    command : [
      genhtml, 'coverage.info', '--output-directory', 'coverage-html'
    ]
  )
endif

# Documentation
if enable_docs and doxygen.found()
  subdir('docs')
endif

# Subdirectories
if kernel_enabled
  subdir('usr/sys')
endif

if userland_enabled  
  subdir('usr/source')
endif

if compiler_enabled
  subdir('usr/c')
endif

# Tags target
if ctags.found()
  run_target('tags',
    command : [ctags, '-R', '--c-kinds=+p', '--fields=+S', meson.current_source_dir()]
  )
endif

# Summary
summary({
  'Kernel build' : kernel_enabled,
  'Userland build' : userland_enabled, 
  'Compiler build' : compiler_enabled,
  'Use Clang' : use_clang,
  'Coverage' : enable_coverage,
  'Analysis' : enable_analysis,
  'Documentation' : enable_docs,
}, section : 'Configuration')

summary({
  'cppcheck' : cppcheck.found(),
  'lizard' : lizard.found(),
  'lcov' : lcov.found(),
  'doxygen' : doxygen.found(),
  'ctags' : ctags.found(),
}, section : 'Tools')

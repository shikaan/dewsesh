#compdef dewsesh

_dewsesh() {
  _arguments \
    '(-c --configure)'{-c,--configure}'[Path to the configuration file]:path:_files' \
    '(-d --debug)'{-d,--debug}'[Enable debugging output]' \
    '(-h --help)'{-h,--help}'[Show this help message and quit]' \
    '(-v --version)'{-v,--version}'[Show the version number and quit]'
}

_dewsesh

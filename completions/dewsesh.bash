_dewsesh() {
  local cur prev opts
  COMPREPLY=()
  cur="${COMP_WORDS[COMP_CWORD]}"
  prev="${COMP_WORDS[COMP_CWORD - 1]}"
  opts="-c --configure -d --debug -h --help -v --version"

  case "$prev" in
    -c|--configure)
      COMPREPLY=($(compgen -f -- "$cur"))
      return
      ;;
  esac

  COMPREPLY=($(compgen -W "$opts" -- "$cur"))
}

complete -F _dewsesh dewsesh

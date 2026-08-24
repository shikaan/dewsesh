<h1 align="center">dewsesh</h1>

<p align="center">
A minimal, beautiful session manager for Wayland.
</p>

<p align="center">
  <img width="1024" alt="dimmed screen with dewsesh" src="https://github.com/shikaan/dewsesh/blob/main/screenshot.png?raw=true" />
</p>

dewsesh is a customizable logout menu for Wayland compositors: lock, log out,
suspend, hibernate, restart, or shut down, with sensible defaults and a
confirmation screen before anything irreversible.

## Installation

```sh
curl -s https://shikaan.github.io/sup/install | REPO=shikaan/dewsesh sh -
```

For a manual installation, grab a binary from the
[releases](https://github.com/shikaan/dewsesh/releases) page. To build from
source, see [CONTRIBUTING.md](CONTRIBUTING.md).

### Shell completions

Optionally, you can install shell completions:

```sh
# bash
curl -sL --create-dirs \
  https://raw.githubusercontent.com/shikaan/dewsesh/main/completions/dewsesh.bash \
  -o ~/.local/share/bash-completion/completions/dewsesh

# zsh - any directory on your $fpath works
curl -sL --create-dirs \
  https://raw.githubusercontent.com/shikaan/dewsesh/main/completions/dewsesh.zsh \
  -o ~/.local/share/zsh/site-functions/_dewsesh

# fish
curl -sL --create-dirs \
  https://raw.githubusercontent.com/shikaan/dewsesh/main/completions/dewsesh.fish \
  -o ~/.config/fish/completions/dewsesh.fish
```

## Usage

```sh
dewsesh
```

Configuration lives in a single file, by default
`$XDG_CONFIG_HOME/dewsesh/config`. See [dewsesh(1)](dewsesh.1.scd.tpl) for
every option and configuration key.

## Compatibility

dewsesh runs on any Wayland compositor implementing the layer-shell protocol,
which includes all wlroots-based ones.

> [!NOTE]
> Some compositors (notably, GNOME) do NOT implement the layer-shell protocol.
> Please open an [issue](https://github.com/shikaan/dewsesh/issues) if you'd
> like to use dewsesh on one of those compositors.

## Contributing

If you'd like to contribute code, request a feature, or report a bug, please
feel free to open an [issue](https://github.com/shikaan/dewsesh/issues).

## License

[MIT](./LICENSE)

<h1 align="center">dewsesh</h1>

<p align="center">
A minimal, beautiful session manager for Wayland.
</p>

<p align="center">
  <img width="1024" alt="dimmed screen with dewsesh" src="https://github.com/shikaan/dewsesh/blob/main/screenshot.png?raw=true" />
</p>

dewsesh is a customizable session manager, also called a logout menu, for 
Wayland compositors.

It takes inspiration from wlogout and oblogout, and differs in three ways:

- Irreversible actions can be stopped
- No config required
- One config file, no CSS

## Installation

### Static binary

```sh
curl -s https://shikaan.github.io/sup/install | REPO=shikaan/dewsesh sh -
```

### Packaged releases

Packages are available at
[Releases](https://github.com/shikaan/dewsesh/releases/latest).

For Debian-based distributions:

```sh
curl -LO https://github.com/shikaan/dewsesh/releases/latest/download/dewsesh-amd64.deb
sudo apt install ./dewsesh-amd64.deb
```

Replace `amd64` with `arm64` on ARM systems.

For Arch-based distributions, every release also includes a `PKGBUILD`:

```sh
curl -LO https://github.com/shikaan/dewsesh/releases/latest/download/PKGBUILD
makepkg -si
```

Packages install the executable, the manpage, and the shell completions.

### From source

Install the dependencies listed in [CONTRIBUTING.md](CONTRIBUTING.md), then
build and install dewsesh:

```sh
make all
sudo make install
```

This installs the files under `/usr/local`. To install dewsesh in a different
location, set the `PREFIX` variable:

```sh
make install PREFIX=~/.local
```

## Usage

```sh
dewsesh
```

Configuration lives in a single file, by default
`$XDG_CONFIG_HOME/dewsesh/config`. See [dewsesh(1)](dewsesh.1.scd.tpl) for
every option and configuration key.

## Compatibility

dewsesh needs a compositor that implements the layer-shell protocol
(`zwlr_layer_shell_v1`). Every wlroots-based compositor does, including sway,
river, and Wayfire, as do Hyprland and niri.

> [!NOTE]
> GNOME (mutter) and some other compositors do not implement the layer-shell
> protocol. If you want to use dewsesh on one of these compositors, open an
> [issue](https://github.com/shikaan/dewsesh/issues).

## Contributing

To contribute code, request a feature, or report a bug, open an
[issue](https://github.com/shikaan/dewsesh/issues).

## License

[MIT](./LICENSE)

<h1 align="center">dewsesh</h1>

<p align="center">
A minimal, beautiful session manager for Wayland.
</p>

<p align="center">
  <img width="640" alt="dimmed screen with dewsesh" src="https://raw.githubusercontent.com/shikaan/dewsesh/refs/heads/main/screenshot.png" />
</p>

dewsesh is a customizable session manager for Wayland compositors, inspired by 
[wlogout](https://github.com/ArtsyMacaw/wlogout) and
[oblogout](https://github.com/ryukinix/oblogout) with a focus on sensible
defaults, minimal dependencies, improved usability (e.g., confirmation screen). 

It's compatible with wlroots-based compositors and any other Wayland compositor
implementing the layer-shell protocol.

> [!NOTE]
> Some compositors (notably, GNOME) do NOT the layer-shell protocol. Please open
> an [issue](https://github.com/shikaan/dewsesh/issues) if you'd like to use 
> dewsesh on one of those compositors.

## Quick Start

### Installation

There are no packaged builds yet. However, installing from source is as easy as

```
make install
```

See [CONTRIBUTING.md](CONTRIBUTING.md) to check out the required dependencies.

### Usage

```sh
dewsesh
```

Configuration lives in a single file (by default 
`$XDG_CONFIG_HOME/dewsesh/config`). See [dewsesh(1)](dewsesh.1.scd.tpl) for
every option and configuration key.

## Contributing

If you'd like to contirbute code, request a feature or report a bug, please 
feel free to open an [issue](https://github.com/shikaan/dewsesh/issues).

## License

[MIT](./LICENSE)

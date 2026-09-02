Contributing
---

The easiest way to work on dewsesh is inside the provided
[container](#building-from-source-container), it ships all the tooling and
dependencies you need. Building [directly on your
machine](#building-from-source-local) works too, if you'd rather skip
Docker/Podman.

Once you're set up (either way):

```
make                   # build the binary and the manpage (needs scdoc)
make BUILD_TYPE=debug  # build with sanitizers and debug symbols
make install           # install dewsesh under /usr/local
make main              # build only the binary
make help              # show the available targets and parameters
```

## Building from source (local)

Tooling (dev-only, not linked into the binary):
* GNU make
* pkg-config
* wayland-scanner (usually part of `libwayland-bin`/`wayland-utils`)
* a C11 compiler (for example, gcc)
* scdoc (optional: builds the manpage)

Dependencies:

* wayland-client
* cairo
* freetype2

```sh
make
```

## Building from source (container)

The project comes with a [Dockerfile](./Dockerfile) containing all the
dependencies required to build the project.

Using `docker` or `podman` you can

```sh
# build the development image
podman build . -t dewsesh

# run the development container
podman run --rm -it -v "$(pwd):/src:Z" --name dewsesh dewsesh
```

## Building a static binary

Releases include a single statically linked executable.
[Dockerfile.static](./Dockerfile.static) contains the toolchain and the steps to
build it.

```sh
podman build . -f Dockerfile.static -o dist
```

That writes `dist/dewsesh` and `dist/dewsesh.1`. A tool like
[sup](https://github.com/shikaan/sup) can then distribute these files.

> [!NOTE]
> Outside the image, `make STATIC=1` switches `pkg-config` to `--static` and 
> adds `-static` to the link. It needs static libraries for every dependency, 
> which most distributions do not package.
>
> Use a [dynamic build](#building-from-source-container) for local development

## Packaging

### Generating a dynamic build

Build and install dewsesh with make:

```sh
make VERSION="$pkgver" ERR_ON_WARN=0 all
make install DESTDIR="$pkgdir" PREFIX=/usr
```

`ERR_ON_WARN=0` removes `-Werror`. Warnings then do not stop the build when the
distribution adds its own compiler flags.

These commands create the files below:

```
$pkgdir/usr/bin/dewsesh
$pkgdir/usr/share/man/man1/dewsesh.1
$pkgdir/usr/share/bash-completion/completions/dewsesh
$pkgdir/usr/share/zsh/site-functions/_dewsesh
$pkgdir/usr/share/fish/vendor_completions.d/dewsesh.fish
```

### deb and AUR

The [`packaging`](./packaging) directory contains the files that build the deb
and the AUR packages.

Build the deb package with [Dockerfile.deb](./Dockerfile.deb). It contains the
dependencies and the steps to build it.

```sh
podman build -f Dockerfile.deb -o dist .
```

For AUR, the release pipeline creates a `PKGBUILD` from the template
`packaging/aur/PKGBUILD.tpl`, and attaches it to the release.

The pipeline configuration is in
[.github/workflows/release.yml](./.github/workflows/release.yml).

## Using LSPs

You can get LSP (clangd) support from the running container like this

```sh
# run the container as above

# run the LSP 
podman exec -i dewsesh clangd \
    --background-index \
    --path-mappings=<local-path-to-dewsesh>/dewsesh=/src"
```

## Appendix: house style

* **No or very short comments** The code is the documentation; rationale belongs
  in commit messages and pull requests, where it can be read and discussed.

* **Allocate at startup.** Everything is allocated before the event loop, from
  fixed static pools sized to what the program actually needs (see `src/timer.c`
  and `src/ui.c`). Nothing allocates per frame or per event. If a library gives
  you no choice (cairo, for one), do it once, as early as it allows.

* **Fallible functions return `result_t`,** with the destination as the last
  argument: `result_t ui_font_family(const char *family, ui_font_t **font)`. If
  it can't fail, return `void`. New codes go in `src/result.h`, grouped by
  module under a bare `ERR_<MOD>`.

* **Log where it breaks.** The function that detects a failure logs it, right
  next to its `return ERR_...`; callers just propagate. The exception is a
  caller that changes behaviour: a fallback, a retry, or a degraded mode will
  log what they are doing instead.

* **Modules hide their dependencies.** Public headers don't name types from the
  libraries behind them: `src/ui.h` has `typedef struct ui_font ui_font_t;` and
  keeps cairo inside `src/ui.c`.

* **Skip the indirection.** One of something? A file-scope `static`, assigned in
  the module's init function. No getter, no lazy init.

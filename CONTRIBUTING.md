Contributing
---

The easiest way to work on dewsesh is inside the provided
[container](#building-from-source-container), it ships all the tooling and
dependencies you need. Building [directly on your
machine](#building-from-source-local) works too, if you'd rather skip
Docker/Podman.

Once you're set up (either way):

```
make          # to build binary and manpage (if scdoc is available)
make install  # to install dewsesh locally
make main     # to only build the binary
make help     # to see available make targets
```

## Building from source (local)

Tooling (dev-only, not linked into the binary):
* GNU make
* pkg-config
* wayland-scanner (usually part of `libwayland-bin`/`wayland-utils`)
* a C11 compiler (e.g. gcc or clang)
* git (optional: embeds the commit SHA in `--version`)
* scdoc (optional: builds the man page)

Dependencies:

* wayland-client
* wayland-protocols \*
* cairo
* freetype2

_\* Compile-time dep_

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

## Making a static binary

Releases ship a single statically linked executable, built against musl inside
Alpine by [Dockerfile.static](./Dockerfile.static).

```sh
podman build . -f Dockerfile.static -o dist
```

That writes `dist/dewsesh` and `dist/dewsesh.1`.

> [!NOTE]
> Outside the image, `make STATIC=1` switches `pkg-config` to `--static` and 
> adds `-static` to the link. It needs static libraries for every dependency, 
> which most distributions do not package.
>
> Use a [dynamic build](#building-from-source-container) for local development

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

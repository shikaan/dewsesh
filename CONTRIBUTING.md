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

## Using LSPs

You can get LSP (clangd) support from the running container like this

```sh
# run the container as above

# run the LSP 
podman exec -i dewsesh clangd \
    --background-index \
    --path-mappings=<local-path-to-dewsesh>/dewsesh=/src"
```

COMMON_CFLAGS := -std=c11 \
	-D_DEFAULT_SOURCE \
	-Wall \
	-Wextra \
	-pedantic \
	-fdiagnostics-color=always \
	-fno-common \
	-Winit-self \
	-Wfloat-equal \
	-Wundef \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-align \
	-Wstrict-prototypes \
	-Wstrict-overflow=5 \
	-Wwrite-strings \
	-Waggregate-return \
	-Wcast-qual \
	-Wswitch-default \
	-Wswitch-enum \
	-Wconversion \
	-Wno-ignored-qualifiers \
	-Wno-aggregate-return

SANITIZERS := -fsanitize=address,undefined

DEBUG_CFLAGS := -g -O0 $(SANITIZERS) -DDEBUG

# Keep debug symbols, packaging tools split them into their own package
RELEASE_CFLAGS := -O2 -DNDEBUG -g

# Allow packagers to introduce their own flags
DISTRO_CFLAGS := $(CFLAGS)

# ------------------

##P ERR_ON_WARN - 0 to keep warnings non-fatal (default: 1)
ERR_ON_WARN ?= 1
ifeq ($(ERR_ON_WARN),1)
    COMMON_CFLAGS += -Werror
endif

##P BUILD_TYPE  - 'debug' for sanitizers (default: 'release')
BUILD_TYPE ?= release
ifeq ($(BUILD_TYPE),debug)
    CFLAGS := $(COMMON_CFLAGS) $(DEBUG_CFLAGS) $(DISTRO_CFLAGS)
    LDFLAGS += $(SANITIZERS)
else
    CFLAGS := $(COMMON_CFLAGS) $(RELEASE_CFLAGS) $(DISTRO_CFLAGS)
endif

##P STATIC      - 1 to link the dependencies statically (default: 0)
STATIC ?= 0

DEPS := wayland-client cairo freetype2

ifeq ($(STATIC),1)
    LDFLAGS += -static
    DEPS_CFLAGS := $(shell pkg-config --static --cflags $(DEPS))
    # cairo advertises its Xlib and XCB backends in Libs.private. dewsesh never
    # calls them, and Alpine ships no libXau.a, so the link would fail on a
    # library the binary does not use.
    X11_LIBS := -lX11 -lXext -lXrender -lXau -lXdmcp \
	-lxcb -lxcb-render -lxcb-shm
    DEPS_LIBS := $(filter-out $(X11_LIBS),$(shell pkg-config --static --libs $(DEPS)))
else
    DEPS_CFLAGS := $(shell pkg-config --cflags $(DEPS))
    DEPS_LIBS := $(shell pkg-config --libs $(DEPS))
endif

# ------------------

##P VERSION     - version number in help and manpages (default: 'v0.0.0')
VERSION ?= v0.0.0

##P SHA         - SHA hash in help and manpages (default: 'dev')
SHA ?= $(shell git rev-parse --short HEAD 2>/dev/null || echo dev)

##P PREFIX      - install prefix (default: '/usr/local')
PREFIX ?= /usr/local

##P DESTDIR     - staging directory prepended to every install path
DESTDIR ?=

# ------------------

BINDIR := $(DESTDIR)$(PREFIX)/bin
MANDIR := $(DESTDIR)$(PREFIX)/share/man/man1
BASHDIR := $(DESTDIR)$(PREFIX)/share/bash-completion/completions
ZSHDIR := $(DESTDIR)$(PREFIX)/share/zsh/site-functions
FISHDIR := $(DESTDIR)$(PREFIX)/share/fish/vendor_completions.d

.PHONY: all install docs clean help

### all - build the binary and generate the manpage (default)
all: main docs

### install - install the executable, the manpage and the completions
install:
	@if [ ! -f main ]; then \
		echo "ERROR: missing binary. Run 'make all' first."; \
		exit 1; \
	fi
	@echo "Installing dewsesh..."
	@install -Dm755 main ${BINDIR}/dewsesh
	@echo "  Executable : ${BINDIR}/dewsesh"
	@if [ -f dewsesh.1.roff ]; then \
		install -Dm644 dewsesh.1.roff ${MANDIR}/dewsesh.1; \
		echo "  Man        : ${MANDIR}/dewsesh.1"; \
	fi
	@install -Dm644 completions/dewsesh.bash ${BASHDIR}/dewsesh
	@install -Dm644 completions/dewsesh.zsh ${ZSHDIR}/_dewsesh
	@install -Dm644 completions/dewsesh.fish ${FISHDIR}/dewsesh.fish
	@echo "  Completions: ${BASHDIR}, ${ZSHDIR}, ${FISHDIR}"
	@echo "Installing dewsesh... DONE"

### docs - generate the manpage from the scdoc template
docs:
	@if command -v scdoc > /dev/null 2>&1; then \
		echo "Generating manpage dewsesh.1.roff..."; \
		sed "s/##VERSION##/${VERSION}/g; s/##SHA##/${SHA}/g" dewsesh.1.scd.tpl > dewsesh.1.scd; \
		scdoc < dewsesh.1.scd > dewsesh.1.roff; \
		echo "Generating manpage dewsesh.1.roff... DONE"; \
	else \
		echo "WARN: Unable to find scdoc. Skipping manpage generation."; \
	fi

### help - list available targets
help:
	@echo "Usage: make [parameters] [target]"
	@echo
	@echo "Parameters:"
	@grep -e '^##P ' $(MAKEFILE_LIST) | sed 's/^##P /  /'
	@echo
	@echo "Targets:"
	@grep -e '^### ' $(MAKEFILE_LIST) | sed 's/^### /  /'

### clean - remove build artifacts
clean:
	rm -f main *.o src/*.o protocols/*.c protocols/*.h

# ---------------------

protocols/xdg-shell-protocol.h:
	wayland-scanner client-header \
		./protocols/xdg-shell.xml $@

protocols/xdg-shell-protocol.c: protocols/xdg-shell-protocol.h
	wayland-scanner private-code \
		./protocols/xdg-shell.xml $@

protocols/xdg-shell-protocol.o: CFLAGS := -O2 $(DISTRO_CFLAGS)
protocols/xdg-shell-protocol.o: protocols/xdg-shell-protocol.h \
	protocols/xdg-shell-protocol.c

protocols/wlr-layer-shell-unstable-v1.h: protocols/xdg-shell-protocol.o
	wayland-scanner client-header \
		./protocols/wlr-layer-shell-unstable-v1.xml $@

protocols/wlr-layer-shell-unstable-v1.c: protocols/wlr-layer-shell-unstable-v1.h
	wayland-scanner private-code \
		./protocols/wlr-layer-shell-unstable-v1.xml $@

protocols/wlr-layer-shell-unstable-v1.o: CFLAGS := -O2 $(DISTRO_CFLAGS)
protocols/wlr-layer-shell-unstable-v1.o: protocols/wlr-layer-shell-unstable-v1.h \
	protocols/wlr-layer-shell-unstable-v1.c

protocols/cursor-shape-v1.h: protocols/wlr-layer-shell-unstable-v1.o
	wayland-scanner client-header \
		./protocols/cursor-shape-v1.xml $@

protocols/cursor-shape-v1.c: protocols/cursor-shape-v1.h
	wayland-scanner private-code \
		./protocols/cursor-shape-v1.xml $@

protocols/cursor-shape-v1.o: CFLAGS := -O2 $(DISTRO_CFLAGS)
protocols/cursor-shape-v1.o: protocols/cursor-shape-v1.h \
	protocols/cursor-shape-v1.c

src/cli:
main.o: protocols/wlr-layer-shell-unstable-v1.h
src/ui.o: src/ctx.o src/log.o assets/font-awesome-v4.h
src/ctx.o: src/log.o
src/timer.o: src/log.o
src/shell.o: src/log.o src/timer.o src/ctx.o \
	protocols/wlr-layer-shell-unstable-v1.h protocols/cursor-shape-v1.h
src/spawn.o: src/log.o
src/config.o: src/log.o

main: CFLAGS += $(DEPS_CFLAGS) \
	-isystem protocols -DVERSION='"$(VERSION)"' -DSHA='"$(SHA)"'
main: LDLIBS += $(DEPS_LIBS)
main: main.o protocols/wlr-layer-shell-unstable-v1.o \
	protocols/xdg-shell-protocol.o protocols/cursor-shape-v1.o src/log.o \
	src/ctx.o src/shell.o src/ui.o src/spawn.o src/timer.o src/cli.o \
	src/config.o src/app.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


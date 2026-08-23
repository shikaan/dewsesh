BUILD_TYPE ?= debug

COMMON_CFLAGS := -std=c11 \
	-D_DEFAULT_SOURCE \
	-Wall \
	-Wextra \
	-Werror \
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

DEBUG_CFLAGS := -g -O0 -fsanitize=address,undefined -DDEBUG

RELEASE_CFLAGS := -O2 -DNDEBUG

ifeq ($(BUILD_TYPE),release)
    CFLAGS := $(COMMON_CFLAGS) $(RELEASE_CFLAGS)
else
    CFLAGS := $(COMMON_CFLAGS) $(DEBUG_CFLAGS)
endif

# ------------------

WAYLAND_PROTOCOLS_DIR := $(shell pkg-config --variable=pkgdatadir wayland-protocols)

VERSION ?= v0.0.0
SHA ?= $(shell git rev-parse --short HEAD 2>/dev/null || echo dev)

.PHONY: all install docs clean help

### all - build the binary and generate the manpage (default)
all: main docs

### install - build in release mode and install the executable and manpage
install: MAN_FOLDER := ~/.local/share/man/man1
install: BIN_FOLDER := ~/.local/bin
install:
	@echo "Installing dewsesh..."
	@make -s BUILD_TYPE=release all
	@mkdir -p ${BIN_FOLDER}
	@cp ./main ${BIN_FOLDER}/dewsesh
	@chmod +x ${BIN_FOLDER}/dewsesh
	@echo "Installing dewsesh... DONE"
	@echo "  Executable: ${BIN_FOLDER}/dewsesh"
	@if [ -f dewsesh.1.roff ]; then \
		mkdir -p ${MAN_FOLDER}; \
		cp dewsesh.1.roff ${MAN_FOLDER}/dewsesh.1; \
		echo "  Man       : ${MAN_FOLDER}/dewsesh.1"; \
	fi

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
	@echo "Usage: make [target]"
	@echo
	@echo "Targets:"
	@grep -E '^### ' $(MAKEFILE_LIST) | sed 's/^### /  /'

### clean - remove build artifacts
clean:
	rm -f main *.o src/*.o protocols/*.c protocols/*.h

# ---------------------

protocols/xdg-shell-protocol.h:
	wayland-scanner client-header \
		$(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml $@

protocols/xdg-shell-protocol.c: protocols/xdg-shell-protocol.h
	wayland-scanner private-code \
		$(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml $@

protocols/xdg-shell-protocol.o: CFLAGS := -O2
protocols/xdg-shell-protocol.o: protocols/xdg-shell-protocol.h \
	protocols/xdg-shell-protocol.c

protocols/wlr-layer-shell-unstable-v1.h: protocols/xdg-shell-protocol.o
	wayland-scanner client-header \
		./protocols/wlr-layer-shell-unstable-v1.xml $@

protocols/wlr-layer-shell-unstable-v1.c: protocols/wlr-layer-shell-unstable-v1.h
	wayland-scanner private-code \
		./protocols/wlr-layer-shell-unstable-v1.xml $@

protocols/wlr-layer-shell-unstable-v1.o: CFLAGS := -O2
protocols/wlr-layer-shell-unstable-v1.o: protocols/wlr-layer-shell-unstable-v1.h \
	protocols/wlr-layer-shell-unstable-v1.c

protocols/cursor-shape-v1.h: protocols/wlr-layer-shell-unstable-v1.o
	wayland-scanner client-header \
		./protocols/cursor-shape-v1.xml $@

protocols/cursor-shape-v1.c: protocols/cursor-shape-v1.h
	wayland-scanner private-code \
		./protocols/cursor-shape-v1.xml $@

protocols/cursor-shape-v1.o: CFLAGS := -O2
protocols/cursor-shape-v1.o: protocols/cursor-shape-v1.h \
	protocols/cursor-shape-v1.c

src/cli:
src/ui.o: src/ctx.o src/log.o
src/ctx.o: src/log.o
src/timer.o: src/log.o
src/shell.o: src/log.o src/timer.o src/ctx.o \
	protocols/wlr-layer-shell-unstable-v1.h protocols/cursor-shape-v1.h
src/spawn.o: src/log.o
src/config.o: src/log.o

main: CFLAGS += $(shell pkg-config --cflags wayland-client cairo freetype2) \
	-isystem protocols -DVERSION='"$(VERSION)"' -DSHA='"$(SHA)"'
main: LDLIBS += $(shell pkg-config --libs wayland-client cairo freetype2)
main: protocols/wlr-layer-shell-unstable-v1.o protocols/xdg-shell-protocol.o \
	protocols/cursor-shape-v1.o src/log.o src/ctx.o src/shell.o src/ui.o \
	src/spawn.o src/timer.o src/cli.o src/config.o src/app.o


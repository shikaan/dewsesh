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

src/ui.o: src/ctx.o
src/ctx.o: src/log.o
src/timer.o: src/log.o
src/shell.o: src/log.o src/timer.o src/ctx.o
src/spawn.o: src/log.o

main: CFLAGS += $(shell pkg-config --cflags wayland-client cairo) \
	-isystem protocols
main: LDLIBS += $(shell pkg-config --libs wayland-client cairo)
main: protocols/wlr-layer-shell-unstable-v1.o protocols/xdg-shell-protocol.o \
	src/log.o src/ctx.o src/shell.o src/ui.o src/spawn.o src/timer.o	

clean:
	rm -f main *.o src/*.o protocols/*.c protocols/*.h

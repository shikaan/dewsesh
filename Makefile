WAYLAND_PROTOCOLS_DIR := $(shell pkg-config --variable=pkgdatadir wayland-protocols)

CFLAGS  += $(shell pkg-config --cflags wayland-client cairo) -I.
LDLIBS  += $(shell pkg-config --libs wayland-client cairo)

GEN_SRC := wlr-layer-shell-unstable-v1.c xdg-shell-protocol.c
GEN_HDR := wlr-layer-shell-unstable-v1.h xdg-shell-client-protocol.h

protocols/xdg-shell-protocol.h:
	wayland-scanner client-header \
		$(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml $@

protocols/xdg-shell-protocol.c: protocols/xdg-shell-protocol.h
	wayland-scanner private-code \
		$(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml $@

protocols/wlr-layer-shell-unstable-v1.h: protocols/xdg-shell-protocol.h
	wayland-scanner client-header \
		./protocols/wlr-layer-shell-unstable-v1.xml $@

protocols/wlr-layer-shell-unstable-v1.c: protocols/wlr-layer-shell-unstable-v1.h
	wayland-scanner private-code \
		./protocols/wlr-layer-shell-unstable-v1.xml $@

main: protocols/wlr-layer-shell-unstable-v1.o protocols/xdg-shell-protocol.o

clean:
	rm -f main *.o protocols/*.c protocols/*.h

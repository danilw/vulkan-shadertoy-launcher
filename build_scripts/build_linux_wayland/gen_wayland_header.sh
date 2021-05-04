#!/bin/sh

wayland_protocols_dir=$(pkg-config --variable=pkgdatadir wayland-protocols)

wayland-scanner client-header $wayland_protocols_dir/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
wayland-scanner private-code $wayland_protocols_dir/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.c



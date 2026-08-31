dewsesh(1) "##VERSION## (##SHA##)"

# NAME

dewsesh - A session manager for Wayland

# SYNOPSIS

dewsesh [_options_]

# DESCRIPTION

dewsesh is a customizable session manager - also called a logout menu - for
Wayland compositors.

When dewsesh starts, it shows a menu to lock the screen, log out, suspend,
hibernate, restart, or shut down the machine.

## COMPATIBILITY

dewsesh needs a compositor that implements the layer-shell protocol
(_zwlr\_layer\_shell\_v1_). Every wlroots-based compositor does, including
sway, river, and Wayfire, as do Hyprland and niri.

GNOME (mutter) and some others do not. If you want support for your favorite
compositor, open an issue at https://github.com/shikaan/dewsesh/issues

# OPTIONS

*-c, --config* <path>
	The config file to use. By default, the following paths are checked:
	_$XDG\_CONFIG\_HOME/dewsesh/config_ (or _$HOME/.config/dewsesh/config_
	if _$XDG\_CONFIG\_HOME_ is unset), and _SYSCONFDIR/dewsesh/config_. See
	*CONFIGURATION* for details.

*-d, --debug*
	Enable debugging output.

*-h, --help*
	Show help message and quit.

*-v, --version*
	Show the version number and quit.

# CONFIGURATION

The config file consists of _namespace.key=value_ pairs, one per line.
dewsesh treats lines that start with *#* as comments. See *-c* in
*OPTIONS* for the config file lookup paths.

*font.text* <font family>
	Sets the font family for general text. Defaults to _sans-serif_.

*font.status* <font family>
	Sets the font family for status text. Defaults to _monospace_.

*font.icon* <font family>
	Sets the font family for the icons. Defaults to a bundled icon set.

*font.size* <size>
	Sets the font size. dewsesh also uses this value to set the spacing
	of every other element on the screen. Defaults to _16_.

*action.lock* <command>
	Sets the command to lock the screen. Defaults to _loginctl lock-session_.

*action.suspend* <command>
	Sets the command to suspend the machine. Defaults to _systemctl
	suspend-then-hibernate_.

*action.hibernate* <command>
	Sets the command to hibernate the machine. Defaults to _systemctl hibernate_.

*action.logout* <command>
	Sets the command to log out from the current session. Defaults to
	_loginctl terminate-session_.

*action.restart* <command>
	Sets the command to restart the machine. Defaults to _systemctl reboot_.

*action.shutdown* <command>
	Sets the command to shut down the machine. Defaults to _systemctl poweroff_.

*color.overlay* <rrggbbaa>
	Sets the color of the overlay drawn over the screen and behind the
	session menu. Defaults to _000000CC_.

*color.text* <rrggbbaa>
	Sets the color of general text and icons. Defaults to _EAEAEAFF_.

*color.status* <rrggbbaa>
	Sets the color of status text. Defaults to _C4C8C6FF_.

*color.error* <rrggbbaa>
	Sets the color of error messages. Defaults to _FF6B6BFF_.

*color.selected* <rrggbbaa>
	Sets the color of selected items. Defaults to _82A2BE80_.

*color.button* <rrggbbaa>
	Sets the background color of (unselected) buttons. Defaults to _00000000_.

*color.window* <rrggbbaa>
	Sets the background color of window. Defaults to _00000000_.

## DEFAULTS

An empty configuration file is equivalent to the following configuration:

```
font.text=sans-serif
font.status=monospace
font.size=16

action.lock=loginctl lock-session
action.suspend=systemctl suspend-then-hibernate
action.hibernate=systemctl hibernate
action.logout=loginctl terminate-session
action.restart=systemctl reboot
action.shutdown=systemctl poweroff

color.overlay=000000CC
color.text=EAEAEAFF
color.status=C4C8C6FF
color.error=FF6B6BFF
color.selected=82A2BE80
color.button=00000000
color.window=00000000
```

# EXAMPLES

Use dewlock to lock the screen, plain suspend instead of suspend-then-hibernate,
and a lighter overlay:

```
action.lock=dewlock
action.suspend=systemctl suspend

color.overlay=00000088
```

Match a light desktop theme:

```
color.overlay=FFFFFFCC
color.text=1C1C1CFF
color.status=4A4A4AFF
color.selected=82A2BE80
```

Start dewsesh with a config kept outside the search paths:

```
dewsesh --config ~/dotfiles/dewsesh.conf
```

# AUTHOR
	Manuel Spagnolo <_shikaan@disroot.org_>

# SEE ALSO
	Project homepage: _https://github.com/shikaan/dewsesh_

# LICENSE
	dewsesh is MIT licensed.

	The bundled Font Awesome 4.7.0 icon font is licensed separately under
	the SIL Open Font License 1.1 _https://scripts.sil.org/OFL_. Full text:
	_https://github.com/shikaan/dewsesh/blob/##VERSION##/LICENSE_

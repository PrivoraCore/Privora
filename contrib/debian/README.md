
Debian
====================
This directory contains files used to package privorad/privora-qt
for Debian-based Linux systems. If you compile privorad/privora-qt yourself, there are some useful files here.

## privora: URI support ##


privora-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install privora-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your privora-qt binary to `/usr/bin`
and the `../../share/pixmaps/privora128.png` to `/usr/share/pixmaps`

privora-qt.protocol (KDE)


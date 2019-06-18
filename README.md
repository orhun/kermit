# k3rmit [![Release](https://img.shields.io/github/release/KeyLo99/k3rmit.svg?style=flat-square&color=green)](https://github.com/KeyLo99/k3rmit/releases)
[![AUR](https://img.shields.io/aur/version/k3rmit-git.svg?style=flat-square)](https://aur.archlinux.org/packages/k3rmit-git/) [![License](https://img.shields.io/badge/license-GPLv3-blue.svg?style=flat-square&color=red)](./LICENSE)

### A VTE-based terminal emulator that aims to be simple, fast and effective.

![Kermit the Frog](https://user-images.githubusercontent.com/24392180/59636824-2af20180-915d-11e9-95dd-0a077ebc3cfa.gif)

[VTE](https://developer.gnome.org/vte/) is a [GTK](https://developer.gnome.org/gtk3/3.0/) widget that allows creating a virtual terminal emulator which is used in many emulators such as [GNOME Terminal](https://help.gnome.org/users/gnome-terminal/stable/), [ROXTerm](https://github.com/realh/roxterm) and [evilvte](http://www.calno.com/evilvte/). Although there's a lot of (VTE-based or other) terminal emulator options for GNU/Linux users, `k3rmit` tries to differ from them with its simplicity.   
The project inspired by [Vincent Bernat](https://vincent.bernat.ch/en)'s [article](https://vincent.bernat.ch/en/blog/2017-write-own-terminal) and also his [implementation](https://github.com/vincentbernat/vbeterm) of a custom VTE-based terminal. Also, [Rxvt](https://wiki.archlinux.org/index.php/Rxvt-unicode) and [termite](https://github.com/thestinger/termite)'s appearance are taken as an example.

## Installation

### 🞄 AUR ([k3rmit-git](https://aur.archlinux.org/packages/k3rmit-git/))

### 🞄 CMake

```
mkdir -p build && cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
sudo ldconfig
```

### 🞄 GCC

```
cd src/
gcc -O2 -Wall $(pkg-config --cflags vte-2.91) k3rmit.c -o k3rmit.o $(pkg-config --libs vte-2.91)
```

## Features

* Use default shell (with `$SHELL` environment variable)
* Supports transparency with a composite manager (such as [compton](https://github.com/chjj/compton))
* No tab support
* Customizable

## Key Bindings

| Key             | Action                     |
|-----------------|----------------------------|
| ctrl-alt-[c]    | copy to clipboard          |
| ctrl-alt-[v]    | paste from clipboard       |
| ctrl-alt-[+][1] | increase font size         |
| ctrl-alt-[-][2] | decrease font size         |
| ctrl-alt-[=]    | reset font size to default |
| ctrl-alt-[r]    | reload configuration file  |

## Customization
1
https://askubuntu.com/questions/115762/increase-padding-in-gnome-terminal

## Screenshots

![Screenshot I](https://user-images.githubusercontent.com/24392180/59642133-88417f00-916c-11e9-956b-0fc41ac752c7.jpg)

![Screenshot II](https://user-images.githubusercontent.com/24392180/59641515-88407f80-916a-11e9-9834-fda387299419.jpg)


## Todo(s)

* Improve scrolling & fix scroll issues
* Add new key combinations for simpler usage

## License

GNU General Public License v3. (see [gpl](https://www.gnu.org/licenses/gpl.txt))

## Credit

Copyright (C) 2019 by KeyLo99 https://www.github.com/KeyLo99

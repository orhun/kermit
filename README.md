# k3rmit [![Release](https://img.shields.io/github/release/KeyLo99/k3rmit.svg?style=flat-square&color=green)](https://github.com/KeyLo99/k3rmit/releases)
[![AUR](https://img.shields.io/aur/version/k3rmit-git.svg?style=flat-square)](https://aur.archlinux.org/packages/k3rmit-git/) [![License](https://img.shields.io/badge/license-GPLv3-blue.svg?style=flat-square&color=red)](./LICENSE)

### A VTE-based terminal emulator that aims to be simple, fast and effective.

![Kermit the Frog](https://user-images.githubusercontent.com/24392180/59636824-2af20180-915d-11e9-95dd-0a077ebc3cfa.gif)

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

## Customization

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

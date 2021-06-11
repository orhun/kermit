# kermit [![Release](https://img.shields.io/github/release/orhun/kermit.svg?color=5B7C33&style=flat-square)](https://github.com/orhun/kermit/releases) [![AUR](https://img.shields.io/aur/version/kermit.svg?color=5B7C33&style=flat-square)](https://aur.archlinux.org/packages/kermit/)

### A VTE-based, simple and froggy terminal emulator.
![Kermit the Frog](https://user-images.githubusercontent.com/24392180/59636824-2af20180-915d-11e9-95dd-0a077ebc3cfa.gif)

[VTE](https://developer.gnome.org/vte/) is a [GTK](https://developer.gnome.org/gtk3/3.0/) widget that is designed to create virtual terminal emulators. [GNOME Terminal](https://help.gnome.org/users/gnome-terminal/stable/), [ROXTerm](https://github.com/realh/roxterm) and [evilvte](http://www.calno.com/evilvte/) are a few examples of terminal emulators that use the VTE widget. With the case of kermit, although it is a "yet another" VTE implementation, it aims to keep everything simple and customizable while providing some additional features.

The project is inspired by [Vincent Bernat](https://vincent.bernat.ch/en)'s [article](https://vincent.bernat.ch/en/blog/2017-write-own-terminal) and also his [implementation](https://github.com/vincentbernat/vbeterm) of a custom VTE-based terminal. Terminal features and appearance are mostly influenced by [Rxvt](https://wiki.archlinux.org/index.php/Rxvt-unicode), [termite](https://github.com/thestinger/termite) and [st](https://st.suckless.org/).

<details>
  <summary>Table of Contents</summary>

  * [Installation](#installation)
    + [AUR](#aur)
    + [CMake](#cmake)
    + [Make](#make)
    + [GCC](#gcc)
  * [Features](#features)
  * [Arguments](#arguments)
  * [Key Bindings](#key-bindings)
  * [Customization](#customization)
    + [Config File](#config-file)
    + [Theme](#theme)
    + [Font](#font)
    + [Key Bindings](#key-bindings-1)
    + [Padding](#padding)
  * [Screenshots](#screenshots)
  * [TODO(s)](#todos)
  * [License](#license)
  * [Copyright](#copyright)

</details>

## Installation

### AUR
* [kermit](https://aur.archlinux.org/packages/kermit/)
* [kermit-git](https://aur.archlinux.org/packages/kermit-git/)

### CMake

```
mkdir -p build && cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
sudo ldconfig
```

### Make

```
make
sudo make install
```

### GCC

```
cd src/
gcc -s -O3 -Wall -Wno-deprecated-declarations $(pkg-config --cflags vte-2.91) kermit.c -o kermit.o $(pkg-config --libs vte-2.91)
```

\* `kermit` terminal depends on [vte3](https://www.archlinux.org/packages/extra/x86_64/vte3/) and [gtk3](https://www.archlinux.org/packages/extra/x86_64/gtk3/) packages.

## Features

* Uses the default shell (with `$SHELL` environment variable)
* Supports transparency with a composite manager (such as [compton](https://github.com/chjj/compton))
* Supports base16 color schemes (customizable theme)
* Supports custom keys and associated commands
* Supports tabs

## Arguments

```
kermit [-h] [-v] [-d] [-c config] [-t title] [-w workdir] [-e command]

[-h] shows help
[-v] shows version
[-d] enables the debug messages
[-c config]  specifies the configuration file
[-t title]   sets the terminal title
[-w workdir] sets the working directory
[-e command] sets the command to execute in terminal
```

## Key Bindings

| Key                         | Action                         |
|-----------------------------|--------------------------------|
| `ctrl-alt-[c]`              | copy to clipboard              |
| `ctrl-alt-[v]`              | paste from clipboard           |
| `ctrl-alt-[t]`              | open a new tab                 |
| `ctrl-alt-[r]`              | reload configuration file      |
| `ctrl-alt-[d]`              | load the default configuration |
| `ctrl-alt-[q]`              | exit the terminal              |
| `ctrl-alt-[k][up]`          | increase font size             |
| `ctrl-alt-[j][down]`        | decrease font size             |
| `ctrl-alt-[=]`              | reset font size to default     |
| `ctrl-alt-[return]`         | open a new tab                 |
| `ctrl-alt-[num]`            | switch to the tab number       |
| `ctrl-alt-[l][right][pgup]` | switch to the next tab         |
| `ctrl-alt-[h][left][pgdn]`  | switch to the previous tab     |
| `ctrl-alt-[w][backspace]`   | close the selected tab         |

• Key bindings (`ctrl-alt`) can be set to `ctrl-shift` with using the config file.

• Default key bindings (`ctrl-alt`) might conflict with your desktop environments window shortcuts. To solve this issue, key bindings should be changed to `ctrl-shift`.

## Customization

### Config File

Most of the settings including font, opacity and colors can be changed via the config file. The default configuration file is available [here](https://github.com/orhun/kermit/blob/master/.config/kermit.conf).

`kermit` looks for configuration file in `~/.config/kermit.conf`

### Theme

Terminal theme can be changed with either editing the config file manually or using the [base16](https://github.com/chriskempson/base16) color schemes in [orhun/base16-kermit](https://github.com/orhun/base16-kermit) repository.

### Font

`kermit` uses a [PangoFontDescription](https://developer.gnome.org/pygtk/stable/class-pangofontdescription.html) which retrieved from the `kermit.conf` for changing the font family, style and size. The configuration entry format of the font and some examples are shown below and the default value is `monospace 9`.

```
font [FAMILY-LIST] [STYLE-OPTIONS] [SIZE]
```

`FAMILY-LIST` is a comma separated list of families optionally terminated by a comma, `STYLE_OPTIONS` is a whitespace separated list of words where each WORD describes one of style, variant, weight, or stretch, and `SIZE` is an decimal number (size in points).

• Available font families: `Normal, Sans, Serif and Monospace`.  
• Available styles: `Normal, Oblique, Italic`.  
• Available weights: `Ultra-Light, Light, Normal, Bold,Ultra-Bold, Heavy`.  
• Available variants: `Normal, Small-Caps`.  
• Available stretch styles: `Ultra-Condensed, Extra-Condensed, Condensed, Semi-Condensed, Normal, Semi-Expanded, Expanded, Extra-Expanded, Ultra-Expanded`.

Examples:

```
font sans bold 12
font normal 10
font monospace bold italic condensed 12
```

### Key Bindings

Custom keys and associated commands can be specified with the configuration file. An example entry is available [here](https://github.com/orhun/kermit/blob/master/.config/kermit.conf#L14) and entry format is shown below.

```
bind/bindx [KEY]~"[COMMAND]"
```

• bind: `Send command to the terminal.`  
• bindx: `Send command to the terminal and execute.`   

Examples:

```
bindx f~"df -h"
bind r~"rm -i "
bind p~"ps aux | grep "
bind k~"kill -9 "
```

### Padding

In order to change the padding of the terminal, create `~/.config/gtk-3.0/gtk.css` if it does not exist, specify the values there and restart the terminal.

```css
VteTerminal,
TerminalScreen,
vte-terminal {
    padding: 3px 2px 2px 1px;
}
```

The command below can be used to create the both configuration files.

```bash
curl https://raw.githubusercontent.com/orhun/kermit/master/.config/kermit.conf --output ~/.config/kermit.conf && \
  printf "VteTerminal,\nTerminalScreen,\nvte-terminal {\n\tpadding: 3px 2px 2px 1px;\n}\n" > ~/.config/gtk-3.0/gtk.css
```

## Screenshots

![Screenshot](https://user-images.githubusercontent.com/24392180/87167894-5a2e6000-c2d6-11ea-9c99-fa05cf56f40b.gif)

## TODO(s)

* URL handling

## License

GNU General Public License v3.0 only ([GPL-3.0-only](https://www.gnu.org/licenses/gpl.txt))

## Copyright

Copyright © 2019-2021, [Orhun Parmaksız](mailto:orhunparmaksiz@gmail.com)

# k3rmit [![Release](https://img.shields.io/github/release/orhun/k3rmit.svg?color=5B7C33&style=flat-square)](https://github.com/orhun/k3rmit/releases) [![AUR](https://img.shields.io/aur/version/k3rmit.svg?color=5B7C33&style=flat-square)](https://aur.archlinux.org/packages/k3rmit/)

### A VTE-based terminal emulator that aims to be simple, fast and effective.

![Kermit the Frog](https://user-images.githubusercontent.com/24392180/59636824-2af20180-915d-11e9-95dd-0a077ebc3cfa.gif)

[VTE](https://developer.gnome.org/vte/) is a [GTK](https://developer.gnome.org/gtk3/3.0/) widget that allows creating a virtual terminal emulator which is used in many emulators such as [GNOME Terminal](https://help.gnome.org/users/gnome-terminal/stable/), [ROXTerm](https://github.com/realh/roxterm) and [evilvte](http://www.calno.com/evilvte/). Although there's a lot of (VTE-based and other) terminal emulator options for GNU/Linux users, `k3rmit` tries to differ from them with its simplicity.   
The project inspired by [Vincent Bernat](https://vincent.bernat.ch/en)'s [article](https://vincent.bernat.ch/en/blog/2017-write-own-terminal) and also his [implementation](https://github.com/vincentbernat/vbeterm) of a custom VTE-based terminal. Also, [Rxvt](https://wiki.archlinux.org/index.php/Rxvt-unicode) and [termite](https://github.com/thestinger/termite)'s appearance are taken as an example.

## Installation

`k3rmit` terminal depends on [vte3](https://www.archlinux.org/packages/extra/x86_64/vte3/) and [gtk3](https://www.archlinux.org/packages/extra/x86_64/gtk3/) packages.

### • AUR
* [k3rmit](https://aur.archlinux.org/packages/k3rmit/)
* [k3rmit-git](https://aur.archlinux.org/packages/k3rmit-git/)

### • CMake

```
mkdir -p build && cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
sudo ldconfig
```

### • Make

```
make
sudo make install
```

### • GCC

```
cd src/
gcc -s -O3 -Wall -Wno-deprecated-declarations $(pkg-config --cflags vte-2.91) k3rmit.c -o\
 k3rmit.o $(pkg-config --libs vte-2.91)
```

## Features

* Use default shell (with `$SHELL` environment variable)
* Supports transparency with a composite manager (such as [compton](https://github.com/chjj/compton))
* Tab support
* Customizable

## Arguments

```
k3rmit [-h] [-v] [-d] [-c config] [-t title] [-e command]
-h, show help message and exit
-v, show version
-d, enable debug messages
-c, configuration file to read
-t, title to set in terminal
-e, command to execute in terminal
```

## Key Bindings

| Key                         | Action                         |
|-----------------------------|--------------------------------|
| `ctrl-alt-[c]`              | copy to clipboard              |
| `ctrl-alt-[v]`              | paste from clipboard           |
| `ctrl-alt-[t]`              | open a new tab                 |
| `ctrl-alt-[r]`              | reload configuration file      |
| `ctrl-alt-[q]`              | exit the terminal              |
| `ctrl-alt-[k][up]`          | increase font size             |
| `ctrl-alt-[j][down]`        | decrease font size             |
| `ctrl-alt-[=]`              | reset font size to default     |
| `ctrl-alt-[return]`         | open a new tab                 |
| `ctrl-alt-[num]`            | switch to the tab number [num] |
| `ctrl-alt-[l][right][pgup]` | switch to the next tab         |
| `ctrl-alt-[h][left][pgdn]`  | switch to the previous tab     |
| `ctrl-alt-[w][backspace]`   | close the selected tab         |

• Key bindings (`ctrl-alt`) can be set to standard `ctrl-shift` with the config file.  

• Default key bindings (`ctrl-alt`) can conflict with your desktop environments window shortcuts. (especially on Linux Mint) In order to solve this issue, key bindings can be changed to `ctrl-shift`.

## Customization

### Config File

`k3rmit` looks for configuration file in `~/.config/k3rmit.conf`  
The default configuration file is available [here](https://github.com/orhun/k3rmit/blob/master/.config/k3rmit.conf).  
Most of the settings can be changed via the config file including font, opacity and colors.

### Theme

Terminal theme can be changed with either editing the config file manually or using the [base16](https://github.com/chriskempson/base16) color schemes in [orhun/base16-k3rmit](https://github.com/orhun/base16-k3rmit) repository.

### Font

`k3rmit` uses a [PangoFontDescription](https://developer.gnome.org/pygtk/stable/class-pangofontdescription.html) which retrieved from the `k3rmit.conf` for changing the font family, style and size. The configuration entry format of the font and some examples are shown below and the default value is `monospace 9`.

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

Custom keys and associated commands can be specified with the configuration file. An example entry is available [here](https://github.com/orhun/k3rmit/blob/master/.config/k3rmit.conf#L14) and entry format is shown below.

```
bind/bindx [KEY]~"[COMMAND]"
```

• bind: `Send command to the terminal.`  
• bindx: `Send command to the terminal and execute.`   
• Key bindings must be uppercase if the action key is `ctrl-shift`.   
• Commands that contain "`~`" might have cause error when parsing the configuration file.

Examples:
```
bindx l~"ls -l"
bindx f~"df -h"
bind r~"rm -i "
bind p~"ps aux | grep "
bind k~"kill -9 "
```

### Padding

In order to change the padding of the terminal, create `~/.config/gtk-3.0/gtk.css` if does not exist and specify the values there and restart the terminal.

```
VteTerminal,
TerminalScreen,
vte-terminal {
    padding: 3px 2px 2px 1px;
}
```

The command below can be used to create both configuration files.

```
curl https://raw.githubusercontent.com/orhun/k3rmit/master/.config/k3rmit.conf --output ~/.config/k3rmit.conf && \
printf "VteTerminal,\nTerminalScreen,\nvte-terminal {\n\tpadding: 3px 2px 2px 1px;\n}\n" > ~/.config/gtk-3.0/gtk.css
```

## Screenshots

![Screenshot I](https://user-images.githubusercontent.com/24392180/66824998-10386980-ef52-11e9-92c0-7510338b71b7.gif)

![Screenshot II](https://user-images.githubusercontent.com/24392180/66824530-242f9b80-ef51-11e9-8c07-76b5c691e97c.png)

![Screenshot III](https://user-images.githubusercontent.com/24392180/59703686-1a946200-9203-11e9-8043-e58dcc9edc64.png)

## TODO(s)

• URL handling

## License

GNU General Public License ([v3](https://www.gnu.org/licenses/gpl.txt))

## Copyright

Copyright (c) 2019-2020, [orhun](https://www.github.com/orhun)
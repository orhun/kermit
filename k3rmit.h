/*
 * k3rmit, Terminal emulator that aims to be simple, fast and effective.
 * Copyright (C) 2019 by KeyLo99 <https://www.github.com/KeyLo99>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vte/vte.h>
#define TERM_NAME "k3rmit"
#define TERM_VERSION "1.0"
#define TERM_FONT "Monospace"
#define TERM_FONT_DEFAULT_SIZE 9
#define TERM_LOCALE "en_US.UTF-8"
#define TERM_OPACITY 0.95
#define TERM_WORD_CHARS "-./?%&#_=+@~"
#define TERM_FOREGROUND 0xffffff
#define TERM_CONFIG_LENGTH 64
#define TERM_CONFIG_DIR "/.config/"
#define TERM_ATTR_OFF "\x1b[0m"
#define TERM_ATTR_BOLD "\x1b[1m"
#define TERM_ATTR_COLOR "\x1b[34m"
#define TERM_ATTR_DEFAULT "\x1b[39m"

static int parseSettings();
static int configureTerm();
static int setTermFont(int fontSize);
static gboolean termOnKeyPress(GtkWidget *widget, GdkEventKey *key, gpointer gptr);
static gboolean termOnTitleChanged(GtkWidget *term, gpointer gptr);

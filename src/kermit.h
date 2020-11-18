/*
 * kermit, a VTE-based, simple and froggy terminal emulator.
 * Copyright (C) 2019-2020 by orhun <https://www.github.com/orhun>
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
#define TERM_NAME "kermit"
#define TERM_VERSION "3.1"
#define TERM_FONT "Monospace"
#define TERM_FONT_DEFAULT_SIZE 9
#define TERM_LOCALE "en_US.UTF-8"
#define TERM_OPACITY 0.96
#define TERM_WORD_CHARS "-./?%&#_=+@~"
#define TERM_BACKGROUND 0x000000
#define TERM_FOREGROUND 0xffffff
#define TERM_BOLD_COLOR 0xffffff
#define TERM_CURSOR_COLOR 0xffffff
#define TERM_CURSOR_FG 0xffffff
#define TERM_PALETTE_SIZE 256
#define TERM_CONFIG_LENGTH 64
#define TERM_CONFIG_DIR "/.config/"
#define TERM_ATTR_OFF "\x1b[0m"
#define TERM_ATTR_BOLD "\x1b[1m"
#define TERM_ATTR_COLOR "\x1b[34m"
#define TERM_ATTR_DEFAULT "\x1b[39m"

static GtkWidget *getTerm();
static void parseSettings();
static int configureTerm(GtkWidget *term);
static int setTermFont(GtkWidget *term, int fontSize);
static gboolean termOnChildExit(VteTerminal *term,
                                gint status, gpointer userData);
static gboolean termOnKeyPress(GtkWidget *widget,
                               GdkEventKey *key, gpointer gptr);
static gboolean termOnTitleChanged(GtkWidget *term,
                                   gpointer gptr);
static gboolean termOnResize(GtkWidget *widget,
                             GtkAllocation *allocation,
                             gpointer userData);

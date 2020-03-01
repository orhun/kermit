/*
 * k3rmit, a VTE-based terminal emulator that aims to be simple, fast and effective.
 * Copyright (C) 2019 by orhun <https://www.github.com/orhun>
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

#include "k3rmit.h"
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <vte/vte.h>

#define UNUSED(x) (void)(x)
#define CLR_R(x) (((x) & 0xff0000) >> 16)
#define CLR_G(x) (((x) & 0x00ff00) >>  8)
#define CLR_B(x) (((x) & 0x0000ff) >>  0)
#define CLR_16(x) ((double)(x) / 0xff)
#define CLR_GDK(x, a) (const GdkRGBA) { .red = CLR_16(CLR_R(x)), \
                                    .green = CLR_16(CLR_G(x)), \
                                    .blue = CLR_16(CLR_B(x)), \
                                    .alpha = a }

static GtkWidget *window;                             /* Main window widget */
static GtkWidget *paned;                              /* Paned widged for the tab feature */
static GtkWidget *notebook;                           /* Notebook widget for the tab feature */
static GtkWidget *tabLabel;                           /* Label widget for the tab feature */
static PangoFontDescription *fontDesc;                /* Description for the terminal font */
static FILE *configFile;                              /* Terminal configuration file */
static float termOpacity   = TERM_OPACITY;            /* Default opacity value */
static int defaultFontSize = TERM_FONT_DEFAULT_SIZE;  /* Terminal font size */
static int termBackground  = TERM_BACKGROUND;         /* Background color */
static int termForeground  = TERM_FOREGROUND;         /* Foreground color */
static int termBoldColor   = TERM_BOLD_COLOR;         /* Foreground bold color */
static int termCursorColor = TERM_CURSOR_COLOR;       /* Cursor color */
static int termCursorFg    = TERM_CURSOR_FG;          /* Cursor foreground color */
static int currentFontSize;                           /* Necessary for changing font size */
static int keyState;                                  /* State of key press events */
static int actionKey   = GDK_MOD1_MASK;               /* Key to check on press */
static int tabPosition = 0;                           /* Tab position (0/1 -> bottom/top) */
static int keyCount    = 0;                           /* Count of custom binding keys */
static int opt;                                       /* Argument parsing option */
static char *termFont      = TERM_FONT;               /* Default terminal font */
static char *termLocale    = TERM_LOCALE;             /* Terminal locale (numeric) */
static char *termWordChars = TERM_WORD_CHARS;         /* Word characters exceptions */
static char *termTitle;                               /* Title to set in terminal (-t) */
static char *wordChars;                               /* Variables for parsing the config */
static char *fontSize;
static char *colorIndex;
static char *configFileName;                          /* Configuration file name */
static char *termCommand;                             /* Command to execute in terminal (-e) */
static char *tabLabelText;                            /* The label text for showing the tabs situation */
static gchar **envp;                                  /* Variables for starting the terminal */
static gchar **command;
static gboolean defaultConfigFile = TRUE;             /* Boolean value for -c argument */
static gboolean debugMessages     = FALSE;            /* Boolean value for -d argument */
static gboolean closeTab          = FALSE;            /* Close the tab on child-exited signal */
static va_list vargs;                                 /* Hold information about variable arguments */
typedef struct KeyBindings {                          /* Key bindings struct */
	char *key;
    char *cmd;
} Bindings;
static Bindings keyBindings[TERM_CONFIG_LENGTH];      /* Array for custom key bindings */
static GdkRGBA termPalette[TERM_PALETTE_SIZE] = {
        CLR_GDK(0x3f3f3f, 0), CLR_GDK(0xcf0000, 0),
        CLR_GDK(0x33ff00, 0), CLR_GDK(0xf3f828, 0),
        CLR_GDK(0x0300ff, 0), CLR_GDK(0xcc00ff, 0),
        CLR_GDK(0x0300ff, 0), CLR_GDK(0xdcdccc, 0),
        CLR_GDK(0x808080, 0), CLR_GDK(0xcf0000, 0),
        CLR_GDK(0x33ff00, 0), CLR_GDK(0x6b6b6b, 0),
        CLR_GDK(0x0066ff, 0), CLR_GDK(0xcc00ff, 0),
        CLR_GDK(0x34e2e2, 0), CLR_GDK(0xdcdccc, 0)
    };

/*!
 * Print log (debug) message with format specifiers.
 *
 * \param format string and format specifiers for vfprintf function  
 * \return 0 on success
 */
static int printLog(char *format, ...) {
    if (!debugMessages) {
        return 0;
    }
	fprintf(stderr, "%s[ %sdebug%s ] ", 
        TERM_ATTR_BOLD,     /* Bold on */
        TERM_ATTR_COLOR,    /* Light blue */
        TERM_ATTR_DEFAULT); /* Default color */
    /* Format the string & print */
  	va_start(vargs, format);
  	vfprintf(stderr, format, vargs);
  	va_end(vargs);
    /* All attributes off */
    fprintf(stderr, "%s", TERM_ATTR_OFF);
    return 0;
}

/*!
 * Set signals for terminal.
 *
 * \param terminal
 * \return 0 on success
 */
static int connectSignals(GtkWidget* terminal) {
    g_signal_connect(terminal, "child-exited", G_CALLBACK(termOnChildExit), NULL);
    g_signal_connect(terminal, "key-press-event", G_CALLBACK(termOnKeyPress), NULL);
    g_signal_connect(terminal, "window-title-changed", G_CALLBACK(termOnTitleChanged), 
                        GTK_WINDOW(window));
    return 0;
}

/*!
 * Handle terminal exit.
 *
 * \param terminal
 * \param status
 * \param userData
 * \return TRUE on exit
 */
static gboolean termOnChildExit(VteTerminal *terminal, gint status, 
        gpointer userData) {
    /* 'child-exited' signal is emitted on both terminal exit
     * and (notebook) page deletion. Use closeTab variable
     * to solve this issue. Also closes the current tab on exit.
     */
    if(!closeTab) {
        /* Close the current tab */
        if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) != 1) {
            gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), 
                gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
            gtk_widget_queue_draw(GTK_WIDGET(notebook));
        /* Exit the terminal */
        } else {
            gtk_main_quit();
        }
    /* Close tab */
    } else {
        closeTab = FALSE;
    }
    return TRUE;
}

/*!
 * Handle terminal key press events.
 * 
 * \param terminal
 * \param event (key press or release)
 * \param userData
 * \return FALSE on normal press & TRUE on custom actions
 */
static gboolean termOnKeyPress(GtkWidget *terminal, GdkEventKey *event, 
                gpointer userData) {
    /* Unused user data */
    UNUSED(userData);
    /* Check for CTRL, ALT and SHIFT keys */
    keyState = event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK);
    /* CTRL + binding + key */
    if(keyState == (actionKey | GDK_CONTROL_MASK)) {
        if(atoi(gdk_keyval_name(event->keyval)) != 0) {
           gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),
                atoi(gdk_keyval_name(event->keyval))-1);
           return TRUE;
        }
        switch (event->keyval) {
            /* Copy & Paste */
            case GDK_KEY_C: /* Fallthrough */
            case GDK_KEY_c:
                vte_terminal_copy_clipboard_format(VTE_TERMINAL(terminal), 
                    VTE_FORMAT_TEXT);
                return TRUE;
            case GDK_KEY_V: /* Fallthrough */
            case GDK_KEY_v:
                vte_terminal_paste_clipboard(VTE_TERMINAL(terminal));
                return TRUE;
            /* Reload configuration file */ 
            case GDK_KEY_R: /* Fallthrough */
            case GDK_KEY_r:
                printLog("Reloading configuration file...\n");
                if(defaultConfigFile)
                    configFileName = NULL;
                parseSettings();
                configureTerm(terminal);
                return TRUE;
            /* Open new tab */
            case GDK_KEY_T: /* Fallthrough */
            case GDK_KEY_t:
                gtk_notebook_append_page(GTK_NOTEBOOK(notebook), getTerm(), NULL);
                gtk_widget_show_all(window);
                return TRUE;
            /* Exit */
            case GDK_KEY_Q: /* Fallthrough */
            case GDK_KEY_q:
                gtk_main_quit();
                return TRUE;
            /* Change font size */
            case GDK_KEY_K: /* Fallthrough */
            case GDK_KEY_k: /* Fallthrough */
            case GDK_KEY_Up:
                setTermFont(terminal, currentFontSize + 1);
                return TRUE;
            case GDK_KEY_J: /* Fallthrough */
            case GDK_KEY_j: /* Fallthrough */
            case GDK_KEY_Down:
                setTermFont(terminal, currentFontSize - 1);
                return TRUE;
            case GDK_KEY_equal:
			    setTermFont(terminal, defaultFontSize);
			    return TRUE;
            /* Open new tab */
            case GDK_KEY_Return:
                gtk_notebook_append_page(GTK_NOTEBOOK(notebook), getTerm(), NULL);
                gtk_widget_show_all(window);
                return TRUE;
            /* Switch to the next tab */
            case GDK_KEY_L:          /* Fallthrough */
            case GDK_KEY_l:          /* Fallthrough */
            case GDK_KEY_KP_Page_Up: /* Fallthrough */
            case GDK_KEY_Right:
                gtk_notebook_next_page(GTK_NOTEBOOK(notebook));
                return TRUE;
            /* Switch to the previous tab */
            case GDK_KEY_H:              /* Fallthrough */
            case GDK_KEY_h:              /* Fallthrough */
            case GDK_KEY_KP_Page_Down:   /* Fallthrough */
            case GDK_KEY_Left:
                gtk_notebook_prev_page(GTK_NOTEBOOK(notebook));
                return TRUE;
            /* Close the current tab */
            case GDK_KEY_W:         /* Fallthrough */
            case GDK_KEY_w:         /* Fallthrough */
            case GDK_KEY_BackSpace: /* Fallthrough */
                if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) == 1)
                    return TRUE;    
                closeTab = TRUE;
                gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), 
                    gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
                gtk_widget_queue_draw(GTK_WIDGET(notebook));
                return TRUE;
            default:
                for(int i = 0; i < keyCount; i++) {
                    if(!strcmp(gdk_keyval_name(event->keyval), keyBindings[i].key)) {
                        vte_terminal_feed_child(VTE_TERMINAL(terminal),
                            keyBindings[i].cmd, -1);
                        return TRUE;
                    }
                }
        }
	}
	return FALSE;
}

/*!
 * Set the terminal title on changes.
 *
 * \param terminal
 * \param userData
 * \return TRUE on title change
 */
static gboolean termOnTitleChanged(GtkWidget *terminal, gpointer userData) {
	GtkWindow *window = userData;
	if (termTitle == NULL) gtk_window_set_title(window,
            vte_terminal_get_window_title(VTE_TERMINAL(terminal))?:TERM_NAME);
    else gtk_window_set_title(window, termTitle);
	return TRUE;
}

/*!
 * Set the divider position using current window size.
 *
 * \param widget
 * \param allocation
 * \param userData
 * \return TRUE on size change
 */
static gboolean termOnResize(GtkWidget *widget, GtkAllocation *allocation, 
        gpointer userData) {
    if (tabPosition == 1)
        allocation->height = 0;
    if (GTK_PANED(userData) != NULL)
        gtk_paned_set_position(GTK_PANED(userData), allocation->height-20);
    return TRUE;
}

/*!
 * Switch to last page when new tab added.
 *
 * \param notebook
 * \param child
 * \param pageNum
 * \param userData
 * \return TRUE on tab addition
 */
static gboolean termTabOnAdd(GtkNotebook *notebook, GtkWidget *child, 
        guint pageNum, gpointer userData) {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), pageNum);
    return TRUE;
}

/*!
 * The switch event for terminal tabs.
 *
 * \param notebook
 * \param page
 * \param pageNum
 * \param userData
 * \return TRUE on switch
 */
static gboolean termTabOnSwitch(GtkNotebook *notebook, GtkWidget *page, 
        guint pageNum, gpointer userData) {
    /* Destroy tabs label if there's not more than one tabs */
    if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) == 1) {
        if(tabLabel != NULL)
            gtk_widget_destroy(tabLabel);
        return TRUE;
    /* Add tabs label to paned if it doesn't exist */
    } else if(gtk_paned_get_child1(GTK_PANED(paned)) == NULL ||
        gtk_paned_get_child2(GTK_PANED(paned)) == NULL) {
        tabLabel = gtk_label_new(NULL);
        gtk_label_set_xalign(GTK_LABEL(tabLabel), 0);
        if (tabPosition == 0)
            gtk_paned_add2(GTK_PANED(paned), tabLabel);
        else
            gtk_paned_add1(GTK_PANED(paned), tabLabel);
    }
    /* Same font as terminal but smaller */
    gchar *fontStr = g_strconcat(termFont, " ",
        g_strdup_printf("%d", defaultFontSize-1), NULL);
    /* Prepare the label text (use different color for current tab) */
    tabLabelText = g_markup_printf_escaped(
        "<span font='\%s' foreground='#\%02X\%02X\%02X'>", 
        fontStr, 
        (int)(termPalette[4].red*255), 
        (int)(termPalette[4].green*255), 
        (int)(termPalette[4].blue*255));
    for (int i = 0; i < gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)); i++) {
        if (i == pageNum)
            tabLabelText = g_strconcat(tabLabelText, 
                g_strdup_printf("<span foreground='#%x'> %d </span>", 
                termForeground, i+1), NULL);
        else
            tabLabelText = g_strconcat(tabLabelText, 
                g_strdup_printf(" %d ", i+1), NULL);
    }
    tabLabelText = g_strconcat(tabLabelText, "~</span>", NULL);    
    /* Set the label text with markup */
    gtk_label_set_markup(GTK_LABEL(tabLabel), tabLabelText);
    g_free(fontStr);
    return TRUE;
}


/*!
 * Set terminal font with given size.
 *
 * \param fontSize
 * \return 0 on success
 */
static int setTermFont(GtkWidget* terminal, int fontSize) {
    gchar *fontStr = g_strconcat(termFont, " ", 
        g_strdup_printf("%d", fontSize), NULL);
    if ((fontDesc = pango_font_description_from_string(fontStr)) != NULL) {
	    vte_terminal_set_font(VTE_TERMINAL(terminal), fontDesc);
        currentFontSize = fontSize;
	    pango_font_description_free(fontDesc);
        g_free(fontStr);
    }
    return 0;
}

/*!
 * Configure the terminal.
 *
 * \param terminal
 * \return 0 on success
 */
static int configureTerm(GtkWidget* terminal) {
    /* Set numeric locale */
    setlocale(LC_NUMERIC, termLocale);
    /* Hide the mouse cursor when typing */
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(terminal), TRUE);
    /* Scroll issues */
    vte_terminal_set_scroll_on_output(VTE_TERMINAL(terminal), FALSE);
    vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(terminal), TRUE);
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), -1);
    /* Rewrap the content when terminal size changed */
    vte_terminal_set_rewrap_on_resize(VTE_TERMINAL(terminal), TRUE);
    /* Disable audible bell */
    vte_terminal_set_audible_bell(VTE_TERMINAL(terminal), FALSE);
    /* Enable bold text */
    vte_terminal_set_allow_bold(VTE_TERMINAL(terminal), TRUE);
    /* Allow hyperlinks */
    vte_terminal_set_allow_hyperlink(VTE_TERMINAL(terminal), TRUE);
    /* Set char exceptions */
    vte_terminal_set_word_char_exceptions(VTE_TERMINAL(terminal),
	    termWordChars);
    /* Zuckerberg feature */
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(terminal),
        VTE_CURSOR_BLINK_OFF);
    /* Set cursor colors */
    vte_terminal_set_color_cursor(VTE_TERMINAL(terminal),
        &CLR_GDK(termCursorColor, 0));
    vte_terminal_set_color_cursor_foreground(VTE_TERMINAL(terminal),
        &CLR_GDK(termCursorFg, 0));
    /* Set the terminal colors and font */
    vte_terminal_set_colors(VTE_TERMINAL(terminal),
        &CLR_GDK(termForeground, 0),            /* Foreground */
        &CLR_GDK(termBackground, termOpacity),  /* Background */
        termPalette ,                           /* Palette */
        sizeof(termPalette)/sizeof(GdkRGBA));
    vte_terminal_set_color_bold(VTE_TERMINAL(terminal),
        &CLR_GDK(termBoldColor, 0));
    setTermFont(terminal, defaultFontSize);
    return 0;
}

/*!
 * Async callback for terminal state.
 *
 * \param terminal
 * \param pid (Process ID)
 * \param error
 * \param userData
 */
static void termStateCallback(VteTerminal *terminal, GPid pid,
            GError *error, gpointer userData) {
    if (error == NULL) {
        printLog("%s started. (PID: %d)\n", TERM_NAME, pid);
    } else {
        printLog("An error occurred: %s\n", error->message);
        g_clear_error(&error);
    }
    UNUSED(userData);
    UNUSED(terminal);
}

/*!
 * Create a new terminal widget with shell.
 *
 * \return terminal
 */
static GtkWidget* getTerm() {
    /* Create a terminal widget */
    GtkWidget *terminal = vte_terminal_new();
    /* Terminal configuration */
    connectSignals(terminal);
    configureTerm(terminal);
    /* Start a new shell */
    envp = g_get_environ();
    command = (gchar *[]) { g_strdup(g_environ_getenv(envp, "SHELL")), NULL };
    if(termCommand != NULL)
        command = (gchar *[]) { g_strdup(g_environ_getenv(envp, "SHELL")),
            "-c", termCommand , NULL };
    printLog("shell: %s\n", *command);
    g_strfreev(envp);
    /* Spawn terminal asynchronously */
    vte_terminal_spawn_async(VTE_TERMINAL(terminal), 
        VTE_PTY_DEFAULT,   /* pty flag */
        NULL,              /* working directory */
        command,           /* argv */
        NULL,              /* environment variables */
        G_SPAWN_DEFAULT,   /* spawn flag */
        NULL,              /* child setup function */
        NULL,              /* child setup data */
        NULL,              /* child setup data destroy */
        -1,                /* timeout */
        NULL,              /* cancellable */
        termStateCallback, /* async callback */
        NULL);             /* callback data */
    /* Show the terminal widget */
    gtk_widget_show(terminal);
    return terminal;
}

/*!
 * Initialize and start the terminal.
 *
 * \return 0 on success
 */
static int startTerm() {
    /* Create & configure the window widget */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), TERM_NAME);
    gtk_widget_set_visual(window, /* Alpha channel for transparency */
        gdk_screen_get_rgba_visual(gtk_widget_get_screen(window)));
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, 
        &CLR_GDK(termBackground, termOpacity));
    /* Create & configure the paned widget */
    paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_wide_handle(GTK_PANED(paned), FALSE);
    /* Create & configure the notebook widget */
    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_notebook_popup_disable(GTK_NOTEBOOK(notebook));
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    /* Connect signals of window and notebook for tab feature */
    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(window, "size-allocate", G_CALLBACK(termOnResize), paned);
    g_signal_connect(notebook, "page-added", G_CALLBACK(termTabOnAdd), NULL);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(termTabOnSwitch), NULL);
    /* Add terminal to notebook as first tab */
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), getTerm(), NULL);
    /* Add notebook to paned */
    if (tabPosition == 0)
        gtk_paned_add1(GTK_PANED(paned), notebook);
    else
        gtk_paned_add2(GTK_PANED(paned), notebook);
    /* Add paned to main window */
    gtk_container_add(GTK_CONTAINER(window), paned);
    /* Show all widgets with childs and run the main loop */
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

/*!
 * Read settings from configuration file and apply.
 */
static void parseSettings() {
    char buf[TERM_CONFIG_LENGTH], 
        option[TERM_CONFIG_LENGTH], 
        value[TERM_CONFIG_LENGTH];
    if(configFileName == NULL)
        configFileName = g_strconcat(getenv("HOME"), 
                TERM_CONFIG_DIR, TERM_NAME, ".conf", NULL);
    else
        defaultConfigFile = FALSE;
    configFile = fopen(configFileName, "r");
    if(configFile == NULL) {
        printLog("config file not found. (%s)\n", configFileName);
        return;
    }
    while(fgets(buf, TERM_CONFIG_LENGTH, configFile)) {
        /* Skip lines starting with '#' and invalid lines */
        if (buf[0] == '#' || strlen(buf) < 4)
            continue;
        /* Get option and value from line */
        sscanf(buf, "%s %s\n", option, value);
        /* Locale */
        if(!strncmp(option, "locale", strlen(option))) {
             termLocale = g_strdup(value);
        /* Word chars */
        } else if(!strncmp(option, "char", strlen(option))) {
            /* Remove '"' from word chars */
            wordChars = g_strdup(value);
            wordChars[strlen(wordChars)-1] = 0;
            termWordChars = wordChars+1;
        /* Action key */
        } else if(!strncmp(option, "key", strlen(option))) {
            if(!strncmp(value, "alt", strlen(value)))
                actionKey = GDK_MOD1_MASK;
            else
                actionKey = GDK_SHIFT_MASK;
        /* Key bindings */
        } else if(!strncmp(option, "bind", strlen(option)-1)) {
            /* Parse the line again for command */
            sscanf(buf, "%s %[^\n]\n", option, value);
            /* Split the line and get last element */
            char *cmd = strrchr(value, '~');
            if (cmd != NULL) {
                /* Trim and add to the commands */
                cmd[strlen(cmd)-1] = 0;
                /* Execute option is provided */
                if (!strcmp(option, "bindx"))
                /* Append carriage return to command */
                    keyBindings[keyCount].cmd = 
                        g_strconcat(g_strdup(cmd+2), "\r", NULL);
                else
                    keyBindings[keyCount].cmd = g_strdup(cmd+2);
                /* Add key binding to the keys */
                *cmd = 0;
                keyBindings[keyCount].key = g_strdup(value);
                /* Increment the keys count */
                keyCount++;
            }
        /* Tab position */
        } else if(!strncmp(option, "tab", strlen(option))) {
            if(!strncmp(value, "bottom", strlen(value)))
                tabPosition = 0;
            else
                tabPosition = 1;
        /* Terminal font */
        } else if(!strncmp(option, "font", strlen(option))) {
            /* Parse the line again for font size */ 
            sscanf(buf, "%s %[^\n]\n", option, value);
            /* Split the line and get last element */
            fontSize = strrchr(value, ' ');
            if (fontSize != NULL) {
                /* Trim and set the font size */
                defaultFontSize = atoi(fontSize+1);
                /* Get the font information excluding font size */
                *fontSize = 0;
                termFont = g_strdup(value);
            }
        /* Opacity value */
        } else if(!strncmp(option, "opacity", strlen(option))) {
            termOpacity = atof(value);
        /* Cursor colors */
        } else if(!strncmp(option, "cursor", strlen(option))) {
            termCursorColor = (int)strtol(value, NULL, 16);
        } else if(!strncmp(option, "cursor_foreground", strlen(option))) {
            termCursorFg = (int)strtol(value, NULL, 16);
        /* Foreground color */
        } else if(!strncmp(option, "foreground", strlen(option))) {
            termForeground = (int)strtol(value, NULL, 16);
        /* Bold color */
        } else if(!strncmp(option, "foreground_bold", strlen(option))) {
            termBoldColor = (int)strtol(value, NULL, 16);
        /* Background color */
        } else if(!strncmp(option, "background", strlen(option))) {
            termBackground = (int)strtol(value, NULL, 16);
        /* Color palette */
        } else if(!strncmp(option, "color", strlen(option)-2)) {
            /* Get the color index */
            colorIndex = strrchr(option, 'r');
            if (colorIndex != NULL) {
                /* Set the color in palette */
                termPalette[atoi(colorIndex + 1) % TERM_PALETTE_SIZE] =
                    CLR_GDK((int)strtol(value, NULL, 16), 0);
            }
        }
    }
    fclose(configFile);
    if(defaultConfigFile)
        g_free(configFileName);
}

/*!
 * Parse command line arguments.
 *
 * \param argc (argument count)
 * \param argv (argument vector)
 * \return 1 on exit
 */
static int parseArgs(int argc, char **argv) {
    while ((opt = getopt(argc, argv, ":c:e:t:vdh")) != -1) {
        switch (opt) {
            case 'c':
                /* Configuration file name to read */
                configFileName = optarg;
                break;
            case 'e':
                /* Command to execute in terminal */
                termCommand = optarg;
                break;
            case 't':
                /* Title to set in terminal */
                termTitle = optarg;
            case 'd':
                /* Activate debug messages */
                debugMessages = TRUE;
                break;
            case 'v':
                /* Show version information */
                fprintf(stderr, 
                    "%s [ %sk3rmit%s ] ~ v%s \n"
                    "   (+)(+)   ~ VTE-based\r\n"
                    "  /      \\  ~ simple\r\n"
                    "  \\ -==- /  ~ fast\r\n"
                    "   \\    /   ~ effective\r\n"
                    "  <\\/\\/\\/>  ~ github.com/orhun/k3rmit\r\n"
                    "  /      \\  ~ gplv3%s\n",
                    TERM_ATTR_BOLD, TERM_ATTR_COLOR,
                    TERM_ATTR_DEFAULT, TERM_VERSION,
                    TERM_ATTR_OFF);
                return 1;
            case 'h': /* Fallthrough */
            case '?':
                /* Show help message */
                fprintf(stderr, "%s[ %susage%s ] %s [-h] "
                "[-v] [-d] [-c config] [-t title] [-e command]%s\n",
                    TERM_ATTR_BOLD,
                    TERM_ATTR_COLOR,  
                    TERM_ATTR_DEFAULT,
                    TERM_NAME,
                    TERM_ATTR_OFF);
                return 1;
            case ':':
                /* Show debug message on missing argument */
                debugMessages = TRUE;
                printLog("Option requires an argument.\n");
                return 1;
        }
    }
    return 0;
}

/*!
 * Entry-point
 */
int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    if(parseArgs(argc, argv))
        return 0;
    /* Parse settings if configuration file exists */
    parseSettings();
    /* Initialize GTK and start the terminal */
    gtk_init(&argc, &argv);
    startTerm();
    return 0;
}

/*
 * k3rmit, a VTE-based terminal emulator that aims to be simple, fast and effective.
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

#include "k3rmit.h"
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <vte/vte.h>

#define UNUSED(x) (void)(x)
#define CLR_R(x)   (((x) & 0xff0000) >> 16)
#define CLR_G(x)   (((x) & 0x00ff00) >>  8)
#define CLR_B(x)   (((x) & 0x0000ff) >>  0)
#define CLR_16(x)  ((double)(x) / 0xff)
#define CLR_GDK(x, a) (const GdkRGBA){ .red = CLR_16(CLR_R(x)), \
                                    .green = CLR_16(CLR_G(x)), \
                                    .blue = CLR_16(CLR_B(x)), \
                                    .alpha = a }

static GtkWidget *window, *terminal; /* Window and terminal widgets */
static PangoFontDescription *fontDesc; /* Description for the terminal font */
static FILE *configFile; /* Terminal configuration file */
static float termOpacity = TERM_OPACITY; /* Default opacity value */
static int defaultFontSize = TERM_FONT_DEFAULT_SIZE, /* Font size */
        termBackground = TERM_BACKGROUND, /* Background color */
        termForeground = TERM_FOREGROUND, /* Foreground color */
        currentFontSize, /* Necessary for changing font size */
        keyState, /* State of key press events */
        actionKey = GDK_MOD1_MASK, /* Key to check on press */
        opt; /* Argument parsing option */ 
static char *termFont = TERM_FONT, /* Default terminal font */
        *termLocale = TERM_LOCALE, /* Terminal locale (numeric) */
        *termWordChars = TERM_WORD_CHARS, /* Word characters exceptions */
        *wordChars, *fontSize, *colorIndex, /* Variables for parsing the config */
        *configFileName, /* Configuration file name */
        *termCommand; /* Command to execute in terminal (-e) */
static gchar **envp, **command; /* Variables for starting the terminal */
static gboolean defaultConfigFile = TRUE, /* Boolean value for -c argument */
        debugMessages = FALSE; /* Boolean value for -d argument */
static va_list vargs; /*! Hold information about variable arguments */
static GdkRGBA termPalette[] = {             
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
static int printLog(char *format, ...){
    if (!debugMessages){
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
 * Set signals for terminal and window.
 *
 * \return 0 on success
 */
static int connectSignals(){
    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
    g_signal_connect(terminal, "key-press-event", G_CALLBACK(termOnKeyPress), 
                        GTK_WINDOW(window));
    g_signal_connect(terminal, "window-title-changed", G_CALLBACK(termOnTitleChanged), 
                        GTK_WINDOW(window));
    return 0;
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
                gpointer userData){
    /* Unused user data */
    UNUSED(userData);
    /* Check for CTRL, ALT and SHIFT keys */
    keyState = event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK);
    /* CTRL + binding + key */
    if(keyState == (actionKey | GDK_CONTROL_MASK)){
        switch (event->keyval) {
            /* Copy & Paste */
            case GDK_KEY_C:
            case GDK_KEY_c:
                vte_terminal_copy_clipboard_format(VTE_TERMINAL(terminal), 
                    VTE_FORMAT_TEXT);
                return TRUE;
            case GDK_KEY_V:
            case GDK_KEY_v:
                vte_terminal_paste_clipboard(VTE_TERMINAL(terminal));
                return TRUE;
            /* Reload configuration file */
            case GDK_KEY_R:
            case GDK_KEY_r:
                printLog("Reloading configuration file...\n");
                if(defaultConfigFile)
                    configFileName = NULL;
                parseSettings();
                configureTerm();
                return TRUE;
        }
    /* CTRL + key */
	}else if (keyState == GDK_CONTROL_MASK){
        switch (event->keyval) {
            /* Change font size */
            case GDK_KEY_0:
            case GDK_KEY_1:
                setTermFont(currentFontSize + 1);
                return TRUE;
            case GDK_KEY_minus:
            case GDK_KEY_2:
                setTermFont(currentFontSize - 1);
                return TRUE;
            case GDK_KEY_equal:
			    setTermFont(defaultFontSize);
			    return TRUE;
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
static gboolean termOnTitleChanged(GtkWidget *terminal, gpointer userData){
	GtkWindow *window = userData;
	gtk_window_set_title(window,
	    vte_terminal_get_window_title(VTE_TERMINAL(terminal))?:TERM_NAME);
	return TRUE;
}

/*!
 * Set terminal font with given size.
 *
 * \param fontSize
 * \return 0 on success
 */
static int setTermFont(int fontSize){
    gchar *fontStr = g_strconcat(termFont, " ", 
        g_strdup_printf("%d", fontSize), NULL);
    if ((fontDesc = pango_font_description_from_string(fontStr)) != NULL){
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
 * \return 0 on success
 */
static int configureTerm(){
    /* Set window title */
    gtk_window_set_title(GTK_WINDOW(window), TERM_NAME);
    /* Set numeric locale */
    setlocale(LC_NUMERIC, termLocale);
    /* Hide the mouse cursor when typing */
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(terminal), TRUE);
    /* Scroll issues */
    vte_terminal_set_scroll_on_output(VTE_TERMINAL(terminal), FALSE);
    vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(terminal), TRUE);
    /* Rewrap the content when terminal size changed */
    vte_terminal_set_rewrap_on_resize(VTE_TERMINAL(terminal), TRUE);
    /* Disable audible bell */
    vte_terminal_set_audible_bell(VTE_TERMINAL(terminal), FALSE);
    /* Enable bold text */
    vte_terminal_set_allow_bold(VTE_TERMINAL(terminal), TRUE);
    /* Allow hyperlinks */
    vte_terminal_set_allow_hyperlink(VTE_TERMINAL(terminal), TRUE);
    /* Zuckerberg feature */
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(terminal), 
        VTE_CURSOR_BLINK_OFF);
    /* Set char exceptions */
    vte_terminal_set_word_char_exceptions(VTE_TERMINAL(terminal),
	    termWordChars);
    /* Set the terminal colors and font */
    vte_terminal_set_colors(VTE_TERMINAL(terminal),
        &CLR_GDK(termForeground, 0),            /* Foreground */
        &CLR_GDK(termBackground, termOpacity),  /* Background */
        termPalette ,                           /* Palette */
        sizeof(termPalette)/sizeof(GdkRGBA)); 
    setTermFont(defaultFontSize);
    /* Create a window with alpha channel for transparency */
    gtk_widget_set_visual(window, 
        gdk_screen_get_rgba_visual(gtk_widget_get_screen(window)));
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
            GError *error, gpointer userData){
    if (error == NULL){
        printLog("%s started. (PID: %d)\n", TERM_NAME, pid);
    }else{
        printLog("An error occurred: %s\n", error->message);
        g_clear_error(&error);
    }
    UNUSED(userData);
    UNUSED(terminal);
}

/*!
 * Initialize and start the terminal.
 *
 * \return 0 on success
 */
static int startTerm(){
    terminal = vte_terminal_new();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    /* Terminal configuration */
    connectSignals();
    configureTerm();
    /* Start a new shell */
    envp = g_get_environ();
    command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), NULL };
    if(termCommand != NULL)
        command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), 
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
    /* Put widgets together and run the main loop */
    gtk_container_add(GTK_CONTAINER(window), terminal);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

/*!
 * Read settings from configuration file and apply.
 *
 * \return 0 on success
 */
static int parseSettings(){
    char buf[TERM_CONFIG_LENGTH], 
        option[TERM_CONFIG_LENGTH], 
        value[TERM_CONFIG_LENGTH];
    if(configFileName == NULL)
        configFileName = g_strconcat(getenv("HOME"), 
                TERM_CONFIG_DIR, TERM_NAME, ".conf", NULL);
    else
        defaultConfigFile = FALSE;
    configFile = fopen(configFileName, "r");
    if(configFile != NULL){
        while(!feof(configFile)){
            fgets(buf, TERM_CONFIG_LENGTH, configFile);
            /* Skip lines starting with '#' and invalid lines */
            if (buf[0] == '#' || strlen(buf) < 4)
                continue;
            /* Get option and value from line */
            sscanf(buf, "%s %s\n", option, value);
            /* Locale */
            if(!strncmp(option, "locale", strlen(option))){
                 termLocale = g_strdup(value);
            /* Word chars */
            }else if(!strncmp(option, "char", strlen(option))){
                /* Remove '"' from word chars */
                wordChars = g_strdup(value);
                wordChars[strlen(wordChars)-1] = 0;
                termWordChars = wordChars+1;
            /* Key bindings */
            }else if(!strncmp(option, "key", strlen(option))){
                if(!strncmp(value, "alt", strlen(value)))
                    actionKey = GDK_MOD1_MASK;
                else
                    actionKey = GDK_SHIFT_MASK;
            /* Terminal font */
            }else if(!strncmp(option, "font", strlen(option))){
                /* Parse the line again for font size */ 
                sscanf(buf, "%s %[^,]\n", option, value);
                /* Split the line and get last element */
                fontSize = strrchr(value, ' ');
                if (fontSize != NULL) {
                    /* Remove unnecessary chars from font size */
                    fontSize[strlen(fontSize)-1] = 0;
                    defaultFontSize = atoi(fontSize+1);
                    /* Get the font information excluding font size */
                    *fontSize = 0;
                    termFont = g_strdup(value);
                }
            /* Opacity value */
            }else if(!strncmp(option, "opacity", strlen(option))){
                termOpacity = atof(value);
            /* Foreground color */
            }else if(!strncmp(option, "foreground", strlen(option))){
                termForeground = (int)strtol(value, NULL, 16);
            /* Background color */
            }else if(!strncmp(option, "background", strlen(option))){
                termBackground = (int)strtol(value, NULL, 16);
            /* Color palette */
            }else if(!strncmp(option, "color", strlen(option)-2)){
                /* Get the color index */
                colorIndex = strrchr(option, 'r');
                if (colorIndex != NULL) {
                    /* Set the color in palette */
                    termPalette[atoi(colorIndex+1)] = 
                        CLR_GDK((int)strtol(value, NULL, 16), 0);
                }
            }
        }
        fclose(configFile);
    } else {
        printLog("config file not found. (%s)\n", configFileName);
    }
    if(defaultConfigFile)
        g_free(configFileName);
    return 0;
}

/*!
 * Parse command line arguments.
 *
 * \param argc (argument count)
 * \param argv (argument vector)
 * \return 1 on exit
 */
static int parseArgs(int argc, char **argv){
    while ((opt = getopt(argc, argv, ":c:e:vdh")) != -1) {
        switch (opt) {
            case 'c':
                /* Configuration file name to read */
                configFileName = optarg;
                break;
            case 'e':
                /* Command to execute in terminal */
                termCommand = optarg;
                break;
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
                    "  <\\/\\/\\/>  ~ github.com/keylo99/k3rmit\r\n"
                    "  /      \\  ~ gplv3%s\n",
                    TERM_ATTR_BOLD, TERM_ATTR_COLOR,
                    TERM_ATTR_DEFAULT, TERM_VERSION,
                    TERM_ATTR_OFF);
                return 1;
            case 'h':
                /* Show help message */
                fprintf(stderr, "%s[ %susage%s ] %s [-h] "
                "[-v] [-d] [-c config] [-e command]%s\n", 
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
}
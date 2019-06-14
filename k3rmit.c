// gcc -O2 -Wall $(pkg-config --cflags vte-2.91) k3rmit.c -o k3rmit.o $(pkg-config --libs vte-2.91)

#include "k3rmit.h"
#include <stdio.h>
#include <locale.h>
#include <vte/vte.h>

#define CLR_R(x)   (((x) & 0xff0000) >> 16)
#define CLR_G(x)   (((x) & 0x00ff00) >>  8)
#define CLR_B(x)   (((x) & 0x0000ff) >>  0)
#define CLR_16(x)  ((double)(x) / 0xff)
#define CLR_GDK(x) (const GdkRGBA){ .red = CLR_16(CLR_R(x)), \
                                    .green = CLR_16(CLR_G(x)), \
                                    .blue = CLR_16(CLR_B(x)), \
                                    .alpha = 0 }

static GtkWidget *window, *terminal; /* Window and terminal widgets */
static PangoFontDescription *fontDesc; /* Description for the terminal font */
static FILE *configFile; /* Terminal configuration file */
/* Variables for the terminal configuration */
static float termOpacity = TERM_OPACITY;
static int defaultFontSize = TERM_FONT_DEFAULT_SIZE,
         termForeground = TERM_FOREGROUND,
         currentFontSize;
static char *termFont = TERM_FONT,
        *termLocale = TERM_LOCALE,
        *termWordChars = TERM_WORD_CHARS,
        *wordChars, *fontSize, *colorIndex, 
        *configFileName;
static GdkRGBA termPalette[] = {                    
        CLR_GDK(0x3f3f3f), CLR_GDK(0xcf0000),
        CLR_GDK(0x33ff00), CLR_GDK(0xf3f828),
        CLR_GDK(0x0300ff), CLR_GDK(0xcc00ff),
        CLR_GDK(0x0300ff), CLR_GDK(0xdcdccc),
        CLR_GDK(0x808080), CLR_GDK(0xcf0000),
        CLR_GDK(0x33ff00), CLR_GDK(0x6b6b6b),
        CLR_GDK(0x0066ff), CLR_GDK(0xcc00ff),
        CLR_GDK(0x34e2e2), CLR_GDK(0xdcdccc)
    };

/*!
 * Set signals for terminal and window.
 */
void connectSignals(){
    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
    g_signal_connect(terminal, "key-press-event", G_CALLBACK(termOnKeyPress), 
                        GTK_WINDOW(window));
    g_signal_connect(terminal, "window-title-changed", G_CALLBACK(termOnTitleChanged), 
                        GTK_WINDOW(window));
}

/*!
 * Handle terminal key press events.
 * 
 * \param terminal
 * \param event (key press or release)
 * \param user_data
 * \return FALSE on normal press & TRUE on custom actions
 */
gboolean termOnKeyPress(GtkWidget *terminal, GdkEventKey *event, 
                gpointer user_data){
    /* Check for CTRL, ALT and SHIFT keys */
    switch (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK)) {
        /* CTRL + ALT */
        case GDK_MOD1_MASK | GDK_CONTROL_MASK:
            switch (event->keyval) {
                /* Copy & Paste */
                case GDK_KEY_c:
                    vte_terminal_copy_clipboard_format(VTE_TERMINAL(terminal), 
                        VTE_FORMAT_TEXT);
                    return TRUE;
                case GDK_KEY_v:
                    vte_terminal_paste_clipboard(VTE_TERMINAL(terminal));
                    return TRUE;
                 /* Change font size */
                case GDK_KEY_plus:
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
 * \param user_data
 * \return TRUE on title change
 */
gboolean termOnTitleChanged(GtkWidget *terminal, gpointer user_data){
	GtkWindow *window = user_data;
	gtk_window_set_title(window,
	    vte_terminal_get_window_title(VTE_TERMINAL(terminal))?:TERM_NAME);
	return TRUE;
}

/*!
 * Set terminal font with given size.
 *
 * \param fontSize
 */
void setTermFont(int fontSize){
    gchar *fontStr = g_strconcat(termFont, " ", 
        g_strdup_printf("%d", fontSize), NULL);
    if ((fontDesc = pango_font_description_from_string(fontStr)) != NULL){
	    vte_terminal_set_font(VTE_TERMINAL(terminal), fontDesc);
        currentFontSize = fontSize;
	    pango_font_description_free(fontDesc);
        g_free(fontStr);
    }
}

/*!
 * Configure the terminal.
 */
void configureTerm(){
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
        &CLR_GDK(termForeground),             /* Foreground */
        &(GdkRGBA){ .alpha = termOpacity },   /* Background */ 
        termPalette ,                         /* Palette */
        sizeof(termPalette)/sizeof(GdkRGBA)); 
    setTermFont(defaultFontSize);
    /* Create a window with alpha channel for transparency */
    gtk_widget_set_visual(window, 
        gdk_screen_get_rgba_visual(gtk_widget_get_screen(window)));
}

/*!
 * Async callback for terminal state.
 *
 * \param terminal
 * \param pid (Process ID)
 * \param error
 * \param user_data
 */
void termStateCallback(VteTerminal *terminal, GPid pid,
            GError *error, gpointer user_data){
    if (error == NULL){
        fprintf(stderr, "%s started. (PID: %d)\n", TERM_NAME, pid);
    }else{
        fprintf(stderr, "An error occurred: %s\n", error->message);
        g_clear_error(&error);
    }
}

/*!
 * Initialize and start the terminal.
 */
void startTerm(){
    terminal = vte_terminal_new();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    /* Terminal configuration */
    connectSignals();
    configureTerm();
    /* Start a new shell */
    gchar **envp = g_get_environ();
    gchar **command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), NULL };
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
}

/*!
 * Read settings from configuration file and apply.
 */
void parseSettings(){
    char buf[TERM_CONFIG_LENGTH], 
        option[TERM_CONFIG_LENGTH], 
        value[TERM_CONFIG_LENGTH];
    configFileName = g_strconcat(getenv("HOME"), 
                TERM_CONFIG_DIR, TERM_NAME, ".conf", NULL);
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
                 termLocale = value;
            /* Word chars */
            }else if(!strncmp(option, "char", strlen(option))){
                /* Remove '"' from word chars */
                wordChars = value;
                wordChars[strlen(wordChars)-1] = 0;
                termWordChars = wordChars+1;
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
                    termFont = value;
                }
            /* Opacity value */
            }else if(!strncmp(option, "opacity", strlen(option))){
                termOpacity = atof(value);
            /* Foreground color */
            }else if(!strncmp(option, "foreground", strlen(option))){
                termForeground = (int)strtol(value, NULL, 16);
            }else if(!strncmp(option, "color", strlen(option)-2)){
                /* Get the color index */
                colorIndex = strrchr(option, 'r');
                if (colorIndex != NULL) {
                    /* Set the color in palette */
                    termPalette[atoi(colorIndex+1)] = 
                        CLR_GDK((int)strtol(value, NULL, 16));
                }
            }
        }
        fclose(configFile);
    } else {
        fprintf(stderr, "%s config file not found.\n", TERM_NAME);
    }
    g_free(configFileName);
}


/*!
 * Entry-point
 */
int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    //parseArgs(argc, argv);
    /* Parse settings if configuration file exists */
    parseSettings();
    /* Initialize GTK and start the terminal */
    gtk_init(&argc, &argv);
    startTerm();
}
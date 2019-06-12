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

/*!
 * Set signals for terminal and window
 */
void connectSignals(){
    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
    g_signal_connect(terminal, "key-press-event", G_CALLBACK(termOnKeyPress), 
                        GTK_WINDOW(window));
}

/*!
 * Handle terminal key press events
 * Return FALSE on normal press & TRUE on custom actions
 */
gboolean termOnKeyPress(GtkWidget *terminal, GdkEventKey *event, 
                gpointer user_data){
    /* Check for CTRL, ALT and SHIFT keys */
    switch (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK)) {
        /* CTRL + ALT */
        case GDK_MOD1_MASK | GDK_CONTROL_MASK:
            switch (event->keyval) {
                /* Copy */
                case GDK_KEY_c:
                    vte_terminal_copy_clipboard_format(VTE_TERMINAL(terminal), 
                        VTE_FORMAT_TEXT);
                    return TRUE;
                /* Paste */
                case GDK_KEY_v:
                    vte_terminal_paste_clipboard(VTE_TERMINAL(terminal));
                    return TRUE;
            }
	}
	return FALSE;
}

/*!
 * Set terminal font with (TODO: given) size
 */
void setTermFont(){
    gchar *fontStr = g_strconcat(TERM_FONT, " ", g_strdup_printf("%d", TERM_FONT_DEFAULT_SIZE), NULL);
    if ((fontDesc = pango_font_description_from_string(fontStr)) != NULL){
	    vte_terminal_set_font(VTE_TERMINAL(terminal), fontDesc);
	    pango_font_description_free(fontDesc);
        g_free(fontStr);
    }
}

/*!
 * Configure the terminal
 */
void configureTerm(){
    /* Set numeric locale to en_US.UTF-8 */
    setlocale(LC_NUMERIC, "en_US.UTF-8");
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
    /* Set the terminal colors and font */
    vte_terminal_set_colors(VTE_TERMINAL(terminal),
        &CLR_GDK(0xffffff),          /* Foreground */
        &(GdkRGBA){ .alpha = 0.85 }, /* Background */
        (const GdkRGBA[]){           /* Palette */
            CLR_GDK(0x3f3f3f),
            CLR_GDK(0xcf0000),
            CLR_GDK(0x33ff00),
            CLR_GDK(0xf3f828),
            CLR_GDK(0x0300ff),
            CLR_GDK(0xcc00ff),
            CLR_GDK(0x0300ff),
            CLR_GDK(0xdcdccc),
            CLR_GDK(0x808080),
            CLR_GDK(0xcf0000),
            CLR_GDK(0x33ff00),
            CLR_GDK(0x6b6b6b),
            CLR_GDK(0x0066ff),
            CLR_GDK(0xcc00ff),
            CLR_GDK(0x34e2e2),
            CLR_GDK(0xdcdccc)
        }, 16);
    setTermFont();
}

/*!
 * Async callback for terminal state
 */
void termStateCallback(VteTerminal *terminal, GPid pid,
            GError *error, gpointer user_data){
    if (error == NULL){
        g_print("k3rmit started. PID: %d", pid);
    }else{
        g_print(error->message);
        g_clear_error(&error);
    }
}

/*!
 * Initialize and start the terminal
 */
void startTerm(){
    terminal = vte_terminal_new();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "k3rmit");
    /* Start a new shell */
    gchar **envp = g_get_environ();
    gchar **command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), NULL };
    g_strfreev(envp);
    /* Spawn asynchronous terminal */
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
        termStateCallback,      /* async callback */
        NULL);             /* callback data */
    /* Terminal configuration */
    connectSignals();
    configureTerm();
    /* Put widgets together and run the main loop */
    gtk_container_add(GTK_CONTAINER(window), terminal);
    gtk_widget_show_all(window);
    gtk_main();
}

/*!
 * Entry-point
 */
int main(int argc, char *argv[]) {
    /* Initialize GTK, the window and the terminal */
    gtk_init(&argc, &argv);
    /* Start the terminal */
    startTerm();
}
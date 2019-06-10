// gcc -O2 -Wall $(pkg-config --cflags vte-2.91) k3rmit.c -o k3rmit.o $(pkg-config --libs vte-2.91)

#include <stdio.h>
#include <gtk/gtk.h>
#include <vte/vte.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#endif

static GtkWidget *window, *terminal; /* Window and terminal widgets */


/*!
 * Async callback for terminal state
 */
void termCallback(VteTerminal *terminal, GPid pid,
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
        termCallback,      /* async callback */
        NULL);             /* callback data */
    /* Connect signals */
    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
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
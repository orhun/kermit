
#include <vte/vte.h>
#define TERM_NAME "k3rmit"
#define TERM_FONT "Monospace"
#define TERM_FONT_DEFAULT_SIZE 9
#define TERM_LOCALE "en_US.UTF-8"

void setTermFont(int fontSize);
gboolean termOnKeyPress(GtkWidget *widget, GdkEventKey *key, gpointer gptr);
gboolean termOnTitleChanged(GtkWidget *terminal, gpointer user_data);

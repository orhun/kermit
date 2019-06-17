
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

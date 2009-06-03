#include <gtk/gtk.h>

#include "msk0/msk0.h"
#include "gmsk/gmsk.h"
#include "header.h"


extern void draw_module(MskModule *mod, long x, long y);
extern void redraw_module(MskModule *mod);
extern GtkWidget *editor;

GtkWidget *rightclick_menu;


void on_mi_createmod_activate(GtkObject *object, gpointer data)
{
    gmsk_create_module((char *) data);
}


void on_mi_createcont_activate(GtkObject *object, gpointer data)
{
    gmsk_create_container((char *) data);
}


void on_mi_createmacro_activate(GtkObject *object, gpointer data)
{
    gmsk_create_macro((char *) data);
}


static void add_module_to_menu(GtkWidget *menu, char *display_name, char *name)
{
    GtkWidget *item;

    item = gtk_menu_item_new_with_label(display_name);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       name);
}

static void add_container_to_menu(GtkWidget *menu, char *display_name, char *name)
{
    GtkWidget *item;

    item = gtk_menu_item_new_with_label(display_name);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createcont_activate),
                       name);
}

static void add_macro_to_menu(GtkWidget *menu, char *name)
{
    GtkWidget *item;

    item = gtk_menu_item_new_with_label(name);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmacro_activate),
                       name);
}

GtkWidget *create_menu()
{
    GtkWidget *menu, *create_menu;
    GtkWidget *item;

    if ( rightclick_menu )
        return rightclick_menu;

    menu = gtk_menu_new();

    gtk_menu_set_title(GTK_MENU(menu), "Menu");

    item = gtk_menu_item_new_with_label("Create module");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    create_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), create_menu);

    add_module_to_menu(create_menu, "Oscillator", "oscillator");
    add_module_to_menu(create_menu, "Pitch to Frequency", "pitch to frequency");
    add_module_to_menu(create_menu, "Constant", "constant");
    add_module_to_menu(create_menu, "Add", "add");
    add_module_to_menu(create_menu, "Multiply", "mul");
    add_module_to_menu(create_menu, "Input", "input");
    add_module_to_menu(create_menu, "Output", "output");
    add_module_to_menu(create_menu, "Voice Number", "voicenumber");
    add_module_to_menu(create_menu, "Voice Active", "voiceactive");
    add_module_to_menu(create_menu, "Voice Pitch", "voicepitch");
    add_module_to_menu(create_menu, "Voice Velocity", "voicevelocity");
    add_module_to_menu(create_menu, "ADSR Envelope", "ADSR");
    add_module_to_menu(create_menu, "Parameter", "parameter");
    add_module_to_menu(create_menu, "Delay", "delay");
    add_module_to_menu(create_menu, "FIR Filter", "FIR filter");
    add_module_to_menu(create_menu, "IIR Filter", "IIR filter");


    item = gtk_menu_item_new_with_label("Create macro");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    create_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), create_menu);

    add_macro_to_menu(create_menu, "Simple Sine Synth");

    add_container_to_menu(menu, "Create container", "container");
    add_container_to_menu(menu, "Create instrument", "instrument");

    rightclick_menu = menu;

    gtk_widget_show_all(rightclick_menu);

    return rightclick_menu;
}


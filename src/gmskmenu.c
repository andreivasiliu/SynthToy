#include <gtk/gtk.h>

#include "header.h"
#include "msk0/msk0.h"

extern void draw_module(MskModule *mod, long x, long y);
extern GtkWidget *editor;

GtkWidget *gmsk_menu;

extern MskContainer *cont;

G_MODULE_EXPORT void on_mi_createmod_activate(GtkObject *object, gpointer data)
{
    MskModule *(*create_some_module)(MskContainer*) = data;
    
    MskModule *mod;
    
    g_mutex_lock(cont->module->world->lock_for_model);
    
    msk_container_deactivate(cont);
    msk_destroy_buffers_on_module(cont->module);
    
    mod = create_some_module(cont);
    
    msk_create_buffers_on_module(cont->module);
    msk_container_activate(cont);
    
    draw_module(mod, 10, 10);
    
    gtk_widget_queue_draw(editor);
    
    g_mutex_unlock(cont->module->world->lock_for_model);
}


GtkWidget *gmsk_create_menu()
{
    GtkWidget *menu, *create_menu;
    GtkWidget *item;
    
    if ( gmsk_menu )
        return gmsk_menu;
    
    menu = gtk_menu_new();
    
    gtk_menu_set_title(GTK_MENU(menu), "Menu");
    
    item = gtk_menu_item_new_with_label("Create");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_widget_show(item);
    
    create_menu = gtk_menu_new();
    gtk_menu_set_title(GTK_MENU(create_menu), "Create");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), create_menu);
    
    item = gtk_menu_item_new_with_label("Oscillator");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_oscillator_create);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Pitch to Frequency");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_pitchtofrequency_create);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Constant");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("AddMul");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Add");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_add_create);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Multiply");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_mul_create);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Input");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Output");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_widget_show(item);
    
    gmsk_menu = menu;
    
    return gmsk_menu;
}

#include <gtk/gtk.h>

#include "header.h"


GtkWidget *gmsk_menu;

void gmsk_create_menu()
{
    GtkWidget *menu, *create_menu;
    GtkWidget *item;
    
    if ( gmsk_menu )
        return;
    
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
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Constant");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("AddMul");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Input");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_widget_show(item);
    
    item = gtk_menu_item_new_with_label("Output");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_widget_show(item);
    
    
    gmsk_menu = menu;
}


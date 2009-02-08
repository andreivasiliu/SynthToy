#include <gtk/gtk.h>

#include "header.h"
#include "msk0/msk0.h"


extern GMutex *lock_for_model;

extern void draw_module(MskModule *mod, long x, long y);
extern void redraw_module(MskModule *mod);
extern GtkWidget *editor;

GtkWidget *gmsk_menu;

extern MskContainer *current_container;
extern MskContainer *cont;

void on_mi_createmod_activate(GtkObject *object, gpointer data)
{
    MskModule *(*create_some_module)(MskContainer*) = data;
    
    MskModule *mod;
    
    g_mutex_lock(lock_for_model);
    
    msk_container_deactivate(cont->module->world->root);
    
    mod = create_some_module(current_container);
    
    msk_container_activate(cont->module->world->root);
    
    draw_module(mod, 10, 10);
    
    /* These modules also create new ports on the parent container. */
    if ( create_some_module == msk_input_create ||
         create_some_module == msk_output_create )
    {
        redraw_module(mod->parent->module);
    }
    
    gtk_widget_queue_draw(editor);
    
    g_mutex_unlock(lock_for_model);
}


void on_mi_createcont_activate(GtkObject *object, gpointer data)
{
    MskContainer *(*create_some_container)(MskContainer*) = data;
    
    MskContainer *container;
    
    g_mutex_lock(lock_for_model);
    
    msk_container_deactivate(cont->module->world->root);
    
    container = create_some_container(current_container);
    
    msk_container_activate(cont->module->world->root);
    
    draw_module(container->module, 10, 10);
    
    gtk_widget_queue_draw(editor);
    
    g_mutex_unlock(lock_for_model);
}


void on_mi_createmacro_activate(GtkObject *object, gpointer data)
{
    MskContainer *(*create_some_macro)(MskContainer*) = data;
    MskContainer *macro;
    
    g_mutex_lock(lock_for_model);
    msk_container_deactivate(cont->module->world->root);
    
    macro = create_some_macro(current_container);
    
    msk_container_activate(cont->module->world->root);
    draw_module(macro->module, 40, 40);
    gtk_widget_queue_draw(editor);
    g_mutex_unlock(lock_for_model);
}


// This should eventually disappear... macros aren't supposed to be
// hard-coded.

MskContainer *macro_simple_sine_generator(MskContainer *parent)
{
    MskContainer *macro;
    MskModule *osc, *pitch, *velocity, *active;
    MskModule *p2f, *adsr, *mul1, *mul2, *out;
    
    /* Create them. */
    macro = msk_instrument_create(parent);
    
    osc = msk_oscillator_create(macro);
    pitch = msk_voicepitch_create(macro);
    velocity = msk_voicevelocity_create(macro);
    active = msk_voiceactive_create(macro);
    
    p2f = msk_pitchtofrequency_create(macro);
    adsr = msk_adsr_create(macro);
    mul1 = msk_mul_create(macro);
    mul2 = msk_mul_create(macro);
    out = msk_output_create(macro);
    
    /* Draw them. */
    draw_module(osc, 245, 65);
    draw_module(pitch, 30, 65);
    draw_module(velocity, 370, 170);
    draw_module(active, 195, 135);
    draw_module(p2f, 110, 65);
    draw_module(adsr, 285, 135);
    draw_module(mul1, 385, 90);
    draw_module(mul2, 470, 115);
    draw_module(out, 555, 115);
    
    /* Connect them. */
    msk_connect_ports(pitch, "pitch", p2f, "pitch");
    msk_connect_ports(p2f, "frequency", osc, "frequency");
    
    msk_connect_ports(osc, "output", mul1, "in1");
    msk_connect_ports(active, "gate", adsr, "gate");
    msk_connect_ports(adsr, "out", mul1, "in2");
    
    msk_connect_ports(mul1, "out", mul2, "in1");
    msk_connect_ports(velocity, "velocity", mul2, "in2");
    
    msk_connect_ports(mul2, "out", out, "out1");
    
    return macro;
}


GtkWidget *gmsk_create_menu()
{
    GtkWidget *menu, *create_menu;
    GtkWidget *item;
    
    if ( gmsk_menu )
        return gmsk_menu;
    
    menu = gtk_menu_new();
    
    gtk_menu_set_title(GTK_MENU(menu), "Menu");
    
    item = gtk_menu_item_new_with_label("Create module");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    
    create_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), create_menu);
    
    item = gtk_menu_item_new_with_label("Oscillator");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_oscillator_create);
    
    item = gtk_menu_item_new_with_label("Pitch to Frequency");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_pitchtofrequency_create);
    
    item = gtk_menu_item_new_with_label("Constant");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    
    item = gtk_menu_item_new_with_label("AddMul");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    
    item = gtk_menu_item_new_with_label("Add");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_add_create);
    
    item = gtk_menu_item_new_with_label("Multiply");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_mul_create);
    
    item = gtk_menu_item_new_with_label("Input");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_input_create);
    
    item = gtk_menu_item_new_with_label("Output");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_output_create);
    
    item = gtk_menu_item_new_with_label("Voice");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_voice_create);
    
    item = gtk_menu_item_new_with_label("Voice Active");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_voiceactive_create);
    
    item = gtk_menu_item_new_with_label("Voice Pitch");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_voicepitch_create);
    
    item = gtk_menu_item_new_with_label("Voice Velocity");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_voicevelocity_create);
    
    item = gtk_menu_item_new_with_label("ADSR Envelope");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmod_activate),
                       &msk_adsr_create);
    
    
    item = gtk_menu_item_new_with_label("Create macro");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    
    create_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), create_menu);
    
    item = gtk_menu_item_new_with_label("Simple Sine Synth");
    gtk_menu_shell_append(GTK_MENU_SHELL(create_menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createmacro_activate),
                       &macro_simple_sine_generator);
    
    
    item = gtk_menu_item_new_with_label("Create container");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createcont_activate),
                       &msk_container_create);
    
    item = gtk_menu_item_new_with_label("Create instrument");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_signal_connect(GTK_OBJECT(item), "activate",
                       GTK_SIGNAL_FUNC(on_mi_createcont_activate),
                       &msk_instrument_create);
    
    gmsk_menu = menu;
    
    gtk_widget_show_all(gmsk_menu);
    
    return gmsk_menu;
}


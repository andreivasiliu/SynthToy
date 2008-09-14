#include <gtk/gtk.h>


extern void emulate_note_on(int channel, int key, int velocity);
extern void emulate_note_off(int channel, int key);

extern GtkWidget *virkb;

static int virkb_last_mouse_note = -1;
static char keys_pressed[128];
static char notes_active[128];


int get_black_key_position(int key, int white_width, int black_width)
{
    const int positions[5] = { 16-2, 32+2, 64-2, 80, 96+2 };
    int octave = key / 5;
    int pos = key % 5;
    
    return octave * (white_width+1) * 7 + positions[pos] - black_width/2;
    
    
    
}

int get_black_key_note(int key)
{
    const int notes[5] = { 1, 3, 6, 8, 10 };
    int octave = key / 5;
    
    return octave * 12 + notes[key % 5];
}

int get_white_key_note(int key)
{
    const int notes[7] = { 0, 2, 4, 5, 7, 9, 11 };
    int octave = key / 7;
    
    return octave * 12 + notes[key % 7];
}

void virkb_setkey(int i)
{
    if ( i == virkb_last_mouse_note )
        return;
    
    if ( virkb_last_mouse_note >= 0 )
        emulate_note_off(1, virkb_last_mouse_note);
    
    emulate_note_on(1, i, 127);
    virkb_last_mouse_note = i;
}

void virkb_mouseoff()
{
    if ( virkb_last_mouse_note >= 0 )
    {
        emulate_note_off(1, virkb_last_mouse_note);
        virkb_last_mouse_note = -1;
    }
}

void virkb_mouseon(gdouble x, gdouble y)
{
    int i, pos;
    
    if ( x < 0 || y < 0 || x >= 465 || y >= 64 )
    {
        virkb_mouseoff();
        return;
    }
    
    /* Check the black keys. */
    if ( y < (60 * 5 / 8) )
    {
        for ( i = 0; i < 20; i++ )
        {
            pos = get_black_key_position(i, 15, 9);
            
            if ( x >= pos && x <= (pos + 9) )
            {
                virkb_setkey(get_black_key_note(i) + 48);
                return;
            }
        }
    }
    
    /* White keys. */
    for ( i = 0; i < 29; i++ )
    {
        if ( x >= i * 16 && x <= (i * 16 + 15) )
        {
            virkb_setkey(get_white_key_note(i) + 48);
            return;
        }
    }
}


void virkb_keypress(int key)
{
    if ( keys_pressed[key] )
        return;
    
    keys_pressed[key] = TRUE;
    emulate_note_on(1, key, 127);
}

void virkb_keyrelease(int key)
{
    if ( !keys_pressed[key] )
        return;
    
    keys_pressed[key] = FALSE;
    emulate_note_off(1, key);
}

void virkb_noteon(int note)
{
    notes_active[note] = TRUE;
    
    gtk_widget_queue_draw(GTK_WIDGET(virkb));
}

void virkb_noteoff(int note)
{
    notes_active[note] = FALSE;
    
    gtk_widget_queue_draw(GTK_WIDGET(virkb));
}


void paint_keyboard(GtkWidget *widget)
{
    cairo_t *cr;
    int white_keys = 29; /* 49 total */
    int key_height = 62;
    int key_width = 15;
    int i;
    int width, height;
    
    gdk_window_get_size(widget->window, &width, &height);
    
    cr = gdk_cairo_create(widget->window);
    
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    
    cairo_translate(cr, 0.5, 0.5);
    cairo_scale(cr, (float) width / 465.0f, (float) height / 64.0f);
    
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_width(cr, 1);
    
    /* Border. */
    cairo_move_to(cr, 0, key_height + 1);
    cairo_line_to(cr, white_keys * (key_width+1), key_height + 1);
    cairo_line_to(cr, white_keys * (key_width+1), 0);
    cairo_line_to(cr, 0, 0);
    
    cairo_stroke(cr);
    
    /* White Keys. */
    for ( i = 0; i < white_keys; i++ )
    {
        cairo_move_to(cr, i * (key_width+1), 1);
        cairo_line_to(cr, i * (key_width+1), key_height);
    }
    
    cairo_stroke(cr);
    
    /* Active White Keys */
    cairo_set_source_rgb(cr, 1, 1, 0);
    for ( i = 0; i < white_keys; i++ )
    {
        if ( notes_active[get_white_key_note(i) + 48] )
        {
            cairo_move_to(cr, i * (key_width+1) + 1, 1);
            cairo_rel_line_to(cr, 0, key_height - 1);
            cairo_rel_line_to(cr, key_width - 1, 0);
            cairo_rel_line_to(cr, 0, -key_height + 1);
            cairo_close_path(cr);
            
            cairo_stroke_preserve(cr);
            cairo_fill(cr);
        }
    }
    
    /* Black Keys. */
    cairo_set_source_rgb(cr, 0, 0, 0);
    for ( i = 0; i < 20; i++ )
    {
        int pos = get_black_key_position(i, key_width, 9);
        
        cairo_move_to(cr, pos-1, 1);
        cairo_rel_line_to(cr, 0, key_height * 5 / 8);
        cairo_rel_line_to(cr, 9 + 1, 0);
        cairo_rel_line_to(cr, 0, -(key_height * 5 / 8));
        cairo_close_path(cr);
    }
    
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
    
    /* Active black keys. */
    cairo_set_source_rgb(cr, 0.5, 0.5, 0);
    for ( i = 0; i < 20; i++ )
    {
        if ( notes_active[get_black_key_note(i) + 48] )
        {
            int pos = get_black_key_position(i, key_width, 9);
            
            cairo_move_to(cr, pos, 1);
            cairo_rel_line_to(cr, 0, key_height * 5 / 8 - 1);
            cairo_rel_line_to(cr, 9 - 1, 0);
            cairo_rel_line_to(cr, 0, -(key_height * 5 / 8 - 1));
            cairo_close_path(cr);
        }
    }
    
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
    
    cairo_destroy(cr);
}


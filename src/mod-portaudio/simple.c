#include <glib/gprintf.h>
#include <gmodule.h>
#include <portaudio.h>


G_MODULE_EXPORT void say_hello()
{
    g_printf ("Hello world.\n");
}


int some_main( )
{
    int err; //?
    
    err = Pa_Initialize ();
    if (err != paNoError)
    {
        Pa_GetErrorText (err);
        return 1;
    }
    
    err = Pa_Terminate ();
    if (err != paNoError)
    {
        Pa_GetErrorText (err);
        return 1;
    }
    
    return 0;
}


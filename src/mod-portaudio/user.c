#include <gmodule.h>

int main()
{
    void (*func)();
    GModule *module;
    
    module = g_module_open("simple", G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
    
    if (!module)
    {
        g_printf("Couldn't open it.\n");
        return 1;
    }
    
    if (!g_module_symbol (module, "say_hello", (gpointer *)&func))
    {
        g_printf("Couldn't get the symbol.\n");
        return 1;
    }
    
    func();
    
    g_module_close (module);
    
    return 0;
}


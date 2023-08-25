#include "PinWorld.hpp"

pw::PinWorld pin_world;

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    void WebUpdate(void)  {
        pin_world.step();
    }
#endif

int main(void)
{
    pin_world.setup();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(WebUpdate, 0, 1);
#else
    pin_world.run();
#endif

    pin_world.shutdown();  

    return 0;
}
#include "application.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

i32 main(void)
{
    init_application();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_application, 0, 1);
#else
    while (!window_should_close()) { // Detect window close button or ESC key
        update_draw_application();
    }
#endif

    exit_application();
    return 0;
}
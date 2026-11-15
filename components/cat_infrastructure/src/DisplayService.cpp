
#include "cat/infrastructure/DisplayService.hpp"

#include "bsp/esp-bsp.h"
#include "bsp/display.h"

namespace cat::infrastructure {

void initialize_display()
{
    bsp_display_start();
    bsp_display_backlight_on();
}

void lock_display()
{
    bsp_display_lock(0);
}

void unlock_display()
{
    bsp_display_unlock();
}

}

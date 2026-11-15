
#include "cat/infrastructure/DisplayService.hpp"
#include "cat/ui/HomeScreen.hpp"

extern "C" void app_main(void)
{
    cat::infrastructure::initialize_display();

    cat::infrastructure::lock_display();

    cat::ui::create_home_screen();

    cat::infrastructure::unlock_display();
}

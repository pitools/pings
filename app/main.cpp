#ifdef ENABLE_UI
#include "pings_ui.hpp"
#else
#include "config_example.hpp"
#endif

int main()
{
#ifdef ENABLE_UI
    return ui::ui();
#else
    return examples::runConfigExample();
#endif
}
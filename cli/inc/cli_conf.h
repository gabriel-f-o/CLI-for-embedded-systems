//Enables or disables all CLI functions (useful to not compile menu structures as well ref menu.c)
#ifndef CLI_EN
#define CLI_EN 1
#endif

//Enables and disables Float support (useful for MCUs that do not have FPU)
#ifndef CLI_FLOAT_EN
#define CLI_FLOAT_EN 0
#endif

//Enables polling mode. In normal mode, the command is executed right after receiving \n character. Since the treatment can be quite lengthy
//ranging from 60 to 400 us, it may not be fitted for some applications (may not be a good idea to execute it in interrupt mode)
//the polling mode allows the user to chose the right moment to treat the received command. The CLI won't accept new characters ultil
//the command is executed
#ifndef CLI_POLLING_EN
#define CLI_POLLING_EN 1
#endif

//Enables the most basic printing (menus, actions and arguments descriptions)
#ifndef CLI_MENU_PRINT_ENABLE
#define CLI_MENU_PRINT_ENABLE 1
#endif

//Enables a step further, printing what went wrong in your command. Useful to know why CLI did not accept your string.
#ifndef CLI_ERROR_PRINT_ENABLE
#define CLI_ERROR_PRINT_ENABLE 1
#endif

//Enables debuging prints, you should leave this as 0
#ifndef CLI_DEBUG_ENABLE
#define CLI_DEBUG_ENABLE 0
#endif
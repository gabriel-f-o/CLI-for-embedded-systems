# CLI-for-embedded-systems

This project aims to develop a Command Line Interface (CLI) suitable for embedded systems, with exremely low RAM footprint.
This project was heavily inspired by the CLI developped by Silicon Labs for their MCUs

How to use:

- In cli_conf.h, you'll find every configuration possible as well as an explanation for what they do.
- Once the CLI is enabled, the implementation of cliElement_t cliMainMenu[] is mandatory (see exemple/menu.c).
- The user has to insert characters one by one using the cli_insert_char function (in DMA interruption, USART interruption, or USART polling for example)
- The user is invited to define the function cli_printf to send a string to the console (weak definition uses normal printf)
- Since the execution time can range from 20 to 400 micro seconds, it is not a good idea to do all this treatment in interrupt mode. I recommend to activate
  polling mode in cli_conf.h, and poll the function cli_treat_command periodically.
- Inside the callback functions, you can use cli_get_int_argument, cli_get_uint_argument, cli_get_string_argument, cli_get_float_argument, and cli_get_buffer_argument to get the     arguments inside the buffer
  

Useful information :

- Using the macros, you can create sub menus, sub sub menus (...), and put actions in whatever organization you like
- The CLI does not need full names to find menus and actions, as long as it is possible to find an unique path
- The CLI is not sensitive to case
- You can fill actions with parameters, such as u (unsigned int), i (int), s (string), b (buffer), * (anything), and ... (various).
  Using this arguments, the programmer can exchange freedom with error check, the more specific is your argument and how many you want,
  the more CLI will be able to pinpoint exactly where is the problem.
  
- Integers can be passed in hex and normal format (10, -10, 0x10), floats can be passed only uing the dot (2.5, -2.5), and strings and buffers can be passed using 
  either {b1 b2 b3} where every byte is interpreted as a raw value and the user can write its values like an unsigned integer, as well as "Hello" where characters are interpreted in ASCII (the escape characters \0, \n, \r, \\", \\\\ are supported)
  
- Hex format is automatically recognized if the letters 'a' to 'f' are found, other wise 0x is mandatory (e.g. '10' is the same as 'a' that is the same is '0xA', '16' is the same as '0x10') 
  
- The difference between the string and the buffer is that the string will always be terminated with \0, where with buffer nothing is added.

This CLI is havily based on constant string literals to reduce RAM footprint, but costing performance and flash. In O0, the full CLI uses 10 KB of flash, and 600B of RAM.
In O3, 8 KB of flash and 600B of ram. Each new element (sub menu or action) will cost around 30 Bytes of RAM, and around 50 bytes of Flash (depending on the size of the strings used and the sie of the code inside the callback functions 

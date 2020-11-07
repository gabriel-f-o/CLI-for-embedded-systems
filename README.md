# CLI-for-embedded-systems

This project aims to develop a Command Line Interface (CLI) suitable for embedded systems, with exremely low RAM footprint.
This project was heavily inspired by the CLI developped by Silicon Labs for their MCUs

How to use:

- In cli_conf.h, you'll find every configuration possible as well as an explanation for what they do.
- Once the CLI is enabled, the implementation of cliElement_t cliMainMenu[] is mandatory (see exemple/menu.c).
- Using the macros, you can create sub menus, sub sub menus (...), and put actions in whatever organization you like
- The CLI does not need full names to find menus and actions, as long as it is possible to find an unique path
- The CLI is not sensitive to case
- You can fill actions with parameters, such as u (unsigned int), i (int), s (string), b (buffer), * (anything), and ... (various).
  Using this arguments, the programmer can exchange freedom with error check, the more specific is your argument and how many you want,
  the more CLI will be able to pinpoint exactly where is the problem.
  
- Integers can be passed in hex and normal format (10, -10, 0x10), floats can be passed only uing the dot (2.5, -2.5), and strings and buffers can be passed using 
  either {0A 0B 0C} where every byte is interpreted as a raw value, as well as "Hello" where characters are interpreted in ASCII (the escape characters \0, \n, \r, \", \\ are       supported)
  
- The difference between the string and the buffer is that the string will always be terminated with \0, where with buffer nothing is added.

#include "cli.h"

#if (defined(CLI_EN) && CLI_EN == 1)

/**********************************************
 * EXTERNAL VARIABLES
 *********************************************/
 
extern cliElement_t subMenu[];

/**********************************************
 * PRIVATE VARIABLES
 *********************************************/

static cliArgumentsDetails_t det[] = {
    "First number to add",
    "Second number to add",
    NULL
};

/**********************************************
 * PRIVATE FUNCTIONS
 *********************************************/

//add 10 11
//add a b
//ad 0xA 0xb
//a a b
//a 10 -10
static void addFn(){
    int8_t n1 = cli_get_int8_argument(0, NULL);
    int8_t n2 = cli_get_int8_argument(1, NULL);

    printf("CLI sum -> %d + %d = %d\n", n1, n2, n1+n2);
}

//hello "hello"
//hello "hello\nmy name is\n\"gabriel\""\0"
//he {68 65 6c 6c 6f}
static void helloFn(){
    char buffer[100];
    
    size_t bRead = cli_get_string_argument(0, buffer, sizeof(buffer), NULL);
    
    printf("CLI hello -> read %lu bytes, string = '%s'\n", bRead, buffer);
}

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)

//d 2.7 3.9
static void floatDiv(){
    float f1 = cli_get_float_argument(0, NULL);
    float f2 = cli_get_float_argument(1, NULL);

    printf("Cli div -> %.3f / %.3f = %.3f", f1, f2, f1/f2);
}
#endif

//fill_l {  1   2  3  4 }
//fill_l { 0xA 10 0010 a b 0x15  }
static void fill_LE(){
    uint8_t buffer[100];
    
    size_t bRead = cli_get_buffer_argument(0, buffer, sizeof(buffer), NULL);
    
    printf("read %lu bytes\r\n", bRead);
    
    for(int i = 0; i < bRead; i++) printf("   [%i] = %u -> 0x%02X\n", i, buffer[i], buffer[i]);
}

//fill_b {  1   2  3  4 }
//fill_b { 0xA 10 0010 a b 0x15  }
static void fill_BE(){
    uint8_t buffer[100];
    
    size_t bRead = cli_get_buffer_argument_big_endian(0, buffer, sizeof(buffer), NULL);
    
    printf("read %lu bytes\r\n", bRead);
    
    for(int i = 0; i < bRead; i++) printf("   [%i] = %u -> 0x%02X\n", i, buffer[i], buffer[i]);
}

/**********************************************
 * GLOBAL VARIABLES
 *********************************************/
 
cliElement_t cliMainMenu[] = {
    //SUB menus
    cliSubMenuElement("sub",        subMenu,    "sub menu"      ),
    cliSubMenuElement("robust?",    NULL,       NULL            ),

    //ACTION
    cliActionElement(               "fill_le",                             fill_LE,         "b",        "Fills a buffer with bytes sent by CLI (little endian)"             ),
    cliActionElement(               "fill_be",                             fill_BE,         "b",        "Fills a buffer with bytes sent by CLI (big endian)"                ),

    cliActionElementDetailed(       "add",                                 addFn,           "ii",       "adds 2 numbers and prints the result",                     det     ),
    cliActionElement(               "hello",                               helloFn,         "s",        "Prints a string passed by argument"                                ),
    
    #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
    cliActionElement(               "div",                                 floatDiv,        "ff",       "Divides 2 floating numbers"                                        ),
    #endif
    
    cliMenuTerminator()
};

#endif

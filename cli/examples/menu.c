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

//add 2 -5
//add 0x0A 1
static void addFn(){
    int64_t n1 = 0, n2 = 0;
    
    cli_get_int_argument(0, &n1);
    cli_get_int_argument(1, &n2);

    printf("CLI sum -> %ld + %ld = %ld\n", n1, n2, n1+n2);
}

//hello "hello"
//hello "hello\nmy name is\n\"gabriel\""\0"
//hello {68 65 6c 6c 6f}
static void helloFn(){
    char buffer[100];
    
    size_t bRead = 0;
    
    cli_get_string_argument(0, buffer, sizeof(buffer), &bRead);
    
    printf("CLI hello -> read %lu bytes, string = '%s'\n", bRead, buffer);
}

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)

//div 2.7 3.9
static void floatDiv(){
    float f1 = 0, f2 = 0;
    
    cli_get_float_argument(0, &f1);
    cli_get_float_argument(1, &f2);

    printf("Cli div -> %.3f / %.3f = %.3f", f1, f2, f1/f2);
}
#endif

/**********************************************
 * GLOBAL VARIABLES
 *********************************************/
 
cliElement_t cliMainMenu[] = {
    cliSubMenuElement("sub", subMenu, "sub menu"),
    cliActionElementDetailed("add", addFn, "ii", "adds 2 numbers and prints the result", det),
    cliActionElement("hello", helloFn, "s", "Prints a string passed by argument"),
    
    #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
    cliActionElement("div", floatDiv, "ff", "Prints a string passed by argument"),
    #endif
    
    cliSubMenuElement("robust?", NULL, NULL),
    cliMenuTerminator()
};

#endif
#ifndef SILABS_CLI_H
#define SILABS_CLI_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "cli_conf.h"

/**********************************************
 * GENERAL INFO
 *********************************************/
 
 /**
  * Actions arguments can have the types :
  *     i    : signed integer
  *     u    : unsigned integer
  *     b    : Buffer
  *     s    : string
  *     *    : any argument
  *     ...  : various arguments (must be at the end of the list)
  * 
  * You can pass a parameter as :
  *     i    : -10, 10, 0x10, A, 0xA, a, 0xa (if a-f is in the string, the number is considered HEX. Otherwise the 0x at the beginning is mandatory to HEX valus like 0x10)
  *     u    : 10, 0x10, A, 0xA, a, 0xa (if a-f is in the string, the number is considered HEX. Otherwise the 0x at the beginning is mandatory to HEX valus like 0x10)
  *     b    : {  b1    b2    b3  } (spaces dont matter). b1 b2 and b3 are bytes that can be passed like 'u' arguments (obviously limited to 255 or 0xFF)
  *          : "abc". in this case the data is interpreted as ASCII and copied to the buffer. The \0, \n, \r, \", \\ characters are supported)
  *     s    : same as 'b' but the buffer is terminated with \0 after getting all bytes
  * /

/**********************************************
 * DEFINES
 *********************************************/

#define cli_get_uint8_argument(argNum, res)       ((uint8_t)  cli_get_uint_argument(argNum, res))
#define cli_get_uint16_argument(argNum, res)      ((uint16_t) cli_get_uint_argument(argNum, res))
#define cli_get_uint32_argument(argNum, res)      ((uint32_t) cli_get_uint_argument(argNum, res))
#define cli_get_uint64_argument(argNum, res)      ((uint64_t) cli_get_uint_argument(argNum, res))

#define cli_get_int8_argument(argNum, res)        ((int8_t)  cli_get_int_argument(argNum, res))
#define cli_get_int16_argument(argNum, res)       ((int16_t) cli_get_int_argument(argNum, res))
#define cli_get_int32_argument(argNum, res)       ((int32_t) cli_get_int_argument(argNum, res))
#define cli_get_int64_argument(argNum, res)       ((int64_t) cli_get_int_argument(argNum, res))

#define cliMenuTerminator()                                             { NULL,     { NULL },            NULL,       NULL,       NULL }
#define cliSubMenuElement(name, ref, desc)                              { (name),   { (void*)(ref) },    NULL,       (desc),     NULL }
#define cliActionElement(name, fn, args, desc)                          { (name),   { (void*)(fn)  },    (args),     (desc),     NULL }
#define cliActionElementDetailed(name, fn, args, desc, details)         { (name),   { (void*)(fn)  },    (args),     (desc),     (details) }


/**********************************************
 * PUBLIC TYPES
 *********************************************/

//CLI insert character status. Not very useful.
typedef enum{
    CLI_NONE,
    CLI_CONTINUE,
    CLI_COMMAND_RCV,
    CLI_TOO_BIG,
    CLI_WAITING_TREATMENT,
    CLI_DISABLED,
    CLI_ERR,
}cli_status_e;

//CLI functions are always void foo(void)
typedef void(* const cliAction_t)(void);

//CLI argument details
typedef char const * const cliArgumentsDetails_t;

//CLI element (sub menu or action)
typedef struct cliElement{
    char const * const                          name;       //Unique name
    
    union{
        cliAction_t const                       action;     //Function to execute (if action)
        struct cliElement const * const         subMenuRef; //Reference to sub menu (if sub menu)
    };
    
    char const * const                          args;       //Arguments (if action), NULL if sub menu
    char const * const                          desc;       //Description of the element (NULL to ignore)
    cliArgumentsDetails_t const * const         argsDesc;   //Array of strings to describe each argument (NULL to ignore)
    
}cliElement_t;

/**********************************************
 * PUBLIC FUNCTIONS
 *********************************************/
 
#if (defined(CLI_POLLING_EN) && CLI_POLLING_EN == 1)

/*************************************************
 * CLI treat command
 * 
 * @brief This function is only accessible if polling mode is active. Its job is to execute the command inside the CLI cliBuffer
 * 
 * @param char cliBuffer[] : Buffer that contains the string [in]
 * @param size_t maxLen    : Maximum length accepted (sizeof(buffer) normally) [in]
 * 
 ************************************************/
void cli_treat_command(char cliBuffer[], size_t maxLen);

#endif //CLI_POLLING_EN

/*************************************************
 * CLI Printf
 * 
 * @brief This function is responsible of implementing printf in your system. It has a weak implementation using printf, but feel
 * free to overide it
 * 
 * @param char* str : String containing information to be printed [in]
 * @param ...       : Various arguments to print [in]
 * 
 ************************************************/
void cli_printf(char const * const str, ...);


/*************************************************
 * CLI Insert Char
 * 
 * @brief This function inserts a character in the buffer. The insert will be cyclical, but once a \n arrives, the command will be ignored 
 * 
 * @param uint8_t cliBuffer[] : Buffer that contains the string [in]
 * @param size_t maxLen       : Maximum length accepted (sizeof(buffer) normally) [in]
 * @param char c              : Character to be inserted [in]
 * 
 * @return cli_status_e : current CLI status 
 * 
 ************************************************/
cli_status_e cli_insert_char(char cliBuffer[], size_t maxLen, char const c);


/*************************************************
 * CLI Get Int Argument
 * 
 * @brief This function searches for the argument in the given position, and converts it to int64_t 
 * 
 * @param size_t argNum : Index of the argument [in]
 * @param bool *res : true if operation was successful (NULL if ignored) [out]
 * 
 * @return int64_t : result 
 * 
 ************************************************/
int64_t cli_get_int_argument(size_t argNum, bool *res);

/*************************************************
 * CLI Get UInt Argument
 * 
 * @brief This function searches for the argument in the given position, and converts it to uint64_t 
 * 
 * @param size_t argNum : Index of the argument [in]
 * @param bool *res : true if operation was successful (NULL if ignored) [out]
 * 
 * @return uint64_t : result 
 * 
 ************************************************/
uint64_t cli_get_uint_argument(size_t argNum, bool *res);

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)

/*************************************************
 * CLI Get Float Argument
 * 
 * @brief This function searches for the argument in the given position, and converts it in float.
 * It is only accessible if float option is enabled
 * 
 * @param size_t argNum : Index of the argument [in]
 * @param bool *res: true if operation was successful (NULL if ignored) [out]
 * 
 * @return float : result
 ************************************************/
float cli_get_float_argument(size_t argNum, bool *res);

#endif

/*************************************************
 * CLI Get Buffer Argument
 * 
 * @brief This function searches for the argument in the given position, and writes in the given buffer, respecting its max size
 * 
 * @param size_t argNum  : Index of the argument [in]
 * @param uint8_t buff[] : buffer to be filled [out]
 * @param size_t buffLen : maximum number of bytes to write [in]
 * @param bool *res: true if operation was successful
 * 
 * @return size_t amount if bytes read
 ************************************************/
size_t cli_get_buffer_argument(size_t argNum, uint8_t buff[], size_t buffLen, bool* res);

/*************************************************
 * CLI Get Buffer Argument Big endian
 * 
 * @brief This function searches for the argument in the given position, and writes in the given buffer, respecting its max size and flipping the order
 * of the read bytes
 * 
 * @param size_t argNum  : Index of the argument [in]
 * @param uint8_t buff[] : buffer to be filled [out]
 * @param size_t buffLen : maximum number of bytes to write [in]
 * @param bool *res: true if operation was successful
 * 
 * @return size_t amount if bytes read
 ************************************************/
size_t cli_get_buffer_argument_big_endian(size_t argNum, uint8_t buff[], size_t buffLen, bool* res);

/*************************************************
 * CLI Get String Argument
 * 
 * @brief This function searches for the argument in the given position, and writes in the given buffer, respecting its max size.
 * The string will be always terminated with \0
 * 
 * @param size_t argNum  : Index of the argument [in]
 * @param uint8_t buff[] : buffer to be filled [out]
 * @param size_t buffLen : maximum number of bytes to write [in]
 * @param bool *res: true if operation was successful
 * 
 * @return size_t amount if bytes read
 ************************************************/
size_t cli_get_string_argument(size_t argNum, uint8_t buff[], size_t buffLen, bool* res);

#endif

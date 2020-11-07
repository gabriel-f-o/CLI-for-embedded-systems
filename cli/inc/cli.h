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
 * DEFINES
 *********************************************/
 
 #ifndef __weak
 #define __weak __attribute__((weak))
 #endif

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
 * @param uint8_t cliBuffer[] : Buffer that contains the string [in]
 * @param size_t maxLen       : Maximum length accepted (sizeof(buffer) normally) [in]
 * 
 ************************************************/
void cli_treat_command(uint8_t cliBuffer[], size_t maxLen);

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
cli_status_e cli_insert_char(uint8_t cliBuffer[], size_t maxLen, char const c);


/*************************************************
 * CLI Get Int Argument
 * 
 * @brief This function searches for the argument in the given position, and converts it in int64_t 
 * 
 * @param size_t argNum : Index of the argument [in]
 * @param int64_t *res : pointer to receive the information [out]
 * 
 * @return bool : true if operation was successful
 * 
 ************************************************/
bool cli_get_int_argument(size_t argNum, int64_t *res);

/*************************************************
 * CLI Get Uint Argument
 * 
 * @brief This function searches for the argument in the given position, and converts it in uint64_t 
 * 
 * @param size_t argNum : Index of the argument [in]
 * @param uint64_t *res : pointer to receive the information [out]
 * 
 * @return bool : true if operation was successful
 * 
 ************************************************/
bool cli_get_uint_argument(size_t argNum, uint64_t *res);

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)

/*************************************************
 * CLI Get Float Argument
 * 
 * @brief This function searches for the argument in the given position, and converts it in float.
 * It is only accessible if float option is enabled
 * 
 * @param size_t argNum : Index of the argument [in]
 * @param float *res : pointer to receive the information [out]
 * 
 * @return bool : true if operation was successful
 ************************************************/
bool cli_get_float_argument(size_t argNum, float *res);

#endif

/*************************************************
 * CLI Get Buffer Argument
 * 
 * @brief This function searches for the argument in the given position, and writes in the given buffer, respecting its max size
 * 
 * @param size_t argNum  : Index of the argument [in]
 * @param uint8_t buff[] : buffer to be filled [out]
 * @param size_t buffLen : maximum number of bytes to write [in]
 * @param size_t *res : pointer to receive the amount if bytes read [out]
 * 
 * @return bool : true if operation was successful
 * 
 ************************************************/
bool cli_get_buffer_argument(size_t argNum, uint8_t buff[], size_t buffLen, size_t* bRead);

/*************************************************
 * CLI Get String Argument
 * 
 * @brief This function searches for the argument in the given position, and writes in the given buffer, respecting its max size.
 * The string will be always terminated with \0
 * 
 * @param uint8_t buff[] : buffer to be filled [out]
 * @param size_t buffLen : maximum number of bytes to write [in]
 * @param size_t *res : pointer to receive the amount if bytes read [out]
 * 
 * @return bool : true if operation was successful
 * 
 ************************************************/
bool cli_get_string_argument(size_t argNum, uint8_t buff[], size_t buffLen, size_t* bRead);

#endif

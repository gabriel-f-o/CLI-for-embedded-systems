#include "cli.h"

#if (defined(CLI_EN) && CLI_EN == 1)

/**********************************************
 * DEFINES
 *********************************************/

#define BASE_PRINT(S, ...)       cli_printf(S,## __VA_ARGS__)
#define BASE_PRINTF(S, ...)      cli_printf(S,## __VA_ARGS__)
#define BASE_PRINTLN(S, ...)     cli_printf(S"\r\n", ##__VA_ARGS__)

#if (defined(CLI_MENU_PRINT_ENABLE) && CLI_MENU_PRINT_ENABLE == 1)
    #define MENU_PRINT(S, ...)       BASE_PRINT(S,## __VA_ARGS__)
    #define MENU_PRINTF(S, ...)      BASE_PRINTF(S,## __VA_ARGS__)
    #define MENU_PRINTLN(S, ...)     BASE_PRINTLN(S,## __VA_ARGS__)
#else
    #define MENU_PRINT(S, ...)
    #define MENU_PRINTF(S, ...)
    #define MENU_PRINTLN(S, ...)    
#endif

#if (defined(CLI_ERROR_PRINT_ENABLE) && CLI_ERROR_PRINT_ENABLE == 1)
    #define ERR_PRINT(S, ...)       BASE_PRINT(S,## __VA_ARGS__)
    #define ERR_PRINTF(S, ...)      BASE_PRINTF(S,## __VA_ARGS__)
    #define ERR_PRINTLN(S, ...)     BASE_PRINTLN(S,## __VA_ARGS__)
#else
    #define ERR_PRINT(S, ...)
    #define ERR_PRINTF(S, ...)
    #define ERR_PRINTLN(S, ...)    
#endif

#if (defined(CLI_DEBUG_ENABLE) && CLI_DEBUG_ENABLE == 1)
    #define DBG_PRINT(S, ...)       BASE_PRINT("[%s:%d]" S, __FILE__, __LINE__, ## __VA_ARGS__)
    #define DBG_PRINTF(S, ...)      BASE_PRINTF(S,## __VA_ARGS__)
    #define DBG_PRINTLN(S, ...)     BASE_PRINTLN("[%s:%d]" S, __FILE__, __LINE__,## __VA_ARGS__)
#else
    #define DBG_PRINT(S, ...)
    #define DBG_PRINTF(S, ...)
    #define DBG_PRINTLN(S, ...)    
#endif

/**********************************************
 * EXTERNAL VARIABLES
 *********************************************/

extern cliElement_t cliMainMenu[];
 
/**********************************************
 * PRIVATE VARIABLES
 *********************************************/

static size_t len = 0;
static char* argsStr = NULL;
static cliElement_t* currEl = NULL;

#if (defined(CLI_POLLING_EN) && CLI_POLLING_EN == 1)
bool cli_cmd_waiting_treatment = false;
#else
static void cli_treat_command(char cliBuffer[], size_t maxLen);
#endif //CLI_POLLING_EN

/**********************************************
 * PRIVATE FUNCTIONS
 *********************************************/
 
static inline bool cli_is_terminator(cliElement_t const * const e){
    if(e == NULL) return false;
    return (e->name == NULL && e->action == NULL && e->args == NULL && e->desc == NULL && e->argsDesc == NULL);
}

static inline bool cli_is_sub_menu(cliElement_t const * const e){
    if(e == NULL) return false;
    return (e->args == NULL);  
}

static inline bool cli_is_action(cliElement_t const * const e){
    if(e == NULL) return false;
    return (e->args != NULL);
}

static int64_t cli_verify_args_str(cliElement_t const * const e, bool* elipsisPresent){
    if(e->args == NULL) return -1;
    
    size_t len = strlen(e->args);
    size_t argsLen = 0;
    *elipsisPresent = 0;
    
    for(int i = 0; i < len; i++){
        switch(e->args[i]){
            #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
            case 'f' :
            #endif
            
            case 'u' :
            case 'i' :
            case 's' :
            case 'b' :
            case '*' : {
                argsLen++;
                break;
            }
            
            case '.' : {
                if(len < i + 2 || e->args[i + 1] != '.' || e->args[i + 2] != '.'){
                    ERR_PRINTLN("Arguments string list contains incomplete elipsis for action '%s'", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                    return -1;
                }
                
                DBG_PRINTLN("%ld, %d", len, i);
                
                if(len > i + 3){
                    ERR_PRINTLN("Arguments string list contains arguments after elipsis for action '%s'", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                    return -1;
                }
                
                *elipsisPresent = 1;
                return argsLen++;
            }
            
            default : {
                ERR_PRINTLN("Unrecognized argument in argument list index %d for action '%s'", i, ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                return -1;
            }
        }
    }
    
    return argsLen;
}

static void cli_print_element(cliElement_t const * const e){
    if(e == NULL) return;
    
    bool const is_sub_menu = cli_is_sub_menu(e);

    char const * const name = ( (e->name == NULL) ? "NULL_NAME" : e->name );
    
    MENU_PRINTF("   %s%s", name, ( (is_sub_menu == true) ? "... " : " " ) );
    
    size_t len = strlen(name) + (3 * is_sub_menu) + 1; //name length with '... '

    size_t spaces = (len + CLI_AMOUNT_OF_ALIGN_CHARS) / CLI_AMOUNT_OF_ALIGN_CHARS * CLI_AMOUNT_OF_ALIGN_CHARS; //Gets nearest multiple of CLI_AMOUNT_OF_ALIGN_CHARS that is bigger than len
    
    spaces -= len;

    for(uint32_t i = 0; i < spaces; i++) MENU_PRINTF(" ");
    
    MENU_PRINTLN("- %s", ( (e->desc == NULL) ? "NULL_DESC" : e->desc) );
} 

static void cli_print_menu(cliElement_t const * const e){
    if(e == NULL) return;
    if(e->subMenuRef == NULL){
        ERR_PRINTLN("Reference of menu '%s' is NULL", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
        return;  
    } 

    for(uint8_t i = 0; !cli_is_terminator(&e->subMenuRef[i]); i++)
        cli_print_element(&e->subMenuRef[i]);
        
    MENU_PRINTLN("");
}

static void cli_print_action(cliElement_t const * const e){
    if(e == NULL) return;
    if(e->args == NULL) {
        ERR_PRINTLN("Action of element '%s' is NULL", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
        return;
    }
    
    MENU_PRINTLN("Action '%s' - %s - usage :", ( (e->name == NULL) ? "NULL_NAME" : e->name), ( (e->desc == NULL) ? "NULL_DESC" : e->desc ) );
    
    bool elipsisPresent = 0;
    int64_t len = cli_verify_args_str(e, &elipsisPresent);
    
    if(len == -1){
        return;
    }
    
    if(len == 0){
        MENU_PRINTLN("   No arguments");
        return;
    }
    
    bool argsDescEnded = (e->argsDesc == NULL || e->argsDesc[0] == NULL);
    
    int i = 0;
    for(i = 0; i < len; i++){
        switch(e->args[i]){
            #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
            case 'f' : MENU_PRINTF("   < float   > - "); break;
            #endif

            case 'u' : MENU_PRINTF("   < uint64  > - "); break;
            case 'i' : MENU_PRINTF("   < int64   > - "); break;
            case 's' : MENU_PRINTF("   < string  > - "); break;
            case 'b' : MENU_PRINTF("   < buffer  > - "); break;
            case '*' : MENU_PRINTF("   < any     > - "); break;
            default  : MENU_PRINTF("   < unknown > - "); break;
        }
        
        argsDescEnded = (argsDescEnded == true || e->argsDesc == NULL || e->argsDesc[i] == NULL);
        
        if(argsDescEnded){
            MENU_PRINTLN("");
            continue;
        }
        
        MENU_PRINTLN("%s", e->argsDesc[i]);   
    }
    
    if(elipsisPresent){
        MENU_PRINTF("   < various > - ");
        
        if(!argsDescEnded)
            MENU_PRINTLN("%s", e->argsDesc[i]); 
            MENU_PRINTLN("");
    }
    
    MENU_PRINTLN("");
}

static bool cli_str_starts_with(char const tkn[], char const str[]){
    if(tkn == NULL || str == NULL) return false;
    
    size_t const lenTkn = strlen(tkn);
    size_t const lenStr = strlen(str);
    
    if(lenTkn == 0 || lenStr == 0) return false;
    if(lenTkn > lenStr) return false;

    for(size_t i = 0; tkn[i] != '\0' ; i++){
        if(tolower(tkn[i]) != tolower(str[i])) return false;
    }
    
    return true;
}

static cliElement_t* cli_find_element_in_menu(char* tkn, char cliBuffer[], size_t maxLen, cliElement_t currentMenu[]){
    if(tkn == NULL) return NULL;
    if(maxLen == 0) return NULL;
    if(cliBuffer == NULL) return NULL;
    if(currentMenu == NULL) return NULL;
    if(currentMenu->subMenuRef == NULL) return NULL;
    
    cliElement_t* ret = NULL;
    
    for(uint8_t i = 0; !cli_is_terminator(&currentMenu->subMenuRef[i]); i++){
        if(currentMenu->subMenuRef[i].name == NULL) continue;
        
        if(!cli_str_starts_with(tkn, currentMenu->subMenuRef[i].name)) continue;
        
        if(ret != NULL){
            ERR_PRINTLN("Command not unique");
            return NULL;
        } 
        
        ret = (cliElement_t*) &currentMenu->subMenuRef[i];
    }
    
    DBG_PRINTLN("%s", ret == NULL ? "Not found" : "Sub menu or action found" );
    
    return ret;
}

static int32_t cli_arg_str_len(char* arg){
    if(arg ==  NULL) return -1;
    
    int32_t i = ( (arg[0] != '{' && arg[0] != '"') ? 0 : 1 );
    
    while(1){
        switch(arg[0]){
            
            case '{' :
            case '"' : {
                if(arg[i] == '\0') return -1;
                
                if(arg[0] == '{' && arg[i] == '}'){
                    return i+1;
                }
                else if(arg[0] == '"' && arg[i] == '"' && arg[i-1] != '\\'){
                    return i+1;
                }
                break;
            }
            
            default : {
                if(arg[i] == ' ' || arg[i] == '\0') return i;
                break;
            } 
        }
        
        i++;
    }
    
    return -1;
}

static char* cli_go_to_next_argument(char* arg){
    if(arg == NULL) return NULL;
    
    int32_t size = cli_arg_str_len(arg);
    
    if(size == -1) return NULL;
    
    arg += size;
    
    while(arg[0] != '\0'){ 
        //if(arg[0] == '-' || arg[0] == '{' || arg[0] == '"' || ('0' <= arg[0] && arg[0] <= '9')) break;
        if(arg[0] != ' ') break;
        arg++;
    }
    
    return arg;
}

static void cli_print_arg(char* s){
#if (defined(CLI_DEBUG_ENABLE) && CLI_DEBUG_ENABLE == 1)
    int len = cli_arg_str_len(s);
    DBG_PRINTF("Argument size %d = ", len);
    for(int i = 0; i < len; i++) DBG_PRINTF("%c", s[i]);
    DBG_PRINTLN("");
#endif
}

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
static bool cli_verify_float(char* tkn, bool printEn){
    int32_t len = cli_arg_str_len(tkn);

    int32_t i = 0;
    bool dotPresent = false;
    
    if(tkn[i] == '-'){
        if(len == 1){ 
            if(printEn) ERR_PRINTLN("Negative float entry does not begin");
            return false;
        }
        i++;
    }
    
    while(i < len){
        if(tkn[i] == '.'){
            if(dotPresent == true){
                if(printEn) ERR_PRINTLN("More than one dot in float argument");
                return false;
            }
            dotPresent = true;
            i++;
        } 
        
        if(!( '0' <= tkn[i] && tkn[i] <= '9' ) ){
            if(printEn) ERR_PRINTLN("Invalid character in float argument");
            return false;
        }
        
        i++;
    }  
    
    return true;
}
#endif

static bool cli_verify_int(char* tkn, bool isUnsigned, bool printEn){
    int32_t len = cli_arg_str_len(tkn);
    bool is_negative = false;
    int32_t i = 0;

    if(isUnsigned == false && tkn[0] == '-'){
        if(len == 1) {
            if(printEn) ERR_PRINTLN("Negative int does not begin");
            return false;
        }
        is_negative = true;
        i++;
    }
    
    while(i < len){
        if( tolower(tkn[i]) == 'x' && ( tkn[i-1] != '0' || (tkn[i-2] != ' ' && tkn[i-2] != '\0') || ! ( ( '0' <= tkn[i+1] && tkn[i+1] <= '9' ) || ('a' <= tolower(tkn[i+1]) && tolower(tkn[i+1]) <= 'f') ) ) ){
            if(printEn) ERR_PRINTLN("Incorrect Hex format in %s int argument", ( (isUnsigned == true) ? "unsigned" : "signed" ));
            return false;
        }
        
        if( !( ( ( '0' <= tkn[i] && tkn[i] <= '9' ) ) || ( is_negative == false && ( ('a' <= tolower(tkn[i]) && tolower(tkn[i]) <= 'f') || tolower(tkn[i]) == 'x') ) ) ) {
            if(printEn) ERR_PRINTLN("Invalid character in %s int argument", ( (isUnsigned == true) ? "unsigned" : "signed" ) );
            return false;
        }

        i++;
    }  
    
    return true;
}

static bool cli_verify_buffer(char* tkn, bool printEn){
    int i = 0;
    
    if(tkn == NULL) return false;
    if(tkn[0] != '"' && tkn[0] != '{') {
        if(printEn) ERR_PRINTLN("Buffer argument does not begin with '\"' nor '{'");
        return false;
    }
    
    int32_t len = cli_arg_str_len(tkn);
    
    if(len == -1) {
        if(printEn) ERR_PRINTLN("Buffer beginning with '%c' does not end", tkn[0]);
        return false;
    }
    
    if(tkn[0] == '"') return true;
    
    for(i = 1; i < len; i++){
        if(tkn[i] == '}') break; 
        
        if(tkn[i] == ' ') continue;
        
        if( tolower(tkn[i]) == 'x' && ( tkn[i-1] != '0' || (tkn[i-2] != ' ' && tkn[i-2] != '{') || ! ( ( '0' <= tkn[i+1] && tkn[i+1] <= '9' ) || ('a' <= tolower(tkn[i+1]) && tolower(tkn[i+1]) <= 'f') ) ) ){
            if(printEn) ERR_PRINTLN("Incorrect Hex format in buffer beginning with '{'");
            return false;
        }
        
        if( !( ( ( '0' <= tkn[i] && tkn[i] <= '9' ) ) || ('a' <= tolower(tkn[i]) && tolower(tkn[i]) <= 'f') ) && tolower(tkn[i]) != 'x' ) {
            if(printEn) ERR_PRINTLN("Invalid character in buffer beginning with '{'");
            return false;
        }
    }
    
    return true;
}

static bool cli_verify_arguments(cliElement_t* e){
    if(e == NULL) return false;
    if(e->args == NULL) {
        ERR_PRINTLN("Arguments string is null for action '%s'", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
        return false;
    }
    
    DBG_PRINTLN("args = %s", argsStr);
    
    char* argTkn = argsStr;
    
    bool elipsisPresent = 0;
    int64_t len = cli_verify_args_str(e, &elipsisPresent);
    
    DBG_PRINTLN("Len = %ld, elipsisPresent = %d", len, elipsisPresent);
    
    if(len == -1){
        return false;
    }
    
    size_t i = 0;
    
    for(i = 0; i < len; i++){
        if(argTkn == NULL) {
            ERR_PRINTLN("No arguments in argument list for action '%s'", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
            return false;
        }
        
        if(argTkn[0] == '\0'){
            ERR_PRINTLN("Expected more arguments for action '%s'", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
            return false;
        }
        
        cli_print_arg(argTkn);

        switch(e->args[i]){
            
            #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
            case 'f' : {
                if(cli_verify_float(argTkn, true)) break;
                ERR_PRINTLN("Error occured in argument %ld in action '%s'", i, ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                return false;
            }
            #endif
            
            case 'i' : {
                if(cli_verify_int(argTkn, false, true)) break;
                ERR_PRINTLN("Error occured in argument %ld in action '%s'", i, ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                return false;
            }
            
            case 'u' : {
                if(cli_verify_int(argTkn, true, true)) break;
                ERR_PRINTLN("Error occured in argument %ld in action '%s'", i, ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                return false;
            }
            
            case 's' :
            case 'b' : {
                if(cli_verify_buffer(argTkn, true)) break;
                ERR_PRINTLN("Error occured in argument %ld in action '%s'", i, ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                return false;
            }
            
            case '*' : {
                if(cli_verify_int(argTkn, false, false) || cli_verify_buffer(argTkn, false)
                
                    #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
                    || cli_verify_float(argTkn, false)
                    #endif
                
                    ) 
                {
                    break;
                }
                
                ERR_PRINTLN("Error occured in argument %ld in action '%s'", i, ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                return false;
            }
            
            default : {
                DBG_PRINTLN("Unrecognized arguments");
                ERR_PRINTLN("Error occured in argument %ld in action '%s'", i, ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
                return false;
            }
        }
        
        argTkn = cli_go_to_next_argument(argTkn);
    }
    
    if(elipsisPresent){
        while(argTkn != NULL && argTkn[0] != '\0'){
            if( !(cli_verify_int(argTkn, false, false) || cli_verify_buffer(argTkn, false) 
            
                #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
                || cli_verify_float(argTkn, false)
                #endif
            
                ) ) 
            {
                ERR_PRINTLN("Error occured in argument %ld in action '%s'", i, ( (e->name == NULL) ? "NULL_NAME" : e->name ));
                return false;
            }
            
            argTkn = cli_go_to_next_argument(argTkn);
            i++;
        }
    }
    
    else {
        if(argTkn != NULL && argTkn[0] != '\0'){
            ERR_PRINTLN("Expected less arguments for action '%s'", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
            return false;
        }
    }
    
    //if(argTkn[0] != '\0') ERR_PRINTLN();
    return true;
    //return ( (argTkn[0] == '\0') ? true : false );
}

static void cli_execute_action(cliElement_t* e){
    if(e == NULL) return;
    if(e->args == NULL) {
        ERR_PRINTLN("Argument list NULL in action '%s'", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
        return;
    }
    
    do{
        bool elipsisPresent = false;
        int64_t len = cli_verify_args_str(e, &elipsisPresent);
        
        argsStr = strtok(NULL, "\0");
        
        if(argsStr == NULL) argsStr = "";
        
        if(argsStr == NULL && len != 0) { ERR_PRINTLN("No arguments in list"); break; }
        
        while(argsStr != NULL && argsStr[0] != '\0') {
            if(argsStr[0] == '-' || argsStr[0] == '{' || argsStr[0] == '"' || ('0' <= argsStr[0] && argsStr[0] <= '9') || ('a' <= argsStr[0] && argsStr[0] <= 'f')) break;
            argsStr++;
        }
        
        if(argsStr != NULL && argsStr[0] == '\0' && len != 0) { ERR_PRINTLN("Unable to find first argument in list"); break; }
        
        if(cli_verify_arguments(e) == false) { DBG_PRINTLN("Invalid args"); break; }
        
        if(e->action == NULL) { 
            ERR_PRINTF("No action configured for action '%s'", ( (e->name == NULL)  ? "NULL_NAME" : e->name ) );
            argsStr = NULL;
            return;
        }
        
        currEl = e;
        
        e->action();
        
        currEl = NULL;
        argsStr = NULL;
        
        return;
        
    }while(0);
    
    argsStr = NULL;
    MENU_PRINTF("Invalid arguments, use 10, -10, or 0x10 for integers, 2.5 or -2.5 for float, and \"foo\" or { 0A 0B } for strings and buffers\r\n\r\n");
    cli_print_action(e);
}

static void cli_find_action(char cliBuffer[], size_t maxLen){
    if(maxLen == 0) return;
    if(cliBuffer == NULL) return;
    
    cliElement_t init = cliSubMenuElement("main", cliMainMenu, "main menu");
    cliElement_t* currentMenu = &init;
    char* tkn = strtok((char*)cliBuffer, " ");
    
    while(tkn != NULL){
        cliElement_t* e = cli_find_element_in_menu(tkn, cliBuffer, maxLen, currentMenu);
        
        if(e == NULL){
            break;
        }
        
        if(cli_is_sub_menu(e)){
            DBG_PRINTLN("Entering submenu '%s'", e->name);
            currentMenu = e;
        }
        
        if(cli_is_action(e)){
            DBG_PRINTLN("Action '%s' found", e->name);
            cli_execute_action(e);
            return;
        }
        
        tkn = strtok(NULL, " ");
    }
    
    MENU_PRINTF("Menu '%s' - %s\r\n", currentMenu->name, currentMenu->desc);
    cli_print_menu(currentMenu);
}

static bool cli_buff_element_is_hex(char* base, size_t argLen){
    if(base[0] == '0' && tolower(base[1]) == 'x') return true;
    
    for(size_t i = 0; i < argLen; i++){
        if('a' <= tolower(base[i]) && tolower(base[i]) <= 'z') return true;
    }
    
    return false;
}

static bool cli_get_int_arg(size_t argNum, int64_t *res, bool isUnsigned){
    if(currEl == NULL || argsStr == NULL) {
        ERR_PRINTLN("Function usage is exculise to functions inside CLI");
        return false;
    }
    
    if(currEl->args == NULL) {
        ERR_PRINTLN("Argument string is null");
        return false;
    }
    
    bool elipsisPresent = false;
    int64_t len = cli_verify_args_str(currEl, &elipsisPresent);
    
    if(argNum >= len && !elipsisPresent) {
        ERR_PRINTLN("Argument index out of bounds");
        return false;
    }
    
    char* argBase = argsStr;
    
    for(size_t i = 0; i < argNum; i++)
        argBase = cli_go_to_next_argument(argBase);
        
    if(argBase[0] == '\0'){ 
        ERR_PRINTLN("Argument '%c' index %lu not found in buffer", ( (isUnsigned) ? 'u' : 'i' ), argNum);
        return false; 
    }

    if( argNum < len && ( (isUnsigned && currEl->args[argNum] != 'u') || (!isUnsigned && currEl->args[argNum] != 'i') ) && currEl->args[argNum] != '*' ) {
        ERR_PRINTLN("Expected argument of type '%c', but argument list says '%c' in index %lu", ( (isUnsigned) ? 'u' : 'i' ) , currEl->args[argNum], argNum);
        return false;
    }
    else if(argNum >= len || currEl->args[argNum] == '*'){
        if(!cli_verify_int(argBase, isUnsigned, true)) return false;
    }
        
    int32_t argLen = cli_arg_str_len(argBase);
    
    int num_base = ( (cli_buff_element_is_hex(argBase, argLen) == true) ? 16 : 10 );
        
    *res = strtol(argBase, NULL, num_base);

    return true;
}

static bool cli_get_quotes(char* base, int32_t argLen, uint8_t buff[], size_t buffLen, size_t *res, bool isString){
    int pos = 0;
    bool err = true;
    int endString = ( (isString == true) ? 1 : 0 );
    
    for(int i = 1; i < argLen - 1; i++){
        
        if(pos >= buffLen - endString){
            ERR_PRINTLN("Buffer received is too tiny, exiting...");
            err = false;
            break;
        }
        
        if(base[i] == '\\'){
            switch(base[i + 1]){
                case '\\' : buff[pos++] = '\\'; i++; break;
                case '"'  : buff[pos++] = '"';  i++; break;
                case '0'  : buff[pos++] = '\0'; i++; break;
                case 'n'  : buff[pos++] = '\n'; i++; break;
                case 'r'  : buff[pos++] = '\r'; i++; break;
                
                default   : buff[pos++] = '\\'; break;
            }
            continue;
        }
        
        buff[pos++] = base[i];
    }
    
    if(isString && pos < buffLen) buff[pos++] = '\0';
    
    *res = pos;
    
    return err;
}

static bool cli_get_curly_braces(char* base, int32_t argLen, uint8_t buff[], size_t buffLen, size_t *res,  bool isString){
    int pos = 0;
    bool err = true;
    int endString = ( (isString == true) ? 1 : 0 );
        
    for(int i = 1; i < argLen - 1; i++){
        
        if(base[i] == ' ') continue;

        int32_t sz = cli_arg_str_len(&base[i]);
        
        int num_base = ( (cli_buff_element_is_hex(&base[i], sz) == true) ? 16 : 10 );
        
        uint64_t num = strtol(&base[i], NULL, num_base);
        
        if(num > 255){
            ERR_PRINTLN("Buffer argument error in byte number %u : Exceeded maximum value", pos);
            err = false;
            break;
        }
        
        if(pos >= buffLen - endString){
            ERR_PRINTLN("Buffer received is too tiny, exiting...");
            err = false;
            break;
        }
        
        buff[pos++] = (uint8_t) num;
        
        i += sz;
    }
    
    if(isString && pos < buffLen) buff[pos++] = '\0';
    
    *res = pos;
    
    return err;
}

static bool cli_get_buff_arg(size_t argNum, uint8_t buff[], size_t buffLen, size_t *res, bool isString){
    if(currEl == NULL || argsStr == NULL) {
        ERR_PRINTLN("Function usage is exculise to functions inside CLI");
        return false;
    }
    
    if(currEl->args == NULL) {
        ERR_PRINTLN("Argument string is null");
        return false;
    }
    
    bool elipsisPresent = false;
    int64_t len = cli_verify_args_str(currEl, &elipsisPresent);
    
    if(argNum >= len && !elipsisPresent) { 
        ERR_PRINTLN("Argument index out of bounds");
        return false;
    }
    
    char* argBase = argsStr;
    
    for(size_t i = 0; i < argNum; i++)
        argBase = cli_go_to_next_argument(argBase);
        
    if(argBase[0] == '\0') {
        ERR_PRINTLN("Argument '%c' number %lu not found in buffer", ( (isString) ? 's' : 'b' ), argNum );
        return false;
    }
    
    if( argNum < len && ( (!isString && currEl->args[argNum] != 'b') || (isString && currEl->args[argNum] != 's' ) ) && currEl->args[argNum] != '*' ) {
        ERR_PRINTLN("Expected argument of type 's', but argument list says %c in index %lu", currEl->args[argNum], argNum);
        return false;
    }
    else if(argNum >= len || currEl->args[argNum] == '*'){
        cli_verify_buffer(argBase, true);
    }
    
    int32_t argLen = cli_arg_str_len(argBase);
    
    if(argLen == -1) { 
        ERR_PRINTLN("Buffer in argument %lu does not end", argNum);
        return false;
    }

    if(argBase[0] == '{') 
        return cli_get_curly_braces(argBase, argLen, buff, buffLen, res, isString);
    else if(argBase[0] == '"')
        return cli_get_quotes(argBase, argLen, buff, buffLen, res, isString);
    
    ERR_PRINTLN("Buffer does not begin with \" nor {");
    return false;
}

/**********************************************
 * PRIVATE / PUBLIC FUNCTIONS
 *********************************************/
 
void cli_treat_command(char cliBuffer[], size_t maxLen){
    if(maxLen == 0) return;
    if(cliBuffer == NULL) return;
    
#if (defined(CLI_POLLING_EN) && CLI_POLLING_EN == 1)
    if(cli_cmd_waiting_treatment == false) return;
    
    cli_cmd_waiting_treatment = false; 
#endif //CLI_POLLING_EN
    
    cli_find_action(cliBuffer, maxLen);
    
    len = 0;
    memset(cliBuffer, 0, maxLen);
    DBG_PRINT("Cmd treated, enter new command = ");
}

/**********************************************
 * PUBLIC FUNCTIONS
 *********************************************/

int64_t cli_get_int_argument(size_t argNum, bool *res){
    int64_t ret = 0;
    bool success = cli_get_int_arg(argNum, &ret, false);
    
    if(res != NULL) *res = success;
    
    return ret;
}

uint64_t cli_get_uint_argument(size_t argNum, bool *res){
    uint64_t ret = 0;
    bool success = cli_get_int_arg(argNum, (int64_t*)&ret, true);
    
    if(res != NULL) *res = success;
    
    return ret;
}

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
float cli_get_float_argument(size_t argNum, bool *res){
    if(currEl == NULL || argsStr == NULL) {
        ERR_PRINTF("Function usage is exculise to functions inside CLI");
        
        if(res != NULL) *res = 0;
        
        return 0;
    }
    
    if(currEl->args == NULL) {
        ERR_PRINTF("Argument string is null");
        
        if(res != NULL) *res = 0;
        
        return 0;
    }
    
    bool elipsisPresent = false;
    int64_t len = cli_verify_args_str(currEl, &elipsisPresent);
    
    if(argNum >= len && !elipsisPresent) {
        ERR_PRINTF("Argument index out of bounds");
        
        if(res != NULL) *res = 0;
        
        return 0;
    }
    
    char* argBase = argsStr;
    
    for(size_t i = 0; i < argNum; i++)
        argBase = cli_go_to_next_argument(argBase);
        
    if(argBase[0] == '\0') {
        ERR_PRINTF("Argument index %lu not found in buffer", argNum);
        
        if(res != NULL) *res = 0;
        
        return 0;
    }
    
    if( argNum < len && currEl->args[argNum] != 'f' && currEl->args[argNum] != '*') {
        ERR_PRINTF("Expected argument of type 'f', but argument list says %c in index %lu", currEl->args[argNum], argNum);
        
        if(res != NULL) *res = 0;
        
        return 0;
    }
    else if(argNum >= len || currEl->args[argNum] == '*'){
        if(!cli_verify_float(argBase, true)){
            if(res != NULL) *res = 0;
            
            return 0;
        }
    }
    
    float ret = strtof(argBase, NULL);
    
    if(res != NULL) *res = 1;
    
    return ret;
}
#endif

size_t cli_get_buffer_argument(size_t argNum, uint8_t buff[], size_t buffLen, bool* res){
    size_t bRead = 0;
    bool ret = cli_get_buff_arg(argNum, buff, buffLen, &bRead, false);
    
    if(res != NULL) *res = ret;
    
    return bRead;
}

size_t cli_get_buffer_argument_big_endian(size_t argNum, uint8_t buff[], size_t buffLen, bool* res){
    size_t bRead = 0;
    bool ret = cli_get_buff_arg(argNum, buff, buffLen, &bRead, false);
    
    uint8_t aux = 0;
    
    for(size_t i = 0; i < bRead / 2; i++){
        aux = buff[i];
        
        buff[i] = buff[bRead - i - 1];
        buff[bRead - i - 1] = aux;
    }
    
    if(res != NULL) *res = ret;
    
    return bRead;
}

size_t cli_get_string_argument(size_t argNum, uint8_t buff[], size_t buffLen, bool* res){
    size_t bRead = 0;
    bool ret = cli_get_buff_arg(argNum, buff, buffLen, &bRead, true);
    
    if(res != NULL) *res = ret;
    
    return bRead;
}

cli_status_e cli_insert_char(char cliBuffer[], size_t maxLen, char const c){
    if(maxLen == 0) return CLI_ERR;
    if(cliBuffer == NULL) return CLI_ERR;
    
#if (defined(CLI_POLLING_EN) && CLI_POLLING_EN == 1)
    if(cli_cmd_waiting_treatment){
        ERR_PRINTLN("Command waiting for treatment");
        return CLI_WAITING_TREATMENT;
    }
#endif //CLI_POLLING_EN

    if(c == '\r') return CLI_CONTINUE;

    if(c != '\n'){
        cliBuffer[((len++)%maxLen)] = c;
        return CLI_CONTINUE;
    }
    
    if(len >= maxLen){
        len = 0;
        memset(cliBuffer, 0, maxLen);
        ERR_PRINT("COMMAND TOO LARGE! Enter new command = ");
        return CLI_TOO_BIG;
    }
    
    cliBuffer[len] = '\0';
    DBG_PRINTLN("Cmd rcv = '%s'", cliBuffer);
    
#if (defined(CLI_POLLING_EN) && CLI_POLLING_EN == 0)
    cli_treat_command(cliBuffer, maxLen);
    return CLI_COMMAND_RCV;
#else
    cli_cmd_waiting_treatment = true;
    return CLI_WAITING_TREATMENT;
#endif //CLI_POLLING_EN
}

__attribute__((weak)) void cli_printf(char const * const str, ...){
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
}

#else

/**********************************************
 * PUBLIC FUNCTIONS
 *********************************************/
 
#warning "Cli disabled"
 
int64_t cli_get_int_argument(size_t argNum, bool *res){
    return 0;
}

uint64_t cli_get_uint_argument(size_t argNum, bool *res){
    return 0;
}

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
float cli_get_float_argument(size_t argNum, bool *res){
    return 0;
}
#endif

size_t cli_get_buffer_argument(size_t argNum, uint8_t buff[], size_t buffLen, bool* res){
    return 0;
}

size_t cli_get_buffer_argument_big_endian(size_t argNum, uint8_t buff[], size_t buffLen, bool* res){
    return 0;
}

size_t cli_get_string_argument(size_t argNum, uint8_t buff[], size_t buffLen, bool* res){
    return 0;
}

cli_status_e cli_insert_char(char cliBuffer[], size_t maxLen, char const c){
    return CLI_DISABLED;
}

void cli_treat_command(char cliBuffer[], size_t maxLen){
    (void)cliBuffer;
    (void)maxLen;
}

__attribute__((weak)) void cli_printf(char const * const str, ...){}

#endif //CLI_EN


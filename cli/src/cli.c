#include "cli.h"

#if (defined(CLI_EN) && CLI_EN == 1)

/**********************************************
 * DEFINES
 *********************************************/

#define BASE_PRINT(S, ...)       cli_printf(S,## __VA_ARGS__)
#define BASE_PRINTF(S, ...)      cli_printf(S,## __VA_ARGS__)
#define BASE_PRINTLN(S, ...)     cli_printf(S"\n", ##__VA_ARGS__)

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
static void cli_treat_command(uint8_t cliBuffer[], size_t maxLen);
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
                    ERR_PRINTLN("Arguments string list contains incomplete elipsis fo action '%s'", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
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
    
    char const * const name = ( (e->name == NULL) ? "NULL_NAME" : e->name );
    size_t len = strlen(name);
    MENU_PRINTF("   %s%s", name, ( (cli_is_sub_menu(e) == true) ? "..." : "   ") );
    
    for(int i = 0; i < (int)(15 - (len + 5)); i++) MENU_PRINTF(" ");
    
    MENU_PRINTF(" - %s\r\n", ( (e->desc == NULL) ? "NULL_DESC" : e->desc) );
} 

static void cli_print_menu(cliElement_t const * const e){
    if(e == NULL) return;
    if(e->subMenuRef == NULL){
        ERR_PRINTLN("Reference of menu '%s' is NULL", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
        return;  
    } 

    for(uint8_t i = 0; !cli_is_terminator(&e->subMenuRef[i]); i++)
        cli_print_element(&e->subMenuRef[i]);
}

static void cli_print_action(cliElement_t const * const e){
    if(e == NULL) return;
    if(e->args == NULL) {
        ERR_PRINTLN("Action of element '%s' is NULL", ( (e->name == NULL) ? "NULL_NAME" : e->name ) );
        return;
    }
    
    MENU_PRINTF("Action '%s' usage : \r\n", ( (e->name == NULL) ? "NULL_NAME" : e->name) );
    
    bool elipsisPresent = 0;
    int64_t len = cli_verify_args_str(e, &elipsisPresent);
    
    if(len == -1){
        return;
    }
    
    if(len == 0){
        MENU_PRINTF("   No arguments\r\n");
        return;
    }
    
    bool argsDescEnded = false;
    
    for(int i = 0; i < len; i++){
        switch(e->args[i]){
            #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
            case 'f' : MENU_PRINTF("   < float   > - "); break;
            #endif

            case 'u' : MENU_PRINTF("   < uint32  > - "); break;
            case 'i' : MENU_PRINTF("   < int32   > - "); break;
            case 's' : MENU_PRINTF("   < string  > - "); break;
            case 'b' : MENU_PRINTF("   < buffer  > - "); break;
            case '*' : MENU_PRINTF("   < any     > - "); break;
            default  : MENU_PRINTF("   < unknown > - "); break;
        }
        
        argsDescEnded = (argsDescEnded == true || e->argsDesc == NULL || e->argsDesc[i] == NULL);
        
        if(argsDescEnded){
            MENU_PRINTF("\r\n");
            continue;
        }
        
        MENU_PRINTF("%s\r\n", e->argsDesc[i]);   
    }
    
    if(elipsisPresent){
        MENU_PRINTF("   < various > - ");
    }
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

static cliElement_t* cli_find_element_in_menu(char* tkn, uint8_t cliBuffer[], size_t maxLen, cliElement_t currentMenu[]){
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
    DBG_PRINT("Argument size %d = ", len);
    for(int i = 0; i < len; i++) DBG_PRINTF("%c", s[i]);
    DBG_PRINTF("\r\n");
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

    int32_t i = 0;
    bool isHex = false;

    if(isUnsigned == false && tkn[0] == '-'){
        if(len == 1) {
            if(printEn) ERR_PRINTLN("Negative int does not begin");
            return false;
        }
        i++;
    }
    else if(len > 1 && tkn[0] == '0' && tolower(tkn[1]) == 'x'){
        if(len == 2){ 
            if(printEn) ERR_PRINTLN("Int in hex form does not begin");
            return false;
        }
        
        isHex = true;
        i += 2;
    }
    
    while(i < len){
        if( !( ( ( '0' <= tkn[i] && tkn[i] <= '9' ) ) || (isHex == true && 'a' <= tolower(tkn[i]) && tolower(tkn[i]) <= 'f') )  ){ 
            if(printEn) ERR_PRINTLN("Invalid character in %s int argument in %s format", ( (isUnsigned == true) ? "unsigned" : "signed" ), ( (isHex == true) ? "hex" : "normal" ) );
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
    
    int digitCount = 0;

    for(i = 1; i < len; i++){
        if(tkn[i] == '}') break; 
        
        if(tkn[i] == ' ') {
            digitCount = 0; 
            continue; 
        }
        
        if( !( ( ( '0' <= tkn[i] && tkn[i] <= '9' ) ) || ('a' <= tolower(tkn[i]) && tolower(tkn[i]) <= 'f') )  ) {
            if(printEn) ERR_PRINTLN("Invalid character in buffer beginning with '{'");
            return false;
        }
        
        if(++digitCount >= 3){
            if(printEn) ERR_PRINTLN("More than 3 digits in buffer beginning with '{'");
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
                DBG_PRINTF("Unrecognized arguments\r\n");
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
            if(argsStr[0] == '-' || argsStr[0] == '{' || argsStr[0] == '"' || ('0' <= argsStr[0] && argsStr[0] <= '9')) break;
            argsStr++;
        }
        
        if(argsStr != NULL && argsStr[0] == '\0' && len != 0) { ERR_PRINTLN("No arguments in list"); break; }
        
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

static void cli_find_action(uint8_t cliBuffer[], size_t maxLen){
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
    
    int base = 10;

    if(argLen >= 3 && argBase[0] == '0' && tolower(argBase[1]) == 'x' ){ 
        base = 16;
        argBase += 2;
    }
    
    *res = strtol(argBase, NULL, base);
    
    return true;
}

static bool cli_get_quotes(char* base, int32_t argLen, uint8_t buff[], size_t buffLen, size_t *res, bool isString){
    int pos = 0;
    bool err = false;
    int endString = ( (isString == true) ? 1 : 0 );
    
    for(int i = 1; i < argLen - 1; i++){
        
        if(pos >= buffLen - endString){
            ERR_PRINTLN("Buffer received is too tiny, exiting...");
            err = true;
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
    bool err = false;
    int endString = ( (isString == true) ? 1 : 0 );
    
    bool searchNextByte = false;
    
    for(int i = 1; i < argLen - 1; i++){
        if(pos >= (int64_t)(buffLen - endString)){
            ERR_PRINTLN("Buffer received is too tiny, exiting...");
            err = true;
            break;
        }
        
        if(searchNextByte && base[i] != ' '){
            searchNextByte = false;
            continue;
        }
        
        if(base[i] == ' '){
            continue;
        }
        
        buff[pos++] = strtol(&base[i], NULL, 16);
        searchNextByte = true;
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
 
void cli_treat_command(uint8_t cliBuffer[], size_t maxLen){
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

bool cli_get_int_argument(size_t argNum, int64_t *res){
    return cli_get_int_arg(argNum, res, false);
}

bool cli_get_uint_argument(size_t argNum, uint64_t *res){
    return (uint64_t) cli_get_int_arg(argNum, res, true);
}

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
bool cli_get_float_argument(size_t argNum, float *res){
    if(currEl == NULL || argsStr == NULL) {
        ERR_PRINTF("Function usage is exculise to functions inside CLI");
        return false;
    }
    
    if(currEl->args == NULL) {
        ERR_PRINTF("Argument string is null");
        return false;
    }
    
    bool elipsisPresent = false;
    int64_t len = cli_verify_args_str(currEl, &elipsisPresent);
    
    if(argNum >= len && !elipsisPresent) {
        ERR_PRINTF("Argument index out of bounds");
        return false;
    }
    
    char* argBase = argsStr;
    
    for(size_t i = 0; i < argNum; i++)
        argBase = cli_go_to_next_argument(argBase);
        
    if(argBase[0] == '\0') {
        ERR_PRINTF("Argument index %lu not found in buffer", argNum);
        return false;
    }
    
    if( argNum < len && currEl->args[argNum] != 'f' && currEl->args[argNum] != '*') {
            ERR_PRINTF("Expected argument of type 'f', but argument list says %c in index %lu", currEl->args[argNum], argNum);
            return false;
    }
    else if(argNum >= len || currEl->args[argNum] == '*'){
        if(!cli_verify_float(argBase, true)) return false;
    }
    
    *res = strtof(argBase, NULL);
    
    return true;
}
#endif

bool cli_get_buffer_argument(size_t argNum, uint8_t buff[], size_t buffLen, size_t* bRead){
    return cli_get_buff_arg(argNum, buff, buffLen, bRead, false);
}

bool cli_get_string_argument(size_t argNum, uint8_t buff[], size_t buffLen, size_t* bRead){
    return cli_get_buff_arg(argNum, buff, buffLen, bRead, true);
}

cli_status_e cli_insert_char(uint8_t cliBuffer[], size_t maxLen, char const c){
    if(maxLen == 0) return CLI_ERR;
    if(cliBuffer == NULL) return CLI_ERR;
    
#if (defined(CLI_POLLING_EN) && CLI_POLLING_EN == 1)
    if(cli_cmd_waiting_treatment){
        ERR_PRINTLN("Command waiting for treatment");
        return CLI_WAITING_TREATMENT;
    }
#endif //CLI_POLLING_EN

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
 
bool cli_get_int_argument(size_t argNum, int64_t *res){
    return 0;
}

bool cli_get_uint_argument(size_t argNum, uint64_t *res){
    return 0;
}

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
bool cli_get_float_argument(size_t argNum, float *res){
    return 0;
}
#endif

bool cli_get_buffer_argument(size_t argNum, uint8_t buff[], size_t buffLen, size_t* bRead){
    return 0;
}

bool cli_get_string_argument(size_t argNum, uint8_t buff[], size_t buffLen, size_t* bRead){
    return 0;
}

cli_status_e cli_insert_char(uint8_t cliBuffer[], size_t maxLen, char const c){
    return CLI_DISABLED;
}

void cli_treat_command(uint8_t cliBuffer[], size_t maxLen){
    (void)cliBuffer;
    (void)maxLen;
}

__attribute__((weak)) void cli_printf(char const * const str, ...){}

#endif //CLI_EN


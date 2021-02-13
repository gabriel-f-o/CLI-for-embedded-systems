#include "cli.h"

#if (defined(CLI_EN) && CLI_EN == 1)

/**********************************************
 * PRIVATE FUNCTIONS
 *********************************************/
 
 //s a 1 2 3
 //s a 1 2
static void average(){
    float sum = 0;
    bool res = false;
    int i = 0;
    
    while(1){
        uint8_t n = cli_get_uint8_argument(i++, &res);
        
        if(res == false) break;
        
        sum += n;
    }
    
    sum /= (i-1);
    
    printf("CLI sub menu -> average -> Average is %.3f\n", sum);
}

#if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
//s v 2.2 2
//s v 5 2
//s v 6 2.5
static void varSum(){
    bool res;
    int64_t temp = 0;
    float f = 0;
    float sum = 0;
    
    temp = cli_get_int64_argument(0, &res);
    if(res == false){
        f = cli_get_float_argument(0, &res);
        if(res == true) sum += f;
    }
    else
        sum += temp;
    
    temp = cli_get_int64_argument(1, &res);
    if(res == false){
        f = cli_get_float_argument(1, &res);
        if(res == true) sum += f;
    }
    else
        sum += temp;
        
    printf("CLI sub menu -> var_sum -> sum is %.3f\n", sum);
}
#endif

/**********************************************
 * GLOBAL VARIABLES
 *********************************************/
 
cliElement_t subMenu[] = {
    cliActionElement("average",     average,    "ii...",    "This function calculates the average of various int numbers (2 arg minimum)"),
    #if (defined(CLI_FLOAT_EN) && CLI_FLOAT_EN == 1)
    cliActionElement("var_sum",     varSum,     "**",       "This function sums any 2 numbers"),
    #endif
    cliMenuTerminator()
};

#endif
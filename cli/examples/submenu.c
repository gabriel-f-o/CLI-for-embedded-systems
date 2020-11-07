#include "cli.h"

#if (defined(CLI_EN) && CLI_EN == 1)

/**********************************************
 * PRIVATE FUNCTIONS
 *********************************************/
 
 //s a 1 2 3
 //s a 1 2
static void average(){
    float sum = 0;
    int64_t n;
    int i = 0;
    
    while(1){
        bool res = cli_get_int_argument(i++, &n);
        
        if(res == false) break;
        
        sum += n;
    }
    
    sum /= (i-1);
    
    printf("CLI sub menu -> average -> Average is %.3f\n", sum);
}

//s v 2.2 2
//s v 5 2
//s v 6 2.5
static void varSum(){
    bool res;
    int64_t temp = 0;
    float f = 0;
    float sum = 0;
    
    res = cli_get_int_argument(0, &temp);
    if(res == false){
        res = cli_get_float_argument(0, &f);
        if(res == true) sum += f;
    }
    else
        sum += temp;
    
    res = cli_get_int_argument(1, &temp);
    if(res == false){
        res = cli_get_float_argument(1, &f);
        if(res == true) sum += f;
    }
    else
        sum += temp;
        
    printf("CLI sub menu -> var_sum -> sum is %.3f\n", sum);
}

/**********************************************
 * GLOBAL VARIABLES
 *********************************************/
 
cliElement_t subMenu[] = {
    cliActionElement("average", average, "ii...", "This function calculates the average of various int numbers (2 arg minimum)"),
    cliActionElement("var_sum", varSum, "**", "This function sums any 2 numbers"),
    cliMenuTerminator()
};

#endif
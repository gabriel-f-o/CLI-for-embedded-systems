/**************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

***************************/

#include "cli.h"

char cliBuff[128];

int main()
{
    printf("Enter command = ");
    char c = 0;

    while(1){
        c = getc(stdin);
        cli_insert_char(cliBuff, sizeof(cliBuff), c);
        cli_treat_command(cliBuff, sizeof(cliBuff));
        
       // if(c=='\n')return 0;
    }
    return 0;
}










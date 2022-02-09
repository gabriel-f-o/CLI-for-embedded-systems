/*
 * cli.c
 *
 *  Created on: Feb 8, 2021
 *      Author: INGE2
 */

#include "cli.h"

#if ( defined(CLI_EN) && (CLI_EN == 1) )

/**********************************************************
 * GLOBAL VARIABLES
 **********************************************************/

cliElement_t cliMainMenu[] = {
		cliMenuTerminator()
};

#endif

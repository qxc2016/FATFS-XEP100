#ifndef _INIT_H_
#define _INIT_H_

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

void SetBusCLK_80M(void);
void SetBusCLK_40M(void);
void SCI0_INT(void);
void SPI_Init(void);
void PORT_Int(void);
void SYS_Int(void);




#endif
/* 
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#define CTYPE_HF <SYSTEMINCLUDE/ctype.h>
#include CTYPE_HF

#ifndef _CTYPE_H_RIM
#define _CTYPE_H_RIM

#if __ARMCC_VERSION > 310000
int isascii(int i);
#endif

#endif

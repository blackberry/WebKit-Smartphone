/* 
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#define STRING_HF <SYSTEMINCLUDE/string.h>
#include STRING_HF

#ifndef _STRING_H_RIM
#define _STRING_H_RIM

#ifdef __cplusplus
#ifdef __ARMCC_VERSION
using namespace std;
#endif
extern "C" {
#endif

#if __ARMCC_VERSION > 310000
#if __ARMCC_VERSION < 400000
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
#endif
#ifndef _UNISTD_H
char *strdup(const char *s1);
#endif
#endif

#ifdef _MSC_VER
size_t strlcpy(char * /*dst*/, const char * /*src*/, size_t /*len*/);
int strcasecmp(const char * /*s1*/, const char * /*s2*/);
int strncasecmp(const char * /*s1*/, const char * /*s2*/, size_t /*n*/);
#endif

#ifdef __cplusplus
}
#endif

#endif // _STRING_H_RIM

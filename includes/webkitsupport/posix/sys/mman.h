/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CORRECT THIS */
typedef int off_t;

int munmap(void *, size_t);
void *mmap(void *, size_t, int, int, int, off_t);

#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define PROT_EXEC   0x4

#define PROT_NONE   0x0

#define MAP_SHARED  0x01
#define MAP_PRIVATE 0x02

#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS

#define MAP_FAILED ((void *)-1)


#ifdef __cplusplus
}
#endif

#endif

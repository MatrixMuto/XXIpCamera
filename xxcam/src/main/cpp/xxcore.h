//
// Created by wq1950 on 17-4-26.
//

#ifndef PROJECT_XXCORE_H
#define PROJECT_XXCORE_H

#include <string>
#include <deque>
#include <sys/types.h>
#include <sys/select.h>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <malloc.h>

#define  XX_OK          0
#define  XX_ERROR      -1
#define  XX_AGAIN      -2
#define  XX_BUSY       -3
#define  XX_DONE       -4
#define  XX_DECLINED   -5
#define  XX_ABORT      -6


#define xx_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))


#endif //PROJECT_XXCORE_H

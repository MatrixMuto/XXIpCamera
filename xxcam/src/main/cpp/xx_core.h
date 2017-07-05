//
// Created by wq1950 on 17-4-26.
//

#ifndef PROJECT_XXCORE_H
#define PROJECT_XXCORE_H

#include <string>
#include <deque>
#include <list>
#include <sys/types.h>
#include <sys/select.h>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <malloc.h>
#include <cstdlib>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <semaphore.h>

typedef  uint8_t u_char;


struct rtmp_header {
public:
    uint8_t type;
    uint32_t timestamp;
    uint32_t csid;
    uint32_t msid;
    uint32_t mlen; /* message length */
};

typedef void *ngx_buf_tag_t;

typedef struct ngx_buf_s ngx_buf_t;


#include "xx_buf.h"
#include "xx_amf.h"
#include "xx_log.h"
#include "xx_io.h"
#include "xx_stream.h"
#include "xx_rtmp.h"

#define  XX_OK          0
#define  XX_ERROR      -1
#define  XX_AGAIN      -2
#define  XX_BUSY       -3
#define  XX_DONE       -4
#define  XX_DECLINED   -5
#define  XX_ABORT      -6


struct ngx_buf_s {
    u_char *pos;
    u_char *last;
    off_t file_pos;
    off_t file_last;

    u_char *start;         /* Start of buffer */
    u_char *end;           /* end of buffer */
    ngx_buf_tag_t tag;
    ngx_buf_t *shadow;


    /* the buf's content could be changed */
    unsigned temporary:1;

    /*
     * the buf's content is in a memory cache or in a read only memory
     * and must not be changed
     */
    unsigned memory:1;

    /* the buf's content is mmap()ed and must not be changed */
    unsigned mmap:1;

    unsigned recycled:1;
    unsigned in_file:1;
    unsigned flush:1;
    unsigned sync:1;
    unsigned last_buf:1;
    unsigned last_in_chain:1;

    unsigned last_shadow:1;
    unsigned temp_file:1;

    /* STUB */ int num;
};

#define xx_strlen(s)       strlen((const char *) s)
#define xx_strncmp(s1, s2, n)  strncmp((const char *) s1, (const char *) s2, n)


/* msvc and icc7 compile strcmp() to inline loop */
#define ngx_strcmp(s1, s2)  strcmp((const char *) s1, (const char *) s2)

#define xx_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))
#define xx_memzero(buf, n)       (void) memset(buf, 0, n)
#define xx_memset(buf, c, n)     (void) memset(buf, c, n)
#define xx_memmove(dst, src, n)   (void) memmove(dst, src, n)
#define xx_movemem(dst, src, n)   (((u_char *) memmove(dst, src, n)) + (n))

#define ngx_abs(value)       (((value) >= 0) ? (value) : - (value))
#define ngx_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define ngx_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))


#endif //PROJECT_XXCORE_H

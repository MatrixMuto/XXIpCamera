//
// Created by wq1950 on 17-4-26.
//

#ifndef PROJECT_AMF_H
#define PROJECT_AMF_H

#include "xx_core.h"

#define ngx_string(str)     { sizeof(str) - 1, (u_char *) str }
#define ngx_null_string     { 0, NULL }

/* basic types */
#define NGX_RTMP_AMF_NUMBER             0x00
#define NGX_RTMP_AMF_BOOLEAN            0x01
#define NGX_RTMP_AMF_STRING             0x02
#define NGX_RTMP_AMF_OBJECT             0x03
#define NGX_RTMP_AMF_NULL               0x05
#define NGX_RTMP_AMF_ARRAY_NULL         0x06
#define NGX_RTMP_AMF_MIXED_ARRAY        0x08
#define NGX_RTMP_AMF_END                0x09
#define NGX_RTMP_AMF_ARRAY              0x0a

/* extended types */
#define NGX_RTMP_AMF_INT8               0x0100
#define NGX_RTMP_AMF_INT16              0x0101
#define NGX_RTMP_AMF_INT32              0x0102
#define NGX_RTMP_AMF_VARIANT_           0x0103

/* r/w flags */
#define NGX_RTMP_AMF_OPTIONAL           0x1000
#define NGX_RTMP_AMF_TYPELESS           0x2000
#define NGX_RTMP_AMF_CONTEXT            0x4000

#define NGX_RTMP_AMF_VARIANT            (NGX_RTMP_AMF_VARIANT_\
                                        |NGX_RTMP_AMF_TYPELESS)

typedef struct {
    size_t len;
    u_char *data;
} ngx_str_t;

typedef intptr_t ngx_int_t;

typedef struct {
    ngx_int_t type;
    ngx_str_t name;
    void *data;
    size_t len;
} ngx_rtmp_amf_elt_t;


#endif //PROJECT_AMF_H

//
// Created by wq1950 on 17-4-26.
//

#ifndef PROJECT_AMF_H
#define PROJECT_AMF_H

#include "xx_core.h"

#define ngx_string(str)     { sizeof(str) - 1, (u_char *) str }
#define ngx_null_string     { 0, NULL }

/* basic types */
#define XX_RTMP_AMF_NUMBER             0x00
#define XX_RTMP_AMF_BOOLEAN            0x01
#define XX_RTMP_AMF_STRING             0x02
#define XX_RTMP_AMF_OBJECT             0x03
#define XX_RTMP_AMF_NULL               0x05
#define XX_RTMP_AMF_ARRAY_NULL         0x06
#define XX_RTMP_AMF_MIXED_ARRAY        0x08
#define XX_RTMP_AMF_END                0x09
#define XX_RTMP_AMF_ARRAY              0x0a

/* extended types */
#define XX_RTMP_AMF_INT8               0x0100
#define XX_RTMP_AMF_INT16              0x0101
#define XX_RTMP_AMF_INT32              0x0102
#define XX_RTMP_AMF_VARIANT_           0x0103

/* r/w flags */
#define XX_RTMP_AMF_OPTIONAL           0x1000
#define XX_RTMP_AMF_TYPELESS           0x2000
#define XX_RTMP_AMF_CONTEXT            0x4000

#define XX_RTMP_AMF_VARIANT            (XX_RTMP_AMF_VARIANT_\
                                        |XX_RTMP_AMF_TYPELESS)

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

typedef struct ngx_chain_s ngx_chain_t;

struct ngx_chain_s {
    ngx_buf_t *buf;
    ngx_chain_t *next;
};

typedef ngx_chain_t *(*ngx_rtmp_amf_alloc_pt)(void *arg);

typedef xxbuf *(*ngx_rtmp_amf_alloc2_pt)(void *arg);

typedef struct {
    ngx_chain_t *link, *first;
    size_t offset;
    ngx_rtmp_amf_alloc_pt alloc;
    void *arg;
//    ngx_log_t                          *log;
} ngx_rtmp_amf_ctx_t;


/* reading AMF */
ngx_int_t ngx_rtmp_amf_read(ngx_rtmp_amf_ctx_t *ctx,
                            ngx_rtmp_amf_elt_t *elts, size_t nelts);

/* writing AMF */
ngx_int_t ngx_rtmp_amf_write(ngx_rtmp_amf_ctx_t *ctx,
                             ngx_rtmp_amf_elt_t *elts, size_t nelts);

class XXAmfElt {
public:
    XXAmfElt(int type, const char *name, void *data, size_t len);

    ~XXAmfElt();

public:
    int type;
    std::string name;
    void *data;
    size_t len;
};

class XXAmf {
public:
    XXAmf();

    XXAmf(ngx_rtmp_amf_alloc2_pt *alloc);

    XXAmf(std::list<xxbuf *> *buf);
    ~XXAmf();

    XXAmf *push_back(const XXAmfElt &elt);

    int Write(std::list<xxbuf *> *out);

    void Read(std::list<xxbuf *> *list);
    int WriteArray(void *pVoid, size_t len);

    int WriteObject(XXAmf *obj);

    int Put(void *p, size_t n);

    int write_internal(std::list<XXAmfElt> &elements);


    int Get(void *p, size_t n);

    std::list<XXAmfElt> elements;

    void GetFunc(std::string &string);

    void GetNumber();

    void GetTrans(double *pDouble);

private:
    std::list<xxbuf *> *out;
    std::list<xxbuf *> *in;


    std::list<xxbuf *>::iterator it;
    size_t offset_;
    std::list<xxbuf *>::iterator it_;

    int read_object();

    int read_internal(void *data);
};

#endif //PROJECT_AMF_H

//
// Created by wq1950 on 17-4-26.
//

#include "xx_amf.h"

#define XX_DEBUG

static inline void *
ngx_rtmp_amf_reverse_copy(void *dst, void *src, size_t len) {
    size_t k;

    if (dst == NULL || src == NULL) {
        return NULL;
    }

    for (k = 0; k < len; ++k) {
        ((u_char *) dst)[k] = ((u_char *) src)[len - 1 - k];
    }

    return dst;
}

#define XX_RTMP_AMF_DEBUG_SIZE 16

#ifdef XX_DEBUG

static void
ngx_rtmp_amf_debug(const char *op, u_char *p, size_t n) {
    u_char hstr[3 * XX_RTMP_AMF_DEBUG_SIZE + 1];
    u_char str[XX_RTMP_AMF_DEBUG_SIZE + 1];
    u_char *hp, *sp;
    static u_char hex[] = "0123456789ABCDEF";
    size_t i;

    hp = hstr;
    sp = str;

    for (i = 0; i < n && i < XX_RTMP_AMF_DEBUG_SIZE; ++i) {
        *hp++ = ' ';
        if (p) {
            *hp++ = hex[(*p & 0xf0) >> 4];
            *hp++ = hex[*p & 0x0f];
            *sp++ = (*p >= 0x20 && *p <= 0x7e) ?
                    *p : (u_char) '?';
            ++p;
        } else {
            *hp++ = 'X';
            *hp++ = 'X';
            *sp++ = '?';
        }
    }
    *hp = *sp = '\0';

    LOGI("AMF %s (%u)%s '%s'", op, n, hstr, str);
}

#endif

static ngx_int_t
ngx_rtmp_amf_get(ngx_rtmp_amf_ctx_t *ctx, void *p, size_t n) {
    size_t size;
    ngx_chain_t *l;
    size_t offset;
    u_char *pos, *last;
#ifdef XX_DEBUG
    void *op = p;
    size_t on = n;
#endif

    if (!n)
        return XX_OK;

    for (l = ctx->link, offset = ctx->offset; l; l = l->next, offset = 0) {

        pos = l->buf->pos + offset;
        last = l->buf->last;

        if (last >= pos + n) {
            if (p) {
                p = xx_cpymem(p, pos, n);
            }
            ctx->offset = offset + n;
            ctx->link = l;

#ifdef XX_DEBUG
            ngx_rtmp_amf_debug("read", (u_char *) op, on);
#endif

            return XX_OK;
        }

        size = last - pos;

        if (p) {
            p = xx_cpymem(p, pos, size);
        }

        n -= size;
    }

//    ngx_log_debug1(XX_LOG_DEBUG_RTMP, ctx->log, 0,
//                   "AMF read eof (%d)", n);

    return XX_DONE;
}


int XXAmf::Get(void *p, size_t n) {
    size_t size;
    size_t offset;
    u_char *pos, *last;
#ifdef XX_DEBUG
    void *op = p;
    size_t on = n;
#endif

    if (!n)
        return XX_OK;
    std::list<xxbuf *>::iterator l;

    for (l = it_, offset = offset_; l != in->end(); l = l++, offset = 0) {

        pos = (*l)->pos + offset;
        last = (*l)->last;

        if (last >= pos + n) {
            if (p) {
                p = xx_cpymem(p, pos, n);
            }
            offset_ = offset + n;
            it_ = l;

#ifdef XX_DEBUG
            ngx_rtmp_amf_debug("read", (u_char *) op, on);
#endif

            return XX_OK;
        }

        size = last - pos;

        if (p) {
            p = xx_cpymem(p, pos, size);
        }

        n -= size;
    }

//    ngx_log_debug1(XX_LOG_DEBUG_RTMP, ctx->log, 0,
//                   "AMF read eof (%d)", n);

    return XX_DONE;

}

static ngx_int_t
ngx_rtmp_amf_read_object(ngx_rtmp_amf_ctx_t *ctx, ngx_rtmp_amf_elt_t *elts,
                         size_t nelts) {
    uint8_t type;
    uint16_t len;
    size_t n, namelen, maxlen;
    ngx_int_t rc;
    u_char buf[2];

    maxlen = 0;
    for (n = 0; n < nelts; ++n) {
        namelen = elts[n].name.len;
        if (namelen > maxlen)
            maxlen = namelen;
    }

    for (;;) {

#if !(XX_WIN32)
        char name[maxlen];
#else
        char    name[1024];
        if (maxlen > sizeof(name)) {
            return XX_ERROR;
        }
#endif
        /* read key */
        switch (ngx_rtmp_amf_get(ctx, buf, 2)) {
            case XX_DONE:
                /* Envivio sends unfinalized arrays */
                return XX_OK;
            case XX_OK:
                break;
            default:
                return XX_ERROR;
        }

        ngx_rtmp_amf_reverse_copy(&len, buf, 2);

        if (!len)
            break;

        if (len <= maxlen) {
            rc = ngx_rtmp_amf_get(ctx, name, len);

        } else {
            rc = ngx_rtmp_amf_get(ctx, name, maxlen);
            if (rc != XX_OK)
                return XX_ERROR;
            rc = ngx_rtmp_amf_get(ctx, 0, len - maxlen);
        }

        if (rc != XX_OK)
            return XX_ERROR;

        /* TODO: if we require array to be sorted on name
         * then we could be able to use binary search */
        for (n = 0; n < nelts
                    && (len != elts[n].name.len
                        || xx_strncmp(name, elts[n].name.data, len));
             ++n);

        if (ngx_rtmp_amf_read(ctx, n < nelts ? &elts[n] : NULL, 1) != XX_OK)
            return XX_ERROR;
    }

    if (ngx_rtmp_amf_get(ctx, &type, 1) != XX_OK
        || type != XX_RTMP_AMF_END) {
        return XX_ERROR;
    }

    return XX_OK;
}


static ngx_int_t
ngx_rtmp_amf_read_array(ngx_rtmp_amf_ctx_t *ctx, ngx_rtmp_amf_elt_t *elts,
                        size_t nelts) {
    uint32_t len;
    size_t n;
    u_char buf[4];

    /* read length */
    if (ngx_rtmp_amf_get(ctx, buf, 4) != XX_OK)
        return XX_ERROR;

    ngx_rtmp_amf_reverse_copy(&len, buf, 4);

    for (n = 0; n < len; ++n) {
        if (ngx_rtmp_amf_read(ctx, n < nelts ? &elts[n] : NULL, 1) != XX_OK)
            return XX_ERROR;
    }

    return XX_OK;
}


static ngx_int_t
ngx_rtmp_amf_read_variant(ngx_rtmp_amf_ctx_t *ctx, ngx_rtmp_amf_elt_t *elts,
                          size_t nelts) {
    uint8_t type;
    ngx_int_t rc;
    size_t n;
    ngx_rtmp_amf_elt_t elt;

    rc = ngx_rtmp_amf_get(ctx, &type, 1);
    if (rc != XX_OK) {
        return rc;
    }

    xx_memzero(&elt, sizeof(elt));
    for (n = 0; n < nelts; ++n, ++elts) {
        if (type == elts->type) {
            elt.data = elts->data;
            elt.len = elts->len;
        }
    }

    elt.type = type | XX_RTMP_AMF_TYPELESS;

    return ngx_rtmp_amf_read(ctx, &elt, 1);
}


static ngx_int_t
ngx_rtmp_amf_is_compatible_type(uint8_t t1, uint8_t t2) {
    return t1 == t2
           || (t1 == XX_RTMP_AMF_OBJECT && t2 == XX_RTMP_AMF_MIXED_ARRAY)
           || (t2 == XX_RTMP_AMF_OBJECT && t1 == XX_RTMP_AMF_MIXED_ARRAY);
}


ngx_int_t
ngx_rtmp_amf_read(ngx_rtmp_amf_ctx_t *ctx, ngx_rtmp_amf_elt_t *elts,
                  size_t nelts) {
    void *data;
    ngx_int_t type;
    uint8_t type8;
    size_t n;
    uint16_t len;
    ngx_int_t rc;
    u_char buf[8];
    uint32_t max_index;

    for (n = 0; n < nelts; ++n) {

        if (elts && elts->type & XX_RTMP_AMF_TYPELESS) {
            type = elts->type & ~XX_RTMP_AMF_TYPELESS;
            data = elts->data;

        } else {
            switch (ngx_rtmp_amf_get(ctx, &type8, 1)) {
                case XX_DONE:
                    if (elts->type & XX_RTMP_AMF_OPTIONAL) {
                        return XX_OK;
                    }
                case XX_ERROR:
                    return XX_ERROR;
            }
            type = type8;
            data = (elts &&
                    ngx_rtmp_amf_is_compatible_type(
                            (uint8_t) (elts->type & 0xff), (uint8_t) type))
                   ? elts->data
                   : NULL;

            if (elts && (elts->type & XX_RTMP_AMF_CONTEXT)) {
                if (data) {
                    *(ngx_rtmp_amf_ctx_t *) data = *ctx;
                }
                data = NULL;
            }
        }

        switch (type) {
            case XX_RTMP_AMF_NUMBER:
                if (ngx_rtmp_amf_get(ctx, buf, 8) != XX_OK) {
                    return XX_ERROR;
                }
                ngx_rtmp_amf_reverse_copy(data, buf, 8);
                break;

            case XX_RTMP_AMF_BOOLEAN:
                if (ngx_rtmp_amf_get(ctx, data, 1) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_STRING:
                if (ngx_rtmp_amf_get(ctx, buf, 2) != XX_OK) {
                    return XX_ERROR;
                }
                ngx_rtmp_amf_reverse_copy(&len, buf, 2);

                if (data == NULL) {
                    rc = ngx_rtmp_amf_get(ctx, data, len);

                } else if (elts->len <= len) {
                    rc = ngx_rtmp_amf_get(ctx, data, elts->len - 1);
                    if (rc != XX_OK)
                        return XX_ERROR;
                    ((char *) data)[elts->len - 1] = 0;
                    rc = ngx_rtmp_amf_get(ctx, NULL, len - elts->len + 1);

                } else {
                    rc = ngx_rtmp_amf_get(ctx, data, len);
                    ((char *) data)[len] = 0;
                }

                if (rc != XX_OK) {
                    return XX_ERROR;
                }

                break;

            case XX_RTMP_AMF_NULL:
            case XX_RTMP_AMF_ARRAY_NULL:
                break;

            case XX_RTMP_AMF_MIXED_ARRAY:
                if (ngx_rtmp_amf_get(ctx, &max_index, 4) != XX_OK) {
                    return XX_ERROR;
                }

            case XX_RTMP_AMF_OBJECT:
                if (ngx_rtmp_amf_read_object(ctx, (ngx_rtmp_amf_elt_t *) data,
                                             data && elts ? elts->len / sizeof(ngx_rtmp_amf_elt_t) : 0
                ) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_ARRAY:
                if (ngx_rtmp_amf_read_array(ctx, (ngx_rtmp_amf_elt_t *) data,
                                            data && elts ? elts->len / sizeof(ngx_rtmp_amf_elt_t) : 0
                ) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_VARIANT_:
                if (ngx_rtmp_amf_read_variant(ctx, (ngx_rtmp_amf_elt_t *) data,
                                              data && elts ? elts->len / sizeof(ngx_rtmp_amf_elt_t) : 0
                ) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_INT8:
                if (ngx_rtmp_amf_get(ctx, data, 1) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_INT16:
                if (ngx_rtmp_amf_get(ctx, buf, 2) != XX_OK) {
                    return XX_ERROR;
                }
                ngx_rtmp_amf_reverse_copy(data, buf, 2);
                break;

            case XX_RTMP_AMF_INT32:
                if (ngx_rtmp_amf_get(ctx, buf, 4) != XX_OK) {
                    return XX_ERROR;
                }
                ngx_rtmp_amf_reverse_copy(data, buf, 4);
                break;

            case XX_RTMP_AMF_END:
                return XX_OK;

            default:
                return XX_ERROR;
        }

        if (elts) {
            ++elts;
        }
    }

    return XX_OK;
}


XXAmfElt::XXAmfElt(int type, const char *name, void *data, size_t len)
        : type(type), name(name), data(data), len(len) {

}

XXAmfElt::~XXAmfElt() {

}

XXAmf::XXAmf() {

}

XXAmf::~XXAmf() {

}

XXAmf *XXAmf::push_back(const XXAmfElt &elt) {

    elements.push_back(elt);

    return NULL;
}

int XXAmf::Write(std::list<xxbuf *> *out) {
    this->out = out;
    write_internal(elements);
    return XX_OK;
}

int XXAmf::write_internal(std::list<XXAmfElt> &elements) {
    size_t n;
    ngx_int_t type;
    uint8_t type8;
    void *data;
    uint16_t len;
    uint32_t max_index;
    u_char buf[8];
    std::list<XXAmfElt>::iterator it;

    for (it = elements.begin(); it != elements.end(); ++it) {
        XXAmfElt elt = *it;
        type = elt.type;
        data = elt.data;
        len = (uint16_t) elt.len;

        if (type & XX_RTMP_AMF_TYPELESS) {
            type &= ~XX_RTMP_AMF_TYPELESS;
        } else {
            type8 = (uint8_t) type;
            if (Put(&type8, 1) != XX_OK)
                return XX_ERROR;
        }
        LOGI("write_internal type %d", type);
        switch (type) {
            case XX_RTMP_AMF_NUMBER:
                if (Put(ngx_rtmp_amf_reverse_copy(buf, data, 8), 8) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_BOOLEAN:
                if (Put(data, 1) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_STRING:
                if (len == 0 && data) {
                    len = (uint16_t) xx_strlen((u_char *) data);
                }

                if (Put(ngx_rtmp_amf_reverse_copy(buf, &len, 2), 2) != XX_OK) {
                    return XX_ERROR;
                }

                if (Put(data, len) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_NULL:
            case XX_RTMP_AMF_ARRAY_NULL:
                break;

            case XX_RTMP_AMF_MIXED_ARRAY:
                max_index = 0;
                if (Put(&max_index, 4) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_OBJECT:
                type8 = XX_RTMP_AMF_END;
                if (WriteObject((XXAmf *) data) != XX_OK
                    || Put(&type8, 1) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_ARRAY:
                if (WriteArray(data, elt.len / sizeof(ngx_rtmp_amf_elt_t)) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_INT8:
                if (Put(data, 1) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_INT16:
                if (Put(ngx_rtmp_amf_reverse_copy(buf, data, 2), 2) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            case XX_RTMP_AMF_INT32:
                if (Put(ngx_rtmp_amf_reverse_copy(buf, data, 4), 4) != XX_OK) {
                    return XX_ERROR;
                }
                break;

            default:
                return XX_ERROR;
        }
    }

    return XX_OK;
}

int XXAmf::Put(void *p, size_t n) {
    xxbuf *b;
    size_t size;

#ifdef XX_DEBUG
    ngx_rtmp_amf_debug("write", (u_char *) p, n);
#endif

    while (n) {
        b = out->empty() ? NULL : out->back();

        if (b == NULL || b->last == b->end) {
            xxbuf *buf = new xxbuf(4096);
            out->push_back(buf);
            b = buf;
        }

        size = b->end - b->last;

        if (size >= n) {
            b->last = xx_cpymem(b->last, p, n);
            return XX_OK;
        }

        b->last = xx_cpymem(b->last, p, size);
        p = (u_char *) p + size;
        n -= size;
    }

    return XX_OK;
}

int XXAmf::WriteArray(void *elts, size_t len) {
//    uint32_t len;
    size_t n;
    u_char buf[4];

//    XXAmfElt *elts;

//    len = nelts;
//    if (Put(ngx_rtmp_amf_reverse_copy(buf,&len, 4), 4) != XX_OK) {
//        return XX_ERROR;
//    }
//
//    for (n = 0; n < nelts; ++n) {
//        if (write_internal(&elts[n], 1) != XX_OK) {
//            return XX_ERROR;
//        }
//    }

    return XX_OK;
}

int XXAmf::WriteObject(XXAmf *obj) {
    uint16_t len;
    size_t n;
    u_char buf[2];
    std::list<XXAmfElt>::iterator it;

    for (it = obj->elements.begin(); it != obj->elements.end(); ++it) {
        XXAmfElt elt = *it;

        len = (uint16_t) elt.name.length();

        if (Put(ngx_rtmp_amf_reverse_copy(buf, &len, 2), 2) != XX_OK) {
            return XX_ERROR;
        }

        if (Put((void *) elt.name.c_str(), len) != XX_OK) {
            return XX_ERROR;
        }
        std::list<XXAmfElt> temp;
        temp.push_back(elt);
        if (write_internal(temp) != XX_OK) {
            return XX_ERROR;
        }
    }

    if (Put((void *) "\0\0", 2) != XX_OK) {
        return XX_ERROR;
    }

    return XX_OK;
}

void XXAmf::Read(std::list<xxbuf *> *list) {
    this->in = list;

    it_ = list->begin();
//    read_internal();
}

int XXAmf::read_internal(void *data) {
    uint8_t type8;
    uint32_t type;
    u_char buf[8];
    uint16_t len;
    uint32_t max_index;
    int rc;

    Get(&type8, 1);

    type = type8;
    switch (type) {
        case XX_RTMP_AMF_NUMBER:
            if (Get(buf, 8) != XX_OK) {
                return XX_ERROR;
            }
            LOGI("amf number 1");
            ngx_rtmp_amf_reverse_copy(data, buf, 8);
            LOGI("amf number 2");
            break;

        case XX_RTMP_AMF_BOOLEAN:
            if (Get(data, 1) != XX_OK) {
                return XX_ERROR;
            }
            break;

        case XX_RTMP_AMF_STRING:
            if (Get(buf, 2) != XX_OK) {
                return XX_ERROR;
            }
            ngx_rtmp_amf_reverse_copy(&len, buf, 2);

            if (data == NULL) {
                rc = Get(data, len);
//            }
//            } else if (elts->len <= len) {
//                rc = Get(data, elts->len - 1);
//                if (rc != XX_OK)
//                    return XX_ERROR;
//                ((char *) data)[elts->len - 1] = 0;
//                rc = Get(NULL, len - elts->len + 1);
//
            } else {
                rc = Get(data, len);
                ((char *) data)[len] = 0;
            }

            if (rc != XX_OK) {
                return XX_ERROR;
            }

            break;

        case XX_RTMP_AMF_NULL:
        case XX_RTMP_AMF_ARRAY_NULL:
            break;

        case XX_RTMP_AMF_MIXED_ARRAY:
            if (Get(&max_index, 4) != XX_OK) {
                return XX_ERROR;
            }

        case XX_RTMP_AMF_OBJECT:
//            if (ngx_rtmp_amf_read_object(ctx, data,
//                                         data && elts ? elts->len / sizeof(ngx_rtmp_amf_elt_t) : 0
//            ) != XX_OK) {
//                return XX_ERROR;
//            }
//            if (read_object())
//                break;

        case XX_RTMP_AMF_ARRAY:
//            if (ngx_rtmp_amf_read_array(ctx, data,
//                                        data && elts ? elts->len / sizeof(ngx_rtmp_amf_elt_t) : 0
//            ) != XX_OK) {
//                return XX_ERROR;
//            }
            break;

        case XX_RTMP_AMF_VARIANT_:
//            if (ngx_rtmp_amf_read_variant(ctx, data,
//                                          data && elts ? elts->len / sizeof(ngx_rtmp_amf_elt_t) : 0
//            ) != XX_OK) {
//                return XX_ERROR;
//            }
            break;

        case XX_RTMP_AMF_INT8:
            if (Get(data, 1) != XX_OK) {
                return XX_ERROR;
            }
            break;

        case XX_RTMP_AMF_INT16:
            if (Get(buf, 2) != XX_OK) {
                return XX_ERROR;
            }
            ngx_rtmp_amf_reverse_copy(data, buf, 2);
            break;

        case XX_RTMP_AMF_INT32:
            if (Get(buf, 4) != XX_OK) {
                return XX_ERROR;
            }
            ngx_rtmp_amf_reverse_copy(data, buf, 4);
            break;

        case XX_RTMP_AMF_END:
            return XX_OK;

        default:
            return XX_ERROR;
    }

}

int XXAmf::read_object() {
    return 0;
}

XXAmf::XXAmf(std::list<xxbuf *> *buf) {
    in = buf;
    it_ = buf->begin();
    offset_ = 0;
}

void XXAmf::GetFunc(std::string &string) {
    char func[128] = {0};
    read_internal(func);
    string = std::string(func);
}

void XXAmf::GetTrans(double *pDouble) {
    read_internal(pDouble);
}


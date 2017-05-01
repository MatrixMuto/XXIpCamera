//
// Created by muto on 17-4-27.
//
#include "xx_core.h"
#include "xx_stream.h"

xx_stream::xx_stream() {

}

int32_t xx_stream::ParseChunkStreamId(u_char *&p, u_char *last, uint8_t *fmt2, uint32_t *csid2) {
    uint8_t fmt;
    uint32_t csid;

    /* chunk basic header */
    fmt = (*p >> 6) & 0x03;
    csid = *p++ & 0x3f;

    if (csid == 0) {
        if (last - p < 1) {
            return XX_AGAIN;
        }
        csid = 64;
        csid += *(uint8_t *) p++;

    } else if (csid == 1) {
        if (last - p < 2) {
            return XX_AGAIN;
        }
        csid = 64;
        csid += *(uint8_t *) p++;
        csid += (uint32_t) 256 * (*(uint8_t *) p++);
    }

    LOGI("RTMP header fmt=%d csid=%u", (int) fmt, csid);

    *fmt2 = fmt;
    *csid2 = csid;

    return XX_OK;
}

int xx_stream::ParseHeader(uint8_t fmt, u_char *&p, u_char *last) {
    u_char *pp;
    rtmp_header *h;
    uint32_t timestamp, ext;


    h = &header_;
    ext = ext_;
    timestamp = dtime_;
    if (fmt <= 2) {
        if (last - p < 3) {
            return XX_AGAIN;
        }
        /* timestamp:
         *  big-endian 3b -> little-endian 4b */
        pp = (u_char *) &timestamp;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        pp[3] = 0;

        ext = (timestamp == 0x00ffffff);

        if (fmt <= 1) {
            if (last - p < 4) {
                return XX_AGAIN;
            }
            /* size:
             *  big-endian 3b -> little-endian 4b
             * type:
             *  1b -> 1b*/
            pp = (u_char *) &h->mlen;
            pp[2] = *p++;
            pp[1] = *p++;
            pp[0] = *p++;
            pp[3] = 0;
            h->type = *(uint8_t *) p++;

            if (fmt == 0) {
                if (last - p < 4) {
                    return XX_AGAIN;
                }
                /* stream:
                 *  little-endian 4b -> little-endian 4b */
                pp = (u_char *) &h->msid;
                pp[0] = *p++;
                pp[1] = *p++;
                pp[2] = *p++;
                pp[3] = *p++;
            }
        }
    }

    LOGI("RTMP header type=%d ", h->type);

    /* extended header */
    if (ext) {
        if (last - p < 4) {
            return XX_AGAIN;
        }
        pp = (u_char *) &timestamp;
        pp[3] = *p++;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
    }

//            if (st->len == 0) {
//                /* Messages with type=3 should
//                 * never have ext timestamp field
//                 * according to standard.
//                 * However that's not always the case
//                 * in real life */
//                st->ext = (ext && cscf->publish_time_fix);
//                if (fmt) {
//                    st->dtime = timestamp;
//                } else {
//                    h->timestamp = timestamp;
//                    st->dtime = 0;
//                }
//            }
    return XX_OK;
}



//
// Created by muto on 17-4-27.
//

#ifndef XXRTMP_XX_STREAM_H
#define XXRTMP_XX_STREAM_H


#include "xx_core.h"

/* Chunk Stream */
class xx_stream {
public:
    xx_stream();
public:
    int ext_;
    int dtime_;
    rtmp_header header_;
    int len_;
    std::list<xxbuf *> some_;

    int ParseHeader(uint8_t fmt, u_char *&pos, u_char *last);

    int32_t ParseChunkStreamId(u_char *&p, u_char *last, uint8_t *fmt2, uint32_t *csid2);
};

#endif //XXRTMP_XX_STREAM_H

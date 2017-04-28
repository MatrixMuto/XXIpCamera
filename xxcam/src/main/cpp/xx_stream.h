//
// Created by muto on 17-4-27.
//

#ifndef XXRTMP_XX_STREAM_H
#define XXRTMP_XX_STREAM_H


/* Chunk Stream */
class xx_stream {
public:
    xx_stream();
public:
    int ext_;
    int dtime_;
    rtmp_header header_;
    int len_;

    u_char * ParseHeader(u_char *p, u_char *string);
};

#endif //XXRTMP_XX_STREAM_H

//
// Created by muto on 17-4-27.
//

#ifndef XXRTMP_XX_STREAM_H
#define XXRTMP_XX_STREAM_H


/* Chunk Stream */
class xx_stream {
public:
    int ext;
    int dtime;
    rtmp_header header;
    int len;
};

#endif //XXRTMP_XX_STREAM_H

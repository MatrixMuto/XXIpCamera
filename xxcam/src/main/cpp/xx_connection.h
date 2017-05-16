//
// Created by Arcsoft01 on 2017/5/10.
//

#ifndef XXRTMP_XX_CONNECTION_H
#define XXRTMP_XX_CONNECTION_H

#include "xx_api.h"

//namespace xxrtmp {
class xx_stream;

class xxio;

class XXConnectionImpl : public XXConnection {
public:
    XXConnectionImpl(const std::string &string, XXConnectionCallback *pCallback);

    virtual ~XXConnectionImpl();

    virtual int Connect();

    virtual void CreateStream(XXStreamCallback *callback);

    virtual void Release();

    virtual void Set(const std::string &key, int value);

private:
    xxio *io;
    int in_chunk_size;
    xx_stream *in_streams;
    int out_chunk_size_;

    int parse(const char *url);

    char *ip_;
    char *app_;
    char *name_;
    std::string url_;
    int in_csid;
};
//}
#endif //XXRTMP_XX_CONNECTION_H

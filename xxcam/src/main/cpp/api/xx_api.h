//
// Created by Arcsoft01 on 2017/5/10.
//

#ifndef XXRTMP_XX_API_H
#define XXRTMP_XX_API_H

#include <string>

class XXStream {
public:
    virtual void Publish() = 0;

    virtual void Play() = 0;

    virtual void DeleteStream() = 0;

    virtual void CloseStream() =0;

    virtual void Seek() =0;

    virtual void Pasue() = 0;
};

/* flv media stream callback*/
class XXStreamCallback {
public:
    virtual void OnStreamCreated(XXStream* stream) = 0;

    virtual void OnStreamReleased(XXStream* stream) = 0;

    virtual void OnPublished(XXStream* stream) = 0;

    virtual void OnStreamStart(XXStream*stream) = 0;
};

class XXConnection {
public:
    virtual int Connect() = 0;

    virtual void CreateStream(XXStreamCallback* callback)= 0;

    virtual void Release()= 0;

    virtual void Set(const std::string& key, int value)= 0;
};

class XXConnectionCallback {
public:
    virtual void OnConnected(XXConnection* c) = 0;

    virtual void OnError(XXConnection* c) = 0;
};

class XXSession {
public:
    virtual XXConnection* CreateConnection(const std::string& url, XXConnectionCallback* callback) = 0;

    virtual void Finalize() = 0;
};


class XXRtmp {
public:
    static XXSession *CreateSession();
};


#endif //XXRTMP_XX_API_H

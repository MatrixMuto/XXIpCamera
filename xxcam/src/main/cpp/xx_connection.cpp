//
// Created by Arcsoft01 on 2017/5/10.
//

#include "xx_core.h"
#include "xx_stream.h"

XXConnectionImpl::XXConnectionImpl(const std::string &url, XXConnectionCallback *pCallback) {
    io = new xxio();
    in_chunk_size = 128;
    in_streams = new xx_stream[128];
    out_chunk_size_ = 4096;
    url_ = url; /* copy it */
//    sem_init(&sem_out, 0, 1);
}

XXConnectionImpl::~XXConnectionImpl() {
    delete io;
    delete[] in_streams;
}

void XXConnectionImpl::Set(const std::string &key, int value) {

}

int XXConnectionImpl::Connect() {
    int err;

    in_csid = 0;

    err = parse(url_.c_str());
    if (err) {
        return err;
    }

//    io->Connect(ip_, OnConnect, this);

//    SendChallenge();

    /* start io thread */
    io->Start();

    /* Send Connect future*/

    return XX_OK;
}

void XXConnectionImpl::CreateStream(XXStreamCallback *callback) {

    // TODO: SendCreateStream
}

void XXConnectionImpl::Release() {

}

int XXConnectionImpl::parse(const char *url) {
    char *p = strstr(url, "://");
    if (!p) {
        return XX_ERROR;
    }
    char *ip = p + 3;
    char *app = strstr(ip, "/") + 1;
    if (!app) {
        return XX_ERROR;
    }
    *(app - 1) = 0;
    char *name = strstr(app, "/") + 1;
    if (!name) {
        return XX_ERROR;
    }
    *(name - 1) = 0;

    ip_ = ip;
    app_ = app;
    name_ = name;

    LOGI("%s %s %s", ip, app, name);

    return XX_OK;
}


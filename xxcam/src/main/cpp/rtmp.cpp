//
// Created by wq1950 on 17-4-18.
//

#include "rtmp.h"
#include "xxio.h"

#include <android/log.h>
#define  LOG_TAG    "xxio"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#define NGX_RTMP_HANDSHAKE_BUFSIZE                  1537


#define NGX_RTMP_HANDSHAKE_SERVER_RECV_CHALLENGE    1
#define NGX_RTMP_HANDSHAKE_SERVER_SEND_CHALLENGE    2
#define NGX_RTMP_HANDSHAKE_SERVER_SEND_RESPONSE     3
#define NGX_RTMP_HANDSHAKE_SERVER_RECV_RESPONSE     4
#define NGX_RTMP_HANDSHAKE_SERVER_DONE              5


#define NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE    6
#define NGX_RTMP_HANDSHAKE_CLIENT_RECV_CHALLENGE    7
#define NGX_RTMP_HANDSHAKE_CLIENT_RECV_RESPONSE     8
#define NGX_RTMP_HANDSHAKE_CLIENT_SEND_RESPONSE     9
#define NGX_RTMP_HANDSHAKE_CLIENT_DONE              10

XXRtmp::XXRtmp() {
    io = new xxio();
}

XXRtmp::~XXRtmp() {
    delete io;
}

int XXRtmp::Connect() {

    io->start();
    std::string url = "rtmp://127.0.0.1:1935/live/test";

    io->Connect(url, Handshake, this);

    return 0;
}

void XXRtmp::video(uint8_t *data) {
    io->Write(data);
}


/* Callback from io thread. */

void XXRtmp::Handshake(event *ev) {
    LOGI("Handshake");
    XXRtmp *rtmp = (XXRtmp*) ev->data;

    rtmp->handshake(ev);
}

void XXRtmp::handshake(event *ev) {
    int n;
    state_ =  NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE;
    uint8_t buff[255];
    uint8_t *start;
    uint8_t *pos = buff;
    uint8_t *last = buff+255;

    while (pos != last) {
        n = io->Send(ev, pos, last - pos);

        if (n == -1) {
            //fatal error, finalize session.
            LOGE("send return %d", n);
            return ;
        }

        if (n == -EAGAIN || n == 0) {
            LOGE("send return %d", n);
            //io->addEvent();
            return;
        }
        pos += n;
    }

    switch (state_) {
        case NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE:
            break;
        default:
            break;
    }
}

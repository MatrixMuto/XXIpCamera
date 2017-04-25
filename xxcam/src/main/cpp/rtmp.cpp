//
// Created by wq1950 on 17-4-18.
//

#include "rtmp.h"
#include "xxio.h"

#ifdef ANDROID
#include <android/log.h>
    #define  LOG_TAG    "xxio"
    #define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
    #define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define  LOGI(...)  printf(__VA_ARGS__);printf("\n")
#define  LOGE(...)  printf(__VA_ARGS__);printf("\n")
#endif


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
    std::string url = "rtmp://127.0.0.1:1935/live/test";

    io->Connect(url, OnConnect, this);

    SendChallenge();

    io->start();

    return 0;
}

void XXRtmp::video(uint8_t *data) {
    io->Write(data);
}

void XXRtmp::SendChallenge() {

    state_ = NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE;

    /* Tell io the handler*/
    io->SetReadHandler(HandshakeRecv, this);
    io->SetWriteHandler(HandshakeSend, this);

    io->HandleWriteEvnet(1);
}

/* Callback from io thread. */

void XXRtmp::OnConnect(event *ev) {
    LOGI("OnConnect r:%d w:%d", ev->read, ev->write);
    XXRtmp *rtmp = (XXRtmp*) ev->data;
}

void XXRtmp::HandshakeRecv(event *rev) {
    LOGI("HandshakeRecv");
    XXRtmp *rtmp = (XXRtmp *) rev->data;
    rtmp->handshake_recv(rev);
}

void XXRtmp::HandshakeSend(event *wev) {
    LOGI("HandshakeSend");
    XXRtmp *rtmp = (XXRtmp *) wev->data;
    rtmp->handshake_send(wev);
}

void XXRtmp::handshake_send(event *wev) {

    ssize_t n;
    uint8_t buff[1537];
    uint8_t *start;
    uint8_t *pos = buff;
    uint8_t *last = buff + 1536;

    while (pos != last) {
        n = io->Send(wev, pos, last - pos);

        if (n == -1) {
            //fatal error, finalize session.
            LOGE("send return %ld", n);
            io->Close();
            return ;
        }

        if (n == -EAGAIN || n == 0) {
            LOGE("send return %ld", n);
            return;
        }
        pos += n;
    }


    if (wev->active) {
        io->deleteEvnet(wev);
    }

    state_ = NGX_RTMP_HANDSHAKE_CLIENT_RECV_CHALLENGE;

    switch (state_) {
        case NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE:
            break;
        default:
            break;
    }
}

void XXRtmp::handshake_recv(event *rev) {

    ssize_t n;
    uint8_t buff[1537];
    uint8_t *start;
    uint8_t *last = buff;
    uint8_t *end = buff + 1537;

    while (last != end) {
        n = io->Recv(rev, last, end - last);

        if (n == -1 || n == 0) {
            //ngx_rtmp_finalize_session(s);
            LOGI("error , handshake_recv");
            io->Close();
            return;
        }

        if (n == -EAGAIN) {
//            ngx_add_timer(rev, s->timeout);
//            if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
//                ngx_rtmp_finalize_session(s);
//            }
            LOGI("read again");
            return;
        }

        last += n;
    }

    if (rev->active) {
        io->deleteEvnet(rev);
    }

    switch (state_) {
        case NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE:

            break;
    }
}

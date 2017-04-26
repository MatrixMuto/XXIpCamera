//
// Created by wq1950 on 17-4-18.
//

#ifndef XXIPCAMERA_RTMP_CPP_H
#define XXIPCAMERA_RTMP_CPP_H

#include "xxcore.h"
#include "xxio.h"
#include "xxbuf.h"

class XXRtmp {
public:
    XXRtmp();
    ~XXRtmp();

    int Connect();

    void video(uint8_t *data);

    static void OnConnect(event *ev);

    static void HandshakeSend(event *ev);

    static void HandshakeRecv(event *ev);

    static void RtmpSend(event *wev);

    static void RtmpRecv(event *rev);

private:
    void handshake_send(event *ev);
    void SendChallenge();
    void handshake_recv(event *rev);

    void fill_random_buffer(xxbuf *b);

    int handshake_create_challenge(const u_char version[4]);

    void handshake_done();

    void rtmp_send(event *wev);

    void rtmp_recv(event *rev);

    int create_response();

private:
    xxio *io;
    xxbuf *hs_buf;
    int state_;
    int epoch;
    std::deque<xxbuf *> out;
};

#endif //XXIPCAMERA_RTMP_CPP_H

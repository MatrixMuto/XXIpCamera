//
// Created by wq1950 on 17-4-18.
//

#ifndef XXIPCAMERA_RTMP_CPP_H
#define XXIPCAMERA_RTMP_CPP_H

#include "xxcore.h"
#include "xxio.h"
#include "xxbuf.h"

/* RTMP message types */
#define NGX_RTMP_MSG_CHUNK_SIZE         1
#define NGX_RTMP_MSG_ABORT              2
#define NGX_RTMP_MSG_ACK                3
#define NGX_RTMP_MSG_USER               4
#define NGX_RTMP_MSG_ACK_SIZE           5
#define NGX_RTMP_MSG_BANDWIDTH          6
#define NGX_RTMP_MSG_EDGE               7
#define NGX_RTMP_MSG_AUDIO              8
#define NGX_RTMP_MSG_VIDEO              9
#define NGX_RTMP_MSG_AMF3_META          15
#define NGX_RTMP_MSG_AMF3_SHARED        16
#define NGX_RTMP_MSG_AMF3_CMD           17
#define NGX_RTMP_MSG_AMF_META           18
#define NGX_RTMP_MSG_AMF_SHARED         19
#define NGX_RTMP_MSG_AMF_CMD            20
#define NGX_RTMP_MSG_AGGREGATE          22
#define NGX_RTMP_MSG_MAX                22

class rtmp_header {
public:
    uint8_t type;
    uint32_t timestamp;
    uint32_t csid;
    uint32_t msid;
};

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
    void SendChallenge();

    void handshake_send(event *ev);
    void handshake_recv(event *rev);

    void handshake_done();

    void fill_random_buffer(xxbuf *b);

    void rtmp_send(event *wev);
    void rtmp_recv(event *rev);

    int create_response();

    int handshake_create_challenge(const u_char version[4]);

    void SendChunkSize(int chunkSize);

    void send_message(xxbuf *pXxbuf);

    void prepare_message(rtmp_header *h, rtmp_header *lh, xxbuf *buf);
private:
    xxio *io;
    xxbuf *hs_buf;
    int state_;
    int epoch;
    std::deque<xxbuf *> out;


    void handle_rtmp_other_event();
};

#endif //XXIPCAMERA_RTMP_CPP_H

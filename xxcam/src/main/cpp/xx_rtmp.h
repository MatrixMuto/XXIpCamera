//
// Created by wq1950 on 17-4-18.
//

#ifndef XXIPCAMERA_RTMP_CPP_H
#define XXIPCAMERA_RTMP_CPP_H

#include "xx_core.h"
#include "xx_io.h"
#include "xx_amf.h"
#include "xx_stream.h"

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


class XXRtmp {
public:
    XXRtmp();

    ~XXRtmp();

    int CreateSession(const char *string);

    void video(uint8_t *pos, int n, int flag, long long int ts);

    static void OnConnect(event *ev);

    static void HandshakeSend(event *ev);

    static void HandshakeRecv(event *ev);

    static void RtmpSend(event *wev);

    static void RtmpRecv(event *rev);

    static ngx_chain_t *alloc_amf_buf(void *arg);

    void audio(uint8_t *string, int i, int i1, long long int i2);

private:
    void SendChallenge();

    void FiniliazeSession();

    void handshake_send(event *ev);

    void handshake_recv(event *rev);

    void handshake_done();

    void fill_random_buffer(xxbuf *b);

    void rtmp_send(event *wev);

    void Recv(event *rev);

    int create_response();

    int handshake_create_challenge(const u_char version[4]);

    void SendChunkSize(int chunksize);

    void prepare_message(rtmp_header *h, rtmp_header *lh, std::list<xxbuf *> &out);

    void handle_rtmp_other_event();

    void SendAckWindowSize(int ack_size);

    void SendConnect();

    void send_amf(rtmp_header *pHeader, XXAmf *amf);

    void send_message(std::list<xxbuf *> &out2);

    int receive_message(xx_stream *pStream);

    void amf_message_handle(xx_stream *stream);

    void send_create_stream();

    void on_result(XXAmf *pAmf);

    void on_error(XXAmf *pAmf);

    void send_publish();

    void on_status(XXAmf *pAmf);

    void protocol_message_handler();

    void sendVideo();

    void send_metadata();

private:
    xxio *io;
    xxbuf *hs_buf;
    int state_;
    int epoch;
    std::list<xxbuf *> out;
    sem_t sem_out;
    xx_stream *in_streams;
    int in_csid;


    size_t in_chunk_size;
    uint64_t in_bytes;
    bool can_publish_;
    int64_t start_;
    size_t out_chunk_size_;

    int parse(const char *url);

    std::string ip_;
    std::string app_;
    std::string name_;
};

#endif //XXIPCAMERA_RTMP_CPP_H

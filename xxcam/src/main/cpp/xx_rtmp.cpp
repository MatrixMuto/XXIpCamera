//
// Created by wq1950 on 17-4-18.
//

#include <cstdlib>
#include "xx_rtmp.h"
#include "xx_amf.h"


#define RTMP_HANDSHAKE_BUFSIZE                  1537

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
    /* Set io the handler*/
    io->SetReadHandler(HandshakeRecv, this);
    io->SetWriteHandler(HandshakeSend, this);


    state_ = NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE;

//    static const u_char
//            ngx_rtmp_client_version[4] = {
//            0x0C, 0x00, 0x0D, 0x0E
//    };
    static const u_char
            ngx_rtmp_client_version[4] = {
            0x00, 0x00, 0x00, 0x00
    };

    hs_buf = new xxbuf(RTMP_HANDSHAKE_BUFSIZE);

    handshake_create_challenge(ngx_rtmp_client_version);

    io->HandleWriteEvnet(1);
}

/* Callback from io thread. */

void XXRtmp::OnConnect(event *ev) {
    LOGI("OnConnect r:%d w:%d", ev->read, ev->write);
    XXRtmp *rtmp = (XXRtmp *) ev->data;
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

void XXRtmp::RtmpSend(event *wev) {
    LOGI("HandshakeSend");
    XXRtmp *rtmp = (XXRtmp *) wev->data;
    rtmp->rtmp_send(wev);
}

void XXRtmp::RtmpRecv(event *rev) {
    LOGI("HandshakeSend");
    XXRtmp *rtmp = (XXRtmp *) rev->data;
    rtmp->rtmp_recv(rev);
}

void XXRtmp::handshake_send(event *wev) {

    ssize_t n;
    xxbuf *b = hs_buf;
    while (b->pos != b->last) {
        n = io->Send(wev, b->pos, b->last - b->pos);

        if (n == XX_ERROR) {
            //fatal error, finalize session.
            LOGE("send return %ld", n);
            io->Close();
            return;
        }

        if (n == XX_AGAIN || n == 0) {
            LOGE("send return %ld", n);
            return;
        }
        b->pos += n;
    }

    if (wev->active) {
        io->deleteEvnet(wev);
    }

    ++state_;

    switch (state_) {
        case NGX_RTMP_HANDSHAKE_CLIENT_RECV_CHALLENGE:
            hs_buf->pos = hs_buf->last = hs_buf->start;
            handshake_recv(io->read_);
            break;
        case NGX_RTMP_HANDSHAKE_CLIENT_DONE:
            handshake_done();
            break;
    }
}

void XXRtmp::handshake_recv(event *rev) {

    ssize_t n;
    xxbuf *b = hs_buf;

    while (b->last != b->end) {
        n = io->Recv(rev, b->last, b->end - b->last);

        if (n == XX_ERROR || n == 0) {
            //ngx_rtmp_finalize_session(s);
            LOGI("error , handshake_recv");
            io->Close();
            return;
        }

        if (n == XX_AGAIN) {
//            ngx_add_timer(rev, s->timeout);
//            if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
//                ngx_rtmp_finalize_session(s);
//            }
            LOGI("read again");
            return;
        }

        b->last += n;
    }

    if (rev->active) {
        io->deleteEvnet(rev);
    }

    ++state_;

    switch (state_) {
        case NGX_RTMP_HANDSHAKE_CLIENT_RECV_RESPONSE:
            hs_buf->pos = hs_buf->last = hs_buf->start + 1;
            handshake_recv(rev); // go to recv S2
            break;
        case NGX_RTMP_HANDSHAKE_CLIENT_SEND_RESPONSE:
            create_response();
            handshake_send(io->write_); //go to send C2
            break;
    }
}

void XXRtmp::fill_random_buffer(xxbuf *b) {
    for (; b->last != b->end; ++b->last) {
        *b->last = (u_char) rand();
    }
}

int XXRtmp::handshake_create_challenge(const u_char version[4]) {
    xxbuf *b;
    b = hs_buf;
    b->last = b->pos = b->start;
    *b->last++ = '\x03';
    b->last = xx_cpymem(b->last, &epoch, 4);
    b->last = xx_cpymem(b->last, version, 4);
    fill_random_buffer(b);
//    ++b->pos;
//    if (ngx_rtmp_write_digest(b, key, 0, s->connection->log) != NGX_OK) {
//        return NGX_ERROR;
//    }
//    --b->pos;
    return XX_OK;
}


int XXRtmp::create_response() {
    xxbuf *b;
    u_char *p;

    b = hs_buf;
    b->pos = b->last = b->start + 1;
    fill_random_buffer(b);
//    if (s->hs_digest) {
//        p = b->last - NGX_RTMP_HANDSHAKE_KEYLEN;
//        key.data = s->hs_digest;
//        key.len = NGX_RTMP_HANDSHAKE_KEYLEN;
//        if (ngx_rtmp_make_digest(&key, b, p, p, s->connection->log) != NGX_OK) {
//            return NGX_ERROR;
//        }
//    }

    return XX_OK;
}

void XXRtmp::handshake_done() {
    LOGI("handshake done");

    io->SetReadHandler(RtmpRecv, this);
    io->SetWriteHandler(RtmpSend, this);

//    rtmp_recv(io->read_);
    SendChunkSize(4096);
    SendAckWindowSize(5000000);
    SendConnect();

    rtmp_send(io->write_);
}

void XXRtmp::rtmp_recv(event *rev) {

    for (;;) {
        LOGI("main loop");
        sleep(1);
    }
}

void XXRtmp::rtmp_send(event *wev) {
    xxbuf *b;
    ssize_t n;

    while (!out.empty()) {
        b = out.front();
        n = io->Send(wev, b->pos, b->last - b->pos);

        if (n == XX_AGAIN || n == 0) {

            return;
        }

        if (n < 0) {
            return;
        }

        delete b;
        out.pop_front();
    }

    if (wev->active) {
        io->deleteEvnet(wev);
    }

    handle_rtmp_other_event();

}

void XXRtmp::SendChunkSize(int chunksize) {
    xxbuf *b;
    rtmp_header *h;
    u_char *p;

    h = new rtmp_header();

    b = new xxbuf(8192);

    b->pos += 20;
    b->last = b->pos;

    h->type = NGX_RTMP_MSG_CHUNK_SIZE;
    h->csid = 2;

    *(b->last++) = ((u_char *) &chunksize)[3];
    *(b->last++) = ((u_char *) &chunksize)[2];
    *(b->last++) = ((u_char *) &chunksize)[1];
    *(b->last++) = ((u_char *) &chunksize)[0];

    prepare_message(h, NULL, b);

    send_message(b);
}

void XXRtmp::SendAckWindowSize(int ack_size) {
    xxbuf *b;
    rtmp_header *h;
    u_char *p;

    h = new rtmp_header();

    b = new xxbuf(8192);

    b->pos += 20;
    b->last = b->pos;

    h->type = NGX_RTMP_MSG_ACK_SIZE;
    h->csid = 2;

    *(b->last++) = ((u_char *) &ack_size)[3];
    *(b->last++) = ((u_char *) &ack_size)[2];
    *(b->last++) = ((u_char *) &ack_size)[1];
    *(b->last++) = ((u_char *) &ack_size)[0];

    prepare_message(h, NULL, b);

    send_message(b);
}

void XXRtmp::send_message(xxbuf *b) {
    out.push_back(b);
}

void XXRtmp::prepare_message(rtmp_header *h, rtmp_header *lh, xxbuf *buf) {

    u_char *p, *pp;//= buf->pos;
    uint8_t fmt;
    uint32_t mlen, timestamp, ext_timestamp;
    static uint8_t hdrsize[] = {12, 8, 4, 1}; /* basic + message */
    int hsize, thsize;
    u_char th[7];



    /* detect packet size */
    mlen = 0;
//    nbufs = 0;
//    for(l = out; l; l = l->next) {
//        mlen += (l->buf->last - l->buf->pos);
//        ++nbufs;
//    }

    mlen += (buf->last - buf->pos);

    fmt = 0;
    if (lh) {
        /* message's 2,3,... chunk */
    } else {
        timestamp = h->timestamp;
    }

    hsize = hdrsize[fmt];

    ext_timestamp = 0;
    if (timestamp >= 0x00ffffff) {
        ext_timestamp = timestamp;
        timestamp = 0x00ffffff;
        hsize += 4;
    }

    if (h->csid >= 64) {
        ++hsize;
        if (h->csid >= 320) {
            ++hsize;
        }
    }

    buf->pos -= hsize;
    p = buf->pos;

    /* basic header */
    *p = (fmt << 6);
    if (h->csid >= 2 && h->csid <= 63) {
        *p++ |= (((uint8_t) h->csid) & 0x3f);
    } else if (h->csid >= 64 && h->csid < 320) {
        ++p;
        *p++ = (uint8_t) (h->csid - 64);
    } else {
        *p++ |= 1;
        *p++ = (uint8_t) (h->csid - 64);
        *p++ = (uint8_t) ((h->csid - 64) >> 8);
    }

    /* create fmt3 header for successive fragments */
    thsize = p - buf->pos;
    memcpy(th, buf->pos, thsize);
    th[0] |= 0xc0;

    /* message header */
    if (fmt <= 2) {
        pp = (u_char *) &timestamp;
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];
        if (fmt <= 1) {
            pp = (u_char *) &mlen;
            *p++ = pp[2];
            *p++ = pp[1];
            *p++ = pp[0];
            *p++ = h->type;
            if (fmt == 0) {
                pp = (u_char *) &h->msid;
                *p++ = pp[0];
                *p++ = pp[1];
                *p++ = pp[2];
                *p++ = pp[3];
            }
        }
    }

    /* extended header */
    if (ext_timestamp) {
        pp = (u_char *) &ext_timestamp;
        *p++ = pp[3];
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];

        /* This CONTRADICTS the standard
         * but that's the way flash client
         * wants data to be encoded;
         * ffmpeg complains */
//        if (cscf->play_time_fix) {
//            ngx_memcpy(&th[thsize], p - 4, 4);
//            thsize += 4;
//        }
    }

//    /* append headers to successive fragments */
//    for(out = out->next; out; out = out->next) {
//        out->buf->pos -= thsize;
//        memcpy(out->buf->pos, th, thsize);
//    }

}

void XXRtmp::handle_rtmp_other_event() {

}

void XXRtmp::SendConnect() {
#if 0
    static double trans = NGX_RTMP_RELAY_CONNECT_TRANS;
    static double acodecs = 3575;
    static double vcodecs = 252;

    static ngx_rtmp_amf_elt_t out_cmd[] = {

            {NGX_RTMP_AMF_STRING,
                    ngx_string("app"),
                    NULL,     0}, /* <-- fill */

            {NGX_RTMP_AMF_STRING,
                    ngx_string("tcUrl"),
                    NULL,     0}, /* <-- fill */

            {NGX_RTMP_AMF_STRING,
                    ngx_string("pageUrl"),
                    NULL,     0}, /* <-- fill */

            {NGX_RTMP_AMF_STRING,
                    ngx_string("swfUrl"),
                    NULL,     0}, /* <-- fill */

            {NGX_RTMP_AMF_STRING,
                    ngx_string("flashVer"),
                    NULL,     0}, /* <-- fill */

            {NGX_RTMP_AMF_NUMBER,
                    ngx_string("audioCodecs"),
                    &acodecs, 0},

            {NGX_RTMP_AMF_NUMBER,
                    ngx_string("videoCodecs"),
                    &vcodecs, 0}
    };

    static ngx_rtmp_amf_elt_t out_elts[] = {

            {NGX_RTMP_AMF_STRING,
                    ngx_null_string,
                    "connect", 0},

            {NGX_RTMP_AMF_NUMBER,
                    ngx_null_string,
                    &trans,    0},

            {NGX_RTMP_AMF_OBJECT,
                    ngx_null_string,
                    out_cmd,   sizeof(out_cmd)}
    };

    rtmp_header h;
    size_t len, url_len;
    u_char *p, *url_end;

    /* app */
    if (ctx->app.len) {
        out_cmd[0].data = ctx->app.data;
        out_cmd[0].len = ctx->app.len;
    } else {
        out_cmd[0].data = cacf->name.data;
        out_cmd[0].len = cacf->name.len;
    }

    /* tcUrl */
    if (ctx->tc_url.len) {
        out_cmd[1].data = ctx->tc_url.data;
        out_cmd[1].len = ctx->tc_url.len;
    } else {
        len = sizeof("rtmp://") - 1 + ctx->url.len +
              sizeof("/") - 1 + ctx->app.len;
        p = ngx_palloc(s->connection->pool, len);
        if (p == NULL) {
            return NGX_ERROR;
        }
        out_cmd[1].data = p;
        p = ngx_cpymem(p, "rtmp://", sizeof("rtmp://") - 1);

        url_len = ctx->url.len;
        url_end = ngx_strlchr(ctx->url.data, ctx->url.data + ctx->url.len, '/');
        if (url_end) {
            url_len = (size_t) (url_end - ctx->url.data);
        }

        p = ngx_cpymem(p, ctx->url.data, url_len);
        *p++ = '/';
        p = ngx_cpymem(p, ctx->app.data, ctx->app.len);
        out_cmd[1].len = p - (u_char *) out_cmd[1].data;
    }

    /* pageUrl */
    out_cmd[2].data = ctx->page_url.data;
    out_cmd[2].len = ctx->page_url.len;

    /* swfUrl */
    out_cmd[3].data = ctx->swf_url.data;
    out_cmd[3].len = ctx->swf_url.len;

    /* flashVer */
    if (ctx->flash_ver.len) {
        out_cmd[4].data = ctx->flash_ver.data;
        out_cmd[4].len = ctx->flash_ver.len;
    } else {
        out_cmd[4].data = NGX_RTMP_RELAY_FLASHVER;
        out_cmd[4].len = sizeof(NGX_RTMP_RELAY_FLASHVER) - 1;
    }

    ngx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF_INI;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    ngx_rtmp_send_amf(s, &h, out_elts, sizeof(out_elts) / sizeof(out_elts[0]));
#endif
}

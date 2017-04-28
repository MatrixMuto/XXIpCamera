//
// Created by wq1950 on 17-4-27.
//
#include "xx_core.h"
#include "xx_rtmp.h"


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

void XXRtmp::handshake_send(event *wev) {

    ssize_t n;
    xxbuf *b = hs_buf;
    while (b->pos != b->last) {
        n = io->Send(wev, b->pos, b->last - b->pos);

        if (n == XX_ERROR) {
            LOGE("send return %ld", n);
            FiniliazeSession();
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
//
// Created by wq1950 on 17-4-18.
//


#include "xx_rtmp.h"
#include "xx_amf.h"


#define NGX_RTMP_RELAY_FLASHVER                 "LNX.11,1,102,55"

#define NGX_RTMP_RELAY_CONNECT_TRANS            1
#define NGX_RTMP_RELAY_CREATE_STREAM_TRANS      2


#define NGX_RTMP_RELAY_CSID_AMF_INI             3
#define NGX_RTMP_RELAY_CSID_AMF                 5
#define NGX_RTMP_RELAY_MSID                     1

XXRtmp::XXRtmp() {
    io = new xxio();
}

XXRtmp::~XXRtmp() {
    delete io;
}

int XXRtmp::CreateSession() {
    std::string url = "rtmp://127.0.0.1:1935/live/test";

    io->Connect(url, OnConnect, this);

    SendChallenge();

    io->start();

    return 0;
}

void XXRtmp::FiniliazeSession() {

}

void XXRtmp::video(uint8_t *data) {
    io->Write(data);
}

/* Callback from io thread. */

void XXRtmp::OnConnect(event *ev) {
    LOGI("OnConnect r:%d w:%d", ev->read, ev->write);
    XXRtmp *rtmp = (XXRtmp *) ev->data;
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



void XXRtmp::handshake_done() {
    LOGI("handshake done");

    io->SetReadHandler(RtmpRecv, this);
    io->SetWriteHandler(RtmpSend, this);

    SendChunkSize(4096);
    SendAckWindowSize(5000000);
    SendConnect();

    rtmp_send(io->write_);
    rtmp_recv(io->read_);
}

void XXRtmp::rtmp_recv(event *rev) {
    ssize_t n;
    xxbuf buf(128);
    xxbuf *b = &buf;
    u_char *p;
    uint8_t fmt, ext;
    uint32_t csid, timestamp;

    for (;;) {
        LOGI("main loop");

        n = io->Recv(rev, b->last, b->end - b->last);

        if (n < 0 || n == 0) {
            LOGE("error");
            FiniliazeSession();
            return;
        }

        if (n == XX_AGAIN) {
            io->HandleReadEvnet(0);
            return;
        }

        b->last += n;

        if (b->pos == b->start) {
            p = b->pos;

            /* chunk basic header */
            fmt = (*p >> 6) & 0x03;
            csid = *p++ & 0x3f;

            if (csid == 0) {
                if (b->last - p < 1)
                    continue;
                csid = 64;
                csid += *(uint8_t *) p++;

            } else if (csid == 1) {
                if (b->last - p < 2)
                    continue;
                csid = 64;
                csid += *(uint8_t *) p++;
                csid += (uint32_t) 256 * (*(uint8_t *) p++);
            }


        }

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
            LOGE("rtmp_send again");
            return;
        }

        if (n < 0) {
            LOGE("rtmp_send err");
            return;
        }

        out.pop_front();
    }

    if (wev->active) {
        io->deleteEvnet(wev);
    }

    handle_rtmp_other_event();

}

void XXRtmp::SendChunkSize(int chunksize) {
    std::list<xxbuf *> out;
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

    out.push_back(b);
    prepare_message(h, NULL, out);
    send_message(out);
}

void XXRtmp::SendAckWindowSize(int ack_size) {
    std::list<xxbuf *> out;
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

    out.push_back(b);
    prepare_message(h, NULL, out);
    send_message(out);
}

void XXRtmp::send_message(std::list<xxbuf *> &out2) {
    out.merge(out2);
}

void XXRtmp::prepare_message(rtmp_header *h, rtmp_header *lh, std::list<xxbuf *> &out) {

    u_char *p, *pp;//= buf->pos;
    uint8_t fmt;
    uint32_t mlen, timestamp, ext_timestamp;
    static uint8_t hdrsize[] = {12, 8, 4, 1}; /* basic + message */
    int hsize, thsize;
    u_char th[7];
    std::list<xxbuf *>::iterator it;
    xxbuf *buf;

    /* detect packet size */
    mlen = 0;
    int nbufs = 0;
    for (it = out.begin(); it != out.end(); ++it) {

        mlen += ((*it)->last - (*it)->pos);
        ++nbufs;
    }

    buf = out.front();

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
    static double trans = NGX_RTMP_RELAY_CONNECT_TRANS;
    static double acodecs = 3575;
    static double vcodecs = 252;

    XXAmf *obj = new XXAmf();
    obj->push_back(XXAmfElt(XX_RTMP_AMF_STRING, "app", (void *) "live", 0));
    obj->push_back(XXAmfElt(XX_RTMP_AMF_STRING, "tcUrl", (void *) "www.xxx.com", 0));
    obj->push_back(XXAmfElt(XX_RTMP_AMF_STRING, "pageUrl", (void *) "www.xxx.com", 0));
    obj->push_back(XXAmfElt(XX_RTMP_AMF_STRING, "swfUrl", (void *) "www.xxx.com", 0));
    obj->push_back(XXAmfElt(XX_RTMP_AMF_STRING, "flashVer", (void *) NGX_RTMP_RELAY_FLASHVER, 0));
    obj->push_back(XXAmfElt(XX_RTMP_AMF_NUMBER, "audioCodecs", &acodecs, 0));
    obj->push_back(XXAmfElt(XX_RTMP_AMF_NUMBER, "videoCodecs", &vcodecs, 0));

    XXAmf *root = new XXAmf();
    root->push_back({XX_RTMP_AMF_STRING, "", (void *) "connect", 0}); //name property is ignored.
    root->push_back({XX_RTMP_AMF_NUMBER, "", &trans, 0});
    root->push_back({XX_RTMP_AMF_OBJECT, "", obj, 0});

    rtmp_header h;

    xx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF_INI;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    send_amf(&h, root);
}

void XXRtmp::send_amf(rtmp_header *h, XXAmf *amf) {
    ngx_int_t rc;
    std::list<xxbuf *> t;

    rc = amf->Write(&t);

    if (rc) {
        LOGE("amf write error");
        return;
    }
    prepare_message(h, NULL, t);
    send_message(t);
}


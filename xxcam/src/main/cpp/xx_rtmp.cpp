//
// Created by wq1950 on 17-4-18.
//


#include "xx_rtmp.h"
#include "xx_stream.h"
#include "xx_amf.h"


#define NGX_RTMP_RELAY_FLASHVER                 "LNX.11,1,102,55"

#define NGX_RTMP_RELAY_CONNECT_TRANS            1
#define NGX_RTMP_RELAY_CREATE_STREAM_TRANS      2


#define NGX_RTMP_RELAY_CSID_AMF_INI             3
#define NGX_RTMP_RELAY_CSID_AMF                 5
#define NGX_RTMP_RELAY_MSID                     1

XXRtmp::XXRtmp() {
    io = new xxio();
    in_chunk_size = 128;
    in_streams = new xx_stream[128];
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
    LOGE("Oh My God\n\t*\n\t*\n\t*\n");
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
    LOGI("RtmpSend");
    XXRtmp *rtmp = (XXRtmp *) wev->data;
    rtmp->rtmp_send(wev);
}

void XXRtmp::RtmpRecv(event *rev) {
    LOGI("RtmpRecv");
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
    LOGI("rtmp_recv called enter");
    ssize_t         n;
    xxbuf           *b = new xxbuf(5000);
    u_char          *p, *pp, *old_pos;
    uint8_t         fmt, ext;
    uint32_t        csid, timestamp;
    rtmp_header     *h;
    size_t          size, fsize;
    xx_stream       *stream;
    size_t          old_size;
    int rc;

    old_pos = NULL;
    old_size = 0;

    for (;;) {
        LOGI("main loop");

        stream = &in_streams[in_csid];

        if (old_size) {

            b->pos = b->start;
            b->last = xx_movemem(b->pos, old_pos, old_size);

        } else {
            n = io->Recv(rev, b->last, b->end - b->last);
            LOGI(">>>>recv %ld", n);
            if (n == XX_ERROR || n == 0) {
                LOGE("rtmp_recv error n %ld", n);
                FiniliazeSession();
                return;
            }

            if (n == XX_AGAIN) {
                LOGE("rtmp_recv aggin");
                io->HandleReadEvnet(0);
                return;
            }

            b->last += n;
        }

        old_pos = NULL;
        old_size = 0;

        if (b->pos == b->start) {
            p = stream->ParseHeader(b->pos, b->last);
            if (!p) {
                continue;
            }
            /* header done */
            b->pos = p;
        }

        size = b->last - b->pos; /* current buf 's data*/
        fsize = stream->header_.mlen - stream->len_; /* current msg still need*/

        /*fill one chunk or fill all message*/
        if (size < ngx_min(fsize, in_chunk_size))
            continue;

        if (fsize > in_chunk_size) {
            /* message not complete*/
            stream->len_ += in_chunk_size;
            b->last = b->pos + in_chunk_size;
            old_pos = b->last;
            old_size = size - in_chunk_size;

        } else {

            b->last = b->pos + fsize;
            old_pos = b->last;
            old_size = size - fsize;
            stream->len_ = 0;

            if (receive_message(stream) != XX_OK) {
                FiniliazeSession();
                return;
            }
        }

        in_csid = 0;
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
    if ( !io->write_->active) {
        rtmp_send(io->write_);
    }
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

int XXRtmp::receive_message(xx_stream *pStream) {
    LOGI("receive message ing");
    rtmp_header *h = &pStream->header_;
    if ( h->type == NGX_RTMP_MSG_CHUNK_SIZE) {
        in_chunk_size = 4096;
    } else if (h->type == NGX_RTMP_MSG_AMF_CMD) {
        amf_message_handle(pStream);
    }
    return 0;
}

void XXRtmp::amf_message_handle(xx_stream *stream) {
    static int i = 0;
    if (i==0) {
        send_create_stream();
        i = 1;
    }
    else if (i==1) {

    }
}

void XXRtmp::send_create_stream() {
    static double trans = NGX_RTMP_RELAY_CREATE_STREAM_TRANS;

    XXAmf *create_stream = new XXAmf();
    create_stream->push_back( {XX_RTMP_AMF_STRING, "", (void*)"createStream", 0});
    create_stream->push_back( {XX_RTMP_AMF_NUMBER, "", (void*)&trans, 0});
    create_stream->push_back( {XX_RTMP_AMF_NULL, "", NULL, 0});

    rtmp_header           h;

    xx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF_INI;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    return send_amf(&h, create_stream);
}


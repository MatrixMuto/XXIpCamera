//
// Created by wq1950 on 17-4-18.
//


#include "xx_core.h"


#define NGX_RTMP_RELAY_FLASHVER                 "LNX.11,1,102,55"

#define NGX_RTMP_RELAY_CONNECT_TRANS            1
#define NGX_RTMP_RELAY_CREATE_STREAM_TRANS      2

#define NGX_RTMP_MSID                   1
#define NGX_RTMP_RELAY_CSID_AMF_INI             3
#define NGX_RTMP_RELAY_CSID_AMF                 5
#define NGX_RTMP_CSID_AUDIO             6
#define NGX_RTMP_CSID_VIDEO             7
#define NGX_RTMP_RELAY_MSID                     1


/* Called in User thread */
XXRtmpImpl::XXRtmpImpl() {
    io = new xxio();
    in_chunk_size = 128;
    in_streams = new xx_stream[128];
    can_publish_ = false;
    out_chunk_size_ = 4096;
    sem_init(&sem_out, 0, 1);
}

XXRtmpImpl::~XXRtmpImpl() {
    delete io;
}

int XXRtmpImpl::CreateSession(const char *url) {
    int err;
    in_csid = 0;

    err = parse(url);
    if (err) {
        return err;
    }

    io->Connect(ip_, port_, OnConnect, this);

    SendChallenge();

    io->Start();

    return XX_OK;
}

void XXRtmpImpl::audio(uint8_t *data, int n, int flag, long long int ts) {
    if (can_publish_) {
        int copy;
        std::list<xxbuf *> out;
        xxbuf *buf;
        int time = 0;
        uint8_t *pos, *end;

        buf = new xxbuf(20 + out_chunk_size_);
        buf->pos += 20;
        buf->last += 20;

        u_char type = 10;
        type = (type << 4) | (3 << 2) | (1 << 1) | 1;
        u_char avc_packet_type = flag == 2 ? 0 : 1;

        buf->last = xx_cpymem(buf->last, &type, 1);
        buf->last = xx_cpymem(buf->last, &avc_packet_type, 1);
        pos = data;
        end = data + n;
        for (;;) {
            copy = ngx_min(end - pos, buf->end - buf->last);
            buf->last = xx_cpymem(buf->last, pos, copy);
            pos += copy;
            out.push_back(buf);
            if (pos < end) {
                buf = new xxbuf(20 + out_chunk_size_);
                buf->pos += 20;
                buf->last = buf->pos;
            } else {
                break;
            }
        }

        rtmp_header h;
        h.msid = NGX_RTMP_MSID;
//        h.timestamp = ts - start_;
        h.csid = NGX_RTMP_CSID_AUDIO;
        h.type = NGX_RTMP_MSG_AUDIO;

        prepare_message(&h, NULL, out);
        send_message(out);
    }
}

void XXRtmpImpl::video(uint8_t *data, int n, int flag, long long int ts) {

    if (can_publish_) {
        int copy;
        std::list<xxbuf *> out;
        xxbuf *buf;
        int time = 0;
        uint8_t *pos, *end;

        buf = new xxbuf(20 + out_chunk_size_);
        buf->pos += 20;
        buf->last += 20;
        u_char type = flag & 0x1 ? 1 : 2;
        type = (type << 4) | 7;
        u_char avc_packet_type = flag == 2 ? 0 : 1;

        buf->last = xx_cpymem(buf->last, &type, 1);
        buf->last = xx_cpymem(buf->last, &avc_packet_type, 1);
        buf->last = xx_cpymem(buf->last, (((u_char *) &time)), 3);

        pos = data;
        end = data + n;
        for (;;) {
            copy = ngx_min(end - pos, buf->end - buf->last);
            buf->last = xx_cpymem(buf->last, pos, copy);
            pos += copy;
            out.push_back(buf);
            if (pos < end) {
                buf = new xxbuf(20 + out_chunk_size_);
                buf->pos += 20;
                buf->last = buf->pos;
            } else {
                break;
            }
        }

        rtmp_header h;
        h.msid = NGX_RTMP_MSID;
//        h.timestamp = ts - start_;
        h.csid = NGX_RTMP_CSID_VIDEO;
        h.type = NGX_RTMP_MSG_VIDEO;

        prepare_message(&h, NULL, out);
        send_message(out);
    }
}

/* Callback from io thread. */

void XXRtmpImpl::FiniliazeSession() {
    LOGE("Oh My God\n\t*\n\t*\n\t*\n");
    can_publish_ = false;
    if (io->read_->active) {
        io->DeleteEvnet(io->read_);
    }
    if (io->read_->active) {
        io->DeleteEvnet(io->read_);
    }
}

void XXRtmpImpl::OnConnect(event *ev) {
    LOGI("OnConnect r:%d w:%d", ev->read, ev->write);
    XXRtmpImpl *rtmp = (XXRtmpImpl *) ev->data;
}

void XXRtmpImpl::RtmpSend(event *wev) {
    LOGI("RtmpSend");
    XXRtmpImpl *rtmp = (XXRtmpImpl *) wev->data;
    rtmp->rtmp_send(wev);
}

void XXRtmpImpl::RtmpRecv(event *rev) {
    LOGI("RtmpRecv");
    XXRtmpImpl *rtmp = (XXRtmpImpl *) rev->data;
    rtmp->Recv(rev);
}


void XXRtmpImpl::handshake_done() {
    LOGI("handshake done");

    io->SetReadHandler(RtmpRecv, this);
    io->SetWriteHandler(RtmpSend, this);

    SendChunkSize(out_chunk_size_);
    SendAckWindowSize(5000000);
    SendConnect();

    rtmp_send(io->write_);
    Recv(io->read_);
}

void XXRtmpImpl::Recv(event *rev) {
    LOGI("Recv called enter");
    ssize_t n;
    xxbuf *b = new xxbuf(5000);
    u_char *p, *pp, *old_pos;
    uint8_t fmt, ext;
    uint32_t csid, timestamp;
    rtmp_header *h;
    size_t size, fsize;
    xx_stream *stream;
    size_t old_size;
    int rc;

    old_pos = NULL;
    old_size = 0;

    for (;;) {

        stream = &in_streams[in_csid];

        if (stream->some_.empty()) {
            xxbuf *bb = new xxbuf(4096 + 20);
            stream->some_.push_back(bb);
        }

        b = stream->some_.back();

        if (old_size) {

            b->pos = b->start;
            b->last = xx_movemem(b->pos, old_pos, old_size);

        } else {
            n = io->Recv(rev, b->last, b->end - b->last);
            LOGI("recv %ld", n);
            if (n == XX_ERROR || n == 0) {
                LOGE("Recv error n %ld", n);
                FiniliazeSession();
                return;
            }

            if (n == XX_AGAIN) {
                LOGE("Recv aggin");
                io->HandleReadEvnet(0);
                return;
            }

            b->last += n;
            in_bytes += n;

//            if (s->in_bytes >= 0xf0000000) {
//                ngx_log_debug0(NGX_LOG_DEBUG_RTMP, c->log, 0,
//                               "resetting byte counter");
//                s->in_bytes = 0;
//                s->in_last_ack = 0;
//            }
//
//            if (s->ack_size && s->in_bytes - s->in_last_ack >= s->ack_size) {
//
//                s->in_last_ack = s->in_bytes;
//
//                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, c->log, 0,
//                               "sending RTMP ACK(%uD)", s->in_bytes);
//int
//                if (ngx_rtmp_send_ack(s, s->in_bytes)) {
//                    ngx_rtmp_finalize_session(s);
//                    return;
//                }
//            }
        }

        old_pos = NULL;
        old_size = 0;

        /* if parsing pos is in zero, need parse header, else just recv chunk data.*/
        if (b->pos == b->start) {
            p = b->pos;
            if (stream->ParseChunkStreamId(p, b->last, &fmt, &csid) == XX_AGAIN) {
                continue;
            }

            if (csid > 65536) {
                LOGE("max csid");
                FiniliazeSession();
                return;
            }

            if (in_csid == 0) {
                stream->some_.pop_back();

                in_csid = csid;
                stream = &in_streams[in_csid];
                stream->some_.push_back(b);
            }

            if (stream->ParseHeader(fmt, p, b->last) == XX_AGAIN) {
                continue;
            }

            b->pos = p;
            /* header done */

            if (stream->header_.mlen > 19999) {
                LOGE("too big message");
                FiniliazeSession();
                return;
            }
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
            /* handle message*/
            b->last = b->pos + fsize;
            old_pos = b->last;
            old_size = size - fsize;
            stream->len_ = 0;

            LOGI("oldsize %u", old_size);
            if (receive_message(stream) != XX_OK) {
                FiniliazeSession();
                return;
            }

            if (0) {

            } else {
                /*add used buf to stream0*/
                xx_stream *st0 = &in_streams[0];

                stream->some_.clear();
            }
        }

        in_csid = 0;
    }
}

void XXRtmpImpl::rtmp_send(event *wev) {
    xxbuf *b;
    ssize_t n;

    while (!out.empty()) {
        b = out.front();
        n = io->Send(wev, b->pos, b->last - b->pos);

        if (n == XX_AGAIN || n == 0) {
            LOGE("rtmp_send again");
            io->HandleWriteEvnet(0);
            return;
        }

        if (n < 0) {
            LOGE("rtmp_send err");
            FiniliazeSession();
            return;
        }

        b->pos += n;
        if (b->pos == b->last) {
            sem_wait(&sem_out);
            out.pop_front();
            sem_post(&sem_out);
        }
    }

    if (wev->active) {
        io->DeleteEvnet(wev);
    }

    handle_rtmp_other_event();

}

void XXRtmpImpl::SendChunkSize(int chunksize) {
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

void XXRtmpImpl::SendAckWindowSize(int ack_size) {
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

void XXRtmpImpl::send_message(std::list<xxbuf *> &out2) {
    sem_wait(&sem_out);
    out.merge(out2);
    sem_post(&sem_out);
    if (!io->write_->active) {
        rtmp_send(io->write_);
    }
}

void XXRtmpImpl::prepare_message(rtmp_header *h, rtmp_header *lh, std::list<xxbuf *> &out) {

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
    it = out.begin();
    it++;
    for (; it != out.end(); ++it) {
        (*it)->pos -= thsize;
        memcpy((*it)->pos, th, thsize);
    }
}

void XXRtmpImpl::handle_rtmp_other_event() {

}


int XXRtmpImpl::receive_message(xx_stream *stream) {
    LOGI("receive message ing");
    rtmp_header *h = &stream->header_;

    switch (h->type) {
        case NGX_RTMP_MSG_CHUNK_SIZE:
        case NGX_RTMP_MSG_ABORT:
        case NGX_RTMP_MSG_ACK:
        case NGX_RTMP_MSG_ACK_SIZE:
        case NGX_RTMP_MSG_BANDWIDTH:
            protocol_message_handler();
            break;
    }

    if (h->type == NGX_RTMP_MSG_CHUNK_SIZE) {
        in_chunk_size = 4000;
    } else if (h->type == NGX_RTMP_MSG_AMF_CMD) {
        amf_message_handle(stream);
    }
    return 0;
}

void XXRtmpImpl::amf_message_handle(xx_stream *stream) {
    static int i = 0;
    LOGI(" stream bufs size %d", stream->some_.size());
    XXAmf *amf = new XXAmf(&stream->some_);
    std::string func;
//    XXAmfElt *elt = new XXAmfElt();
    amf->GetFunc(func);

    if (func == "_result") {
        on_result(amf);
    } else if (func == "_error") {
        on_error(amf);
    } else if (func == "onStatus") {
        on_status(amf);
    }
}


void XXRtmpImpl::protocol_message_handler() {

}

void XXRtmpImpl::on_result(XXAmf *pAmf) {
    LOGI("on_result E");
    double trans;
    pAmf->GetTrans(&trans);
    LOGI("GetTrans trans %f", trans);

    switch ((int) trans) {
        case NGX_RTMP_RELAY_CONNECT_TRANS:
            send_create_stream();
            break;
        case NGX_RTMP_RELAY_CREATE_STREAM_TRANS:
            send_publish();
            break;
    }
    LOGI("on_result X");
}

void XXRtmpImpl::on_error(XXAmf *pAmf) {

}


void XXRtmpImpl::on_status(XXAmf *pAmf) {
    can_publish_ = true;

    send_metadata();
}

void XXRtmpImpl::SendConnect() {
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

void XXRtmpImpl::send_create_stream() {
    static double trans = NGX_RTMP_RELAY_CREATE_STREAM_TRANS;
    LOGI("send_create_stream");
    XXAmf *create_stream = new XXAmf();
    create_stream->push_back({XX_RTMP_AMF_STRING, "", (void *) "createStream", 0});
    create_stream->push_back({XX_RTMP_AMF_NUMBER, "", (void *) &trans, 0});
    create_stream->push_back({XX_RTMP_AMF_NULL, "", NULL, 0});

    rtmp_header h;

    xx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF_INI;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    return send_amf(&h, create_stream);
}

void XXRtmpImpl::send_publish() {
    static double trans;

    XXAmf *publish = new XXAmf();
    publish->push_back({XX_RTMP_AMF_STRING, "", (void *) "publish", 0});
    publish->push_back({XX_RTMP_AMF_NUMBER, "", (void *) &trans, 0});
    publish->push_back({XX_RTMP_AMF_NULL, "", NULL, 0});
    publish->push_back({XX_RTMP_AMF_STRING, "", (void *) "test", 0});
    publish->push_back({XX_RTMP_AMF_STRING, "", (void *) "live", 0});

    rtmp_header h;

    xx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF_INI;
    h.msid = NGX_RTMP_RELAY_MSID;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    return send_amf(&h, publish);
}

void XXRtmpImpl::send_amf(rtmp_header *h, XXAmf *amf) {
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

void XXRtmpImpl::sendVideo() {
    LOGI("sendVideo");
}

void XXRtmpImpl::send_metadata() {

    static struct {
        double width;
        double height;
        double duration;
        double video_codec_id;
        double audio_codec_id;
        double audio_sample_rate;
    } v;


    XXAmf *obj = new XXAmf();
    obj->push_back({XX_RTMP_AMF_NUMBER, "width", (void *) &v.width, 0});
    obj->push_back({XX_RTMP_AMF_NUMBER, "height", (void *) &v.height, 0});
    obj->push_back({XX_RTMP_AMF_NUMBER, "displayWidth", (void *) &v.width, 0});
    obj->push_back({XX_RTMP_AMF_NUMBER, "displayHeight", (void *) &v.height, 0});
    obj->push_back({XX_RTMP_AMF_NUMBER, "duration", (void *) &v.duration, 0});
    obj->push_back({XX_RTMP_AMF_NUMBER, "videocodecid", (void *) &v.audio_codec_id, 0});
    obj->push_back({XX_RTMP_AMF_NUMBER, "audiocodecid", (void *) &v.audio_sample_rate, 0});

    XXAmf *metadata = new XXAmf();
    metadata->push_back({XX_RTMP_AMF_STRING, "", (void *) "onMetaData", 0});
    metadata->push_back({XX_RTMP_AMF_OBJECT, "", (void *) obj, 0});

    v.width = 640;
    v.height = 480;
    v.duration = 0;
    v.audio_codec_id = 1;
    v.video_codec_id = 2;

    rtmp_header h;
    xx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF;
    h.msid = NGX_RTMP_RELAY_MSID;
    h.type = NGX_RTMP_MSG_AMF_META;


    send_amf(&h, metadata);
}

int XXRtmpImpl::parse(const char *url) {
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
    port_ = 1935;
    app_ = app;
    name_ = name;
    LOGI("%s %s %s", ip, app, name);

    return XX_OK;
}



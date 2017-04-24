//
// Created by wq1950 on 17-4-18.
//

#include "xxio.h"
#include "rtmp.h"

#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <android/log.h>

#define  LOG_TAG    "xxio"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#define NGX_TIMER_INFINITE (-1)

xxio::xxio() {

}

xxio::~xxio() {

}

int xxio::Connect(std::string &address, void *callback) {
    int err;
    int flags;
    struct sockaddr_in addr;
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err = errno;
        LOGE("socket err %d", errno);
        return err;
    }

    /* Set socket to non-blocking */
    if ((flags = fcntl(sockfd, F_GETFL, 0)) < 0)
    {
        /* Handle error */
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        /* Handle error */
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.1.111");
    addr.sin_port = htons(1935);

    err = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (err == -1) {
        err = errno;
        LOGE("connect err %d", err);
        if (err == EAGAIN || err == EINPROGRESS) {
            LOGE("normal %d", err);
        }
        else {
            LOGE("unnormal %d", err);
        }
    }

    if (err == 0) {

    }

    sockfd_ = sockfd;

    event *ev = new event();
    ev->fd = sockfd;
    ev->write = 1;
    ev->read = 1;
    ev->handler = callback;

    addEvent(ev);
    return 0;
}

int xxio::Write(uint8_t *string)
{
    return 0;
}

int xxio::Read()
{
    return 0;
}

void* xxio::loop_enter(void *data)
{
    xxio *io = (xxio*) data;
    int err ;
    for(;;) {

        err = io->process();

        err = io->select(1000);

        if (err) {
            break;
        }
    }
    return 0;
}

void xxio::start() {

    FD_ZERO(&readset_in_);
    FD_ZERO(&writeset_in_);

    maxfd_ = -1;
    nevents_ = 0;

    int err = pthread_create(&thread_, NULL,  loop_enter, this);
}

int xxio::select(long timer) {
    int err = 0;
    int ready, i;
    int found, nready = 0;
    struct timeval     tv, *tp;

    if (quit_) {
        return 1;
    }

    if (maxfd_ == -1) {

    }

    if (timer == NGX_TIMER_INFINITE) {
        tp = NULL;

    } else {
        tv.tv_sec = (long) (timer / 1000);
        tv.tv_usec = (long) ((timer % 1000) * 1000);
        tp = &tv;
    }

    readset_out_ = readset_in_;
    writeset_out_ = writeset_out_;
    ready = ::select(maxfd_ + 1, &readset_out_, &writeset_out_, NULL, tp);

    err = ready ==-1 ? errno : 0;

    if (err) {
        if (err == EBADF) {

        }

        return err;
    }

    if ( ready == 0) {
        //timeout
        LOGI("timeout timer %ldms", timer);
    }

    for (i = 0; i < nevents_; i++) {
        event *ev = events_[i];
        found = 0;

        if (FD_ISSET(ev->fd, &writeset_out_)){
            found = 1;
        }

        if (FD_ISSET(ev->fd, &readset_out_)){
            found = 1;
        }

        if (found) {
            queue_.push_back(ev);
            LOGI("found %d ", ev->fd);
            nready++;
        }
    }

    if (ready != nready) {

    }

    return err;
}

void xxio::AddSockFd(int fd) {

}

int xxio::process() {

    while (!queue_.empty()) {
        event *ev = queue_.front();
        queue_.pop_front();
//        ev->handle(ev, );
    }
    return 0;
}

void xxio::addEvent(event *ev) {

    LOGI("ev %p nevents_ %d ", ev, nevents_);
    LOGI("ev->fd %d ", ev->fd);

    if (ev->write) {
        FD_SET(ev->fd, &writeset_in_);
    }
    if (ev->read) {
        FD_SET(ev->fd, &readset_in_);
    }

    if (ev->fd > maxfd_ ) {
        maxfd_ = ev->fd;

    }

    events_[nevents_++] = ev;
}


event::event() {

}

event::~event() {

}

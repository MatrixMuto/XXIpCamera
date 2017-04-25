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
    int err, rc;
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

    rc = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (rc == -1) {
        err = errno;
        if (err != EAGAIN && err != EINPROGRESS){
            LOGE("connect err %d", err);

            close(sockfd);
            return err;
        }
    }

    event *rev = new event();
    rev->fd = sockfd;
    rev->read = 1;
    rev->handler = callback;

    addEvent(rev);

    if (rc == -1) {
        /*EINPROGRESS*/
        event *wev = new event();
        wev->fd = sockfd;
        wev->write = 1;
        wev->handler = callback;

        addEvent(wev);
    }

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

    max_fd_ = -1;
    nevents_ = 0;

    int err = pthread_create(&thread_, NULL,  loop_enter, this);
}

int xxio::select(long timer) {
    int err = 0;
    int ready, i;
    int found, nready = 0;
    struct timeval     tv, *tp;
    event *ev;
    if (quit_) {
        return 1;
    }

    if (max_fd_ == -1) {
        for (i = 0; i < nevents_; i++) {
            ev = events_[i];
            if (max_fd_ < ev->fd) {
                max_fd_ = ev->fd;
            }
        }

        LOGI("max fd is %d", max_fd_);

    }

    if (timer == NGX_TIMER_INFINITE) {
        tp = NULL;

    } else {
        tv.tv_sec = (long) (timer / 1000);
        tv.tv_usec = (long) ((timer % 1000) * 1000);
        tp = &tv;
    }

    readset_out_ = readset_in_;
    writeset_out_ = writeset_in_;
    ready = ::select(max_fd_ + 1, &readset_out_, &writeset_out_, NULL, tp);

    err = ready ==-1 ? errno : 0;

    if (err) {
        if (err == EBADF) {

        }

        return err;
    }

    if ( ready == 0) {
        /* timeout */
        LOGI("timeout timer %ldms", timer);
        if (timer != NGX_TIMER_INFINITE) {
            return 0;
        }

        return -1;
    }

    for (i = 0; i < nevents_; i++) {
        ev = events_[i];
        found = 0;

        if (ev->write) {
            if (FD_ISSET(ev->fd, &writeset_out_)){
                found = 1;
            }
        } else {
            if (FD_ISSET(ev->fd, &readset_out_)){
                found = 1;
            }
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

        LOGI("processed event");

//        ev->handler();

        deleteEvnet(ev);
    }
    return 0;
}

void xxio::addEvent(event *ev) {

    LOGI("ev %p nevents_ %d ", ev, nevents_);
    LOGI("ev->fd %d ", ev->fd);

    if (ev->write) {
        FD_SET(ev->fd, &writeset_in_);
    } else {
        FD_SET(ev->fd, &readset_in_);
    }

    if (ev->fd > max_fd_ ) {
        max_fd_ = ev->fd;

    }

    ev->index = nevents_;

    events_[nevents_++] = ev;
}

void xxio::deleteEvnet(event *ev) {
    event *e;
    if (ev->write) {
        FD_CLR(ev->fd, &writeset_in_);
    } else {
        FD_CLR(ev->fd, &readset_in_);
    }

    if (max_fd_ == ev->fd) {
        max_fd_ = -1;
    }

    if (ev->index < --nevents_){
        e = events_[nevents_];
        events_[ev->index] = e;
        e->index = ev->index;
    }
}


event::event() {

}

event::~event() {

}

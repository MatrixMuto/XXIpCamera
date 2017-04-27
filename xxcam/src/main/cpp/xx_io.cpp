//
// Created by wq1950 on 17-4-18.
//

#include "xx_io.h"
#include "xx_rtmp.h"

#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define NGX_TIMER_INFINITE (-1)

xxio::xxio() {

}

xxio::~xxio() {

}

int xxio::Connect(std::string &address, event_handler_pt callback, void *data) {
    int err, rc;
    int flags;
    struct sockaddr_in addr;
    int sockfd;


    FD_ZERO(&readset_in_);
    FD_ZERO(&writeset_in_);

    max_fd_ = -1;
    nevents_ = 0;

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
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
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


    sockfd_ = sockfd;

    read_ = new event();
    read_->data = data;
    read_->read = 1;
    read_->handler = callback;

    addEvent(read_);

    if (rc == -1) {
        /*EINPROGRESS*/
        write_ = new event();
        write_->data = data;
        write_->write = 1;
        write_->handler = callback;

        addEvent(write_);
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

        err = io->Select(1000);

        if (err) {
            break;
        }
    }
    return 0;
}

void xxio::start() {
    int err = pthread_create(&thread_, NULL,  loop_enter, this);
}

int xxio::Select(long timer) {
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
            if (max_fd_ < sockfd_) {
                max_fd_ = sockfd_;
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
            if (FD_ISSET(sockfd_, &writeset_out_)) {
                found = 1;
            }
        } else {
            if (FD_ISSET(sockfd_, &readset_out_)) {
                found = 1;
            }
        }

        if (found) {
            ev->ready = 1;
            queue_.push_back(ev);
            LOGI("found %d ", sockfd_);
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

        ev->handler(ev);
    }
    return 0;
}

void xxio::addEvent(event *ev) {

    LOGI("ev %p nevents_ %d ", ev, nevents_);
    LOGI("ev->fd %d ", sockfd_);

    if (ev->write) {
        FD_SET(sockfd_, &writeset_in_);
    } else {
        FD_SET(sockfd_, &readset_in_);
    }

    if (max_fd_ != -1 && max_fd_ < sockfd_) {
        max_fd_ = sockfd_;
    }


    ev->active = 1;

    events_[nevents_] = ev;
    ev->index = nevents_;
    nevents_++;
}

void xxio::deleteEvnet(event *ev) {
    LOGI("deleteEvnet");
    event *e;

    ev->active = 0;

    if (ev->write) {
        FD_CLR(sockfd_, &writeset_in_);
    } else {
        FD_CLR(sockfd_, &readset_in_);
    }

    if (max_fd_ == sockfd_) {
        max_fd_ = -1;
    }

    if (ev->index < --nevents_){
        e = events_[nevents_];
        events_[ev->index] = e;
        e->index = ev->index;
    }
}

int xxio::Send(event *wev, uint8_t *buf, size_t size) {
    ssize_t n;
    int err;

    for (;;) {
        n = send(sockfd_, buf, size, 0);

        if (n > 0) {

            if (n < (ssize_t) size) {

            }

            return n;
        }

        err = errno;

        if (n == 0) {

            return 0;
        }

        if (err == EAGAIN || err == EINTR) {


            if (err == EAGAIN) {
                return -EAGAIN;
            }

        } else {
            return -1;
        }
    }
}

ssize_t xxio::Recv(event *rev, uint8_t *buf, size_t size) {
    ssize_t n;
    int err;

    do {
        n = recv(sockfd_, buf, size, 0);

        if (n == 0) {
            rev->ready = 0;
            rev->eof = 1;
            LOGE("recv return 0");
            return 0;
        }

        if (n > 0) {
            if ((size_t) n < size) {
                rev->ready = 0;
            }

            return n;
        }

        err = errno;
        if (err == EAGAIN || err == EINTR) {

            n = XX_AGAIN;
        } else {
            n = XX_ERROR;
            break;
        }

    } while (err == EINTR);

    rev->ready = 0;

    if (n == XX_ERROR) {
        rev->error = 1;
    }

    return n;
}

void xxio::HandleWriteEvnet(int write) {
    if (write) {
        if (!write_->ready && !write_->active) {
            addEvent(write_);
        }

        if (write_->active && write_->ready) {
            deleteEvnet(write_);
        }
    }
}

void xxio::SetWriteHandler(event_handler_pt fun, void *pRtmp) {
    write_->data = pRtmp;
    write_->handler = fun;
}

void xxio::SetReadHandler(event_handler_pt fun, void *pRtmp) {
    read_->data = pRtmp;
    read_->handler = fun;
}


void xxio::Close() {
    quit_ = 1;
}

void xxio::HandleReadEvnet(int i) {
    if (!read_->ready && !read_->active) {
        addEvent(read_);
    }

    if (read_->active && read_->ready) {
        deleteEvnet(read_);
    }
}




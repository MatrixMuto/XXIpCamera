//
// Created by wq1950 on 17-4-18.
//

#ifndef XXIPCAMERA_XXIO_H
#define XXIPCAMERA_XXIO_H


#include "xx_core.h"
#include "xx_rtmp.h"

class event;

typedef void (*event_handler_pt)(event *ev);

class event {
public:
    int write;
    event_handler_pt handler;
    int read;
    int index;
    void *data;
    int active;
    int ready;
    int eof;
    int error;
};

class queue {

};

class xxconnection {
public:

private:
    int fd_;
};

class xxio {
public:
    xxio();
    ~xxio();

    int Connect(std::string ip, uint16_t port, event_handler_pt callback, void *data);

    static void* loop_enter(void*);

    void Start();

    void Close();

    ssize_t Recv(event *rev, uint8_t *buf, size_t size);

    ssize_t Send(event *wev, uint8_t *buf, size_t size);

    void SetReadHandler(event_handler_pt fun, void *pRtmp);

    void SetWriteHandler(event_handler_pt fun, void *pRtmp);

    void HandleWriteEvnet(int i);

    void HandleReadEvnet(int i);

    void DeleteEvnet(event *ev);

    int Select(long timer);

private:
    int process();

    void addEvent(event *pEvent);

public:
    event *write_;
    event *read_;

private:
    bool quit_;
    pthread_t thread_;
    int sockfd_;

    fd_set readset_in_;
    fd_set writeset_in_;
    fd_set readset_out_;
    fd_set writeset_out_;

    int max_fd_;

    event* events_[128];
    int nevents_;
    std::deque<event*> queue_;


};

#endif //XXIPCAMERA_XXIO_H

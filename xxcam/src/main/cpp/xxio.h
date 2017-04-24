//
// Created by wq1950 on 17-4-18.
//

#ifndef XXIPCAMERA_XXIO_H
#define XXIPCAMERA_XXIO_H

#include <string>
#include <deque>

#include <sys/select.h>

class event {
public:
    event();
    ~event();
public:
    int fd;
    int write;
    void *handler;
    int read;
};

class queue {

};

class xxio {
public:
    xxio();
    ~xxio();

    int Connect(std::string &address, void *callback);

    static void* loop_enter(void*);

    int select(long timer);

    void start();

    int Write(uint8_t *string);

private:
    void AddSockFd(int fd);

    int Read();
private:
    bool quit_;
    pthread_t thread_;
    int sockfd_;

    fd_set readset_in_;
    fd_set writeset_in_;
    fd_set readset_out_;
    fd_set writeset_out_;

    int maxfd_;

    event* events_[128];
    int nevents_;
    std::deque<event*> queue_;

    int process();

    void addEvent(event *pEvent);
};

class Connection {

};
#endif //XXIPCAMERA_XXIO_H

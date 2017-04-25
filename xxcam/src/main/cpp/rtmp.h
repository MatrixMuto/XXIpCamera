//
// Created by wq1950 on 17-4-18.
//

#ifndef XXIPCAMERA_RTMP_CPP_H
#define XXIPCAMERA_RTMP_CPP_H


#include "xxio.h"

class XXRtmp {
public:
    XXRtmp();
    ~XXRtmp();

    int Connect();

    void video(uint8_t *data);
    static void Handshake(event *ev);

    static void HandshakeRecv(event *ev);

    static void HandshakeSend(event *ev);

    static void OnConnect(event *ev);
private:
    xxio *io;


    void handshake_send(event *ev);

    int state_;

    void SendChallenge();


    void handshake_recv(event *rev);
};

#endif //XXIPCAMERA_RTMP_CPP_H

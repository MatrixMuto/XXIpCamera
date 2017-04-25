//
// Created by wq1950 on 17-4-18.
//

#include "rtmp.h"
#include "xxio.h"

XXRtmp::XXRtmp() {
    io = new xxio();
}

XXRtmp::~XXRtmp() {
    delete io;
}

int XXRtmp::Connect() {

    io->start();
    std::string url = "rtmp://127.0.0.1:1935/live/test";
    //Connection conn = ;
    io->Connect(url, this);

    return 0;
}
//
//int XXRtmp::Send_C0_C1()
//{
//
//}

void XXRtmp::video(uint8_t *data) {
    io->Write(data);
}

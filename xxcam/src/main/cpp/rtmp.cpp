//
// Created by wq1950 on 17-4-18.
//

#include "rtmp.h"
#include "xxio.h"

XXRtmp::XXRtmp() {

}

XXRtmp::~XXRtmp() {

}

int XXRtmp::Connect() {
    std::string url = "rtmp://127.0.0.1:1935/live/test";
    //Connection conn = ;
    xxio::Connect(url, this);


    return 0;
}
//
//int XXRtmp::IoCallback(){
//
//}


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

xxio::xxio() {

}

xxio::~xxio() {

}

int xxio::Connect(std::string &address, XXRtmp *callback) {


    int sockfd = socket(AF_INET, SOCK_STREAM, 0);


//    struct sockaddr_in addr = ;
//    addr.;


//    connect(sockfd, &addr, sizeof(addr));



    close(sockfd);
    return 0;
}


void xxio::loop()
{
    for(;;){
        if (quit) {
            break;
        }

//        select();

    }
}

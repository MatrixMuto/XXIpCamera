#include <iostream>


#include "rtmp.h"
#include "xxio.h"


int main(int argc, char *argv[]) {
    XXRtmp *rtmp = new XXRtmp();

    rtmp->Connect();
    char c;
    while (c = getchar()) {

    }
    return 0;
}

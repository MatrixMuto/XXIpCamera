#include <iostream>


#include "../xx_rtmp.h"
#include "../xx_io.h"


int main(int argc, char *argv[]) {
    XXRtmp *rtmp = new XXRtmp();
    const char *url = "rtmp://127.0.0.1/live/test";
    rtmp->CreateSession(url);
    char c;
    while (c = getchar()) {

    }
    return 0;
}

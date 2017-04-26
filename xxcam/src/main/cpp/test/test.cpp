#include <iostream>


#include "../xx_rtmp.h"
#include "../xx_io.h"


int main(int argc, char *argv[]) {
    XXRtmp *rtmp = new XXRtmp();

    rtmp->Connect();
    char c;
    while (c = getchar()) {

    }
    return 0;
}

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

private:
    xxio *io;
};

#endif //XXIPCAMERA_RTMP_CPP_H

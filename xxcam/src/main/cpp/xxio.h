//
// Created by wq1950 on 17-4-18.
//

#ifndef XXIPCAMERA_XXIO_H
#define XXIPCAMERA_XXIO_H

#include <string>
#include "rtmp.h"

class xxio {
public:
    xxio();
    ~xxio();

    static int Connect(std::string &address, XXRtmp *callback);

    void loop();

private:
    bool quit;
};

class Connection {

};
#endif //XXIPCAMERA_XXIO_H

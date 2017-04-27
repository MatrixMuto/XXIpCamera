//
// Created by wq1950 on 17-4-26.
//

#ifndef PROJECT_XXBUF_H
#define PROJECT_XXBUF_H

#include "xx_core.h"

class xxbuf {
public:
    xxbuf(size_t capbity);

    xxbuf(XXPool *pool);
    ~xxbuf();

public:
    u_char *pos;
    u_char *last;


    u_char *start;
    u_char *end;
};


#endif //PROJECT_XXBUF_H

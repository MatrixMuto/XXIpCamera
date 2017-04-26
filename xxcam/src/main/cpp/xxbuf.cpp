//
// Created by wq1950 on 17-4-26.
//

#include "xxbuf.h"

xxbuf::xxbuf(size_t capbity) {
    start = (u_char *) malloc(capbity);
    end = start + capbity;
    pos = last = start;
}

xxbuf::~xxbuf() {
    if (start) {
        free(start);
    }
}
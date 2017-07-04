//
// Created by muto on 17-4-28.
//

#include "xx_session.h"

XXSessionImpl::XXSessionImpl() {

}

XXSessionImpl::~XXSessionImpl() {

}

XXConnection *XXSessionImpl::CreateConnection(const std::string &url, XXConnectionCallback *cb) {
    XXConnection *c = new XXConnectionImpl(url, cb);
    return c;
}

void XXSessionImpl::Finalize() {

}

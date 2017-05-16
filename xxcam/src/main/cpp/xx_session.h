//
// Created by muto on 17-4-28.
//

#ifndef XXRTMP_XX_SESSION_H
#define XXRTMP_XX_SESSION_H

#include "xx_api.h"
#include "xx_core.h"

class  XXSessionImpl: public XXSession{
public:
    XXSessionImpl();
    virtual ~XXSessionImpl();
    XXConnection* CreateConnection(const std::string& url, XXConnectionCallback *cb) override;
    void Finalize() override;
};

#endif //XXRTMP_XX_SESSION_H

#ifndef CFANUCBOTSOCKET_H
#define CFANUCBOTSOCKET_H

#include "cabstractbotsocket.h"
#include "fanucsocket.h"

class CFanucBotSocket:
        public CAbstractBotSocket
{
public:
    CFanucBotSocket() = default;

    BotSocket::TSocketError startSocket();
    void stopSocket();

private:
    FanucSocket fanuc_socket_;
};

#endif // CFANUCBOTSOCKET_H

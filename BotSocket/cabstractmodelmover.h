#ifndef CABSTRACTMODELMOVER_H
#define CABSTRACTMODELMOVER_H

#include "bot_socket_types.h"

class CAbstractBotSocket;

class CAbstractModelMover
{
    friend class CAbstractBotSocket;

public:
    CAbstractModelMover() { }
    virtual ~CAbstractModelMover() { }

protected:
    virtual void transformModel(const BotSocket::TDistance trX,
                                const BotSocket::TDistance trY,
                                const BotSocket::TDistance trZ,
                                const BotSocket::TDegree rX,
                                const BotSocket::TDegree rY,
                                const BotSocket::TDegree rZ) = 0;

    virtual void socketStateChanged(const BotSocket::TSocketState state) = 0;
};

#endif // CABSTRACTMODELMOVER_H

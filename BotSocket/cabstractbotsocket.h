#ifndef CABSTRACTBOTSOCKET_H
#define CABSTRACTBOTSOCKET_H

#include "bot_socket_types.h"

#include <cstdint>

class CAbstractModelMover;
class CAbstractBotSocketPrivate;

class CAbstractBotSocket
{
public:
    virtual ~CAbstractBotSocket();

    uint32_t getLocalIpV4() const;
    uint16_t getLocalUdpPort() const;
    uint32_t getRemoteBotIpV4() const;
    uint16_t getRemoteBotUdpPort() const;

    void setModelMover(CAbstractModelMover &mover);

    BotSocket::TSocketError start();
    void stop();

    BotSocket::TSocketState state() const;

protected:
    CAbstractBotSocket();

    virtual BotSocket::TSocketError startSocket() = 0;
    virtual void stopSocket() = 0;

    virtual BotSocket::TSocketState socketState() const = 0;

    void transformModel(const BotSocket::TDistance trX,
                        const BotSocket::TDistance trY,
                        const BotSocket::TDistance trZ,
                        const BotSocket::TDegree rX,
                        const BotSocket::TDegree rY,
                        const BotSocket::TDegree rZ);
    void stateChanged(const BotSocket::TSocketState state);

private:
    CAbstractBotSocket(const CAbstractBotSocket &) = delete;
    CAbstractBotSocket& operator=(const CAbstractBotSocket &) = delete;

private:
    CAbstractBotSocketPrivate * const d_ptr;
};

#endif // CABSTRACTBOTSOCKET_H

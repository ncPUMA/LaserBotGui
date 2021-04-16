#ifndef CABSTRACTBOTSOCKET_H
#define CABSTRACTBOTSOCKET_H

#include "bot_socket_types.h"

#include <cstdint>

class MainWindow;
class CAbstractBotSocketPrivate;

class CAbstractBotSocket
{
    friend class MainWindow;

public:
    virtual ~CAbstractBotSocket();

    uint32_t getLocalIpV4() const;
    uint16_t getLocalUdpPort() const;
    uint32_t getRemoteBotIpV4() const;
    uint16_t getRemoteBotUdpPort() const;

    BotSocket::TSocketError start();
    void stop();

protected:
    CAbstractBotSocket();

    virtual BotSocket::TSocketError startSocket() = 0;
    virtual void stopSocket() = 0;

    void translateModel(const BotSocket::TDistance x,
                        const BotSocket::TDistance y,
                        const BotSocket::TDistance z);
    void rotateModel(const BotSocket::TAxisType axis,
                     const BotSocket::TDegree degree);

private:
    CAbstractBotSocket(const CAbstractBotSocket &) = delete;
    CAbstractBotSocket& operator=(const CAbstractBotSocket &) = delete;

private:
    CAbstractBotSocketPrivate * const d_ptr;
    MainWindow *gui;
};

#endif // CABSTRACTBOTSOCKET_H

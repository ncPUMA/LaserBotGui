#include "cabstractbotsocket.h"

using namespace BotSocket;

class CAbstractBotSocketPrivate
{
    friend class CAbstractBotSocket;
};

CAbstractBotSocket::CAbstractBotSocket() :
    d_ptr(new CAbstractBotSocketPrivate())
{

}

void CAbstractBotSocket::translateModel(const TDistance x,
                                        const TDistance y,
                                        const TDistance z)
{
    (void)x;
    (void)y;
    (void)z;
}

void CAbstractBotSocket::rotateModel(const TAxisType axis,
                                     const TDegree degree)
{
    (void)axis;
    (void)degree;
}

CAbstractBotSocket::~CAbstractBotSocket()
{
    delete d_ptr;
}

uint32_t CAbstractBotSocket::getLocalIpV4() const
{
    return 0;
}

uint16_t CAbstractBotSocket::getLocalUdpPort() const
{
    return 0;
}

uint32_t CAbstractBotSocket::getRemoteBotIpV4() const
{
    return 0;
}

uint16_t CAbstractBotSocket::getRemoteBotUdpPort() const
{
    return 0;
}

TSocketError CAbstractBotSocket::start()
{
    return startSocket();
}

void CAbstractBotSocket::stop()
{
    stopSocket();
}

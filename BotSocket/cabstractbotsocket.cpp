#include "cabstractbotsocket.h"

#include "cabstractmodelmover.h"

using namespace BotSocket;

class CEmptyModelMover : public CAbstractModelMover
{
public:
    CEmptyModelMover() : CAbstractModelMover() { }

protected:
    void transformModel(const BotSocket::TDistance,
                        const BotSocket::TDistance,
                        const BotSocket::TDistance,
                        const BotSocket::TDegree,
                        const BotSocket::TDegree,
                        const BotSocket::TDegree) final { }

    void socketStateChanged(const BotSocket::TSocketState) final { }
};



class CAbstractBotSocketPrivate
{
    friend class CAbstractBotSocket;

    CAbstractBotSocketPrivate() :
        mover(&emptyMover) { }

    CEmptyModelMover emptyMover;
    CAbstractModelMover *mover;
};



CAbstractBotSocket::CAbstractBotSocket() :
    d_ptr(new CAbstractBotSocketPrivate())
{

}

void CAbstractBotSocket::transformModel(const TDistance trX,
                                        const TDistance trY,
                                        const TDistance trZ,
                                        const TDegree rX,
                                        const TDegree rY,
                                        const TDegree rZ)
{
    d_ptr->mover->transformModel(trX, trY, trZ, rX, rY, rZ);
}

void CAbstractBotSocket::stateChanged(const TSocketState state)
{
    d_ptr->mover->socketStateChanged(state);
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

void CAbstractBotSocket::setModelMover(CAbstractModelMover &mover)
{
    d_ptr->mover = &mover;
    d_ptr->mover->socketStateChanged(state());
}

TSocketError CAbstractBotSocket::start()
{
    return startSocket();
}

void CAbstractBotSocket::stop()
{
    stopSocket();
}

TSocketState CAbstractBotSocket::state() const
{
    return socketState();
}

#include "cabstractbotsocket.h"

#include "cabstractui.h"
#include "cabstractbotsocketsettings.h"

using namespace BotSocket;

static class CEmptyUi : public CAbstractUi
{
public:
    CEmptyUi() : CAbstractUi() { }

protected:
    void transformModel(const BotSocket::TDistance,
                        const BotSocket::TDistance,
                        const BotSocket::TDistance,
                        const BotSocket::TDegree,
                        const BotSocket::TDegree,
                        const BotSocket::TDegree) final { }

    void socketStateChanged(const BotSocket::TSocketState) final { }
} emptyUi;



static class CEmtySettings : public CAbstractBotSocketSettings
{
public:
    CEmtySettings() : CAbstractBotSocketSettings() { }

    uint32_t getLocalIpV4() const final { return 0; }
    uint16_t getLocalUdpPort() const final { return 0; }
    uint32_t getRemoteBotIpV4() const final { return 0; }
    uint16_t getRemoteBotUdpPort() const final { return 0; }
} emptySettings;



class CAbstractBotSocketPrivate
{
    friend class CAbstractBotSocket;

    CAbstractBotSocketPrivate() :
        gui(&emptyUi),
        settings(&emptySettings) { }

    CAbstractUi *gui;
    CAbstractBotSocketSettings *settings;
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
    d_ptr->gui->transformModel(trX, trY, trZ, rX, rY, rZ);
}

void CAbstractBotSocket::stateChanged(const TSocketState state)
{
    d_ptr->gui->socketStateChanged(state);
}

CAbstractBotSocket::~CAbstractBotSocket()
{
    delete d_ptr;
}

uint32_t CAbstractBotSocket::getLocalIpV4() const
{
    return d_ptr->settings->getLocalIpV4();
}

uint16_t CAbstractBotSocket::getLocalUdpPort() const
{
    return d_ptr->settings->getLocalUdpPort();
}

uint32_t CAbstractBotSocket::getRemoteBotIpV4() const
{
    return d_ptr->settings->getRemoteBotIpV4();
}

uint16_t CAbstractBotSocket::getRemoteBotUdpPort() const
{
    return d_ptr->settings->getRemoteBotUdpPort();
}

void CAbstractBotSocket::setSettings(CAbstractBotSocketSettings &settings)
{
    d_ptr->settings = &settings;
    settingsChanged();
}

void CAbstractBotSocket::setUi(CAbstractUi &gui)
{
    d_ptr->gui = &gui;
    d_ptr->gui->socketStateChanged(state());
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

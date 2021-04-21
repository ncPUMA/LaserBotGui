#include "cbotsocketimitator.h"

#include <QTimer>

static const int DEFAULT_TM_INTERVAL = 20;

class CBotSocketImitatorPrivate
{
    friend class CBotSocketImitator;

    CBotSocketImitatorPrivate() :
        tx(0), ty(0), tz(0),
        rx(0), ry(0), rz(0)
    { }

    QTimer tm;
    double tx, ty, tz;
    double rx, ry, rz;
};



CBotSocketImitator::CBotSocketImitator() :
    CAbstractBotSocket(),
    d_ptr(new CBotSocketImitatorPrivate())
{
    d_ptr->tm.setInterval(DEFAULT_TM_INTERVAL);
    connect(&d_ptr->tm, SIGNAL(timeout()), SLOT(slTmTimeout()));
}

CBotSocketImitator::~CBotSocketImitator()
{
    delete d_ptr;
}

void CBotSocketImitator::setMessageInterval(const int interval)
{
    d_ptr->tm.setInterval(interval);
}

BotSocket::TSocketError CBotSocketImitator::startSocket()
{
    d_ptr->tm.start();
    stateChanged(BotSocket::ENSS_OK);
    return BotSocket::ENSE_NO;
}

void CBotSocketImitator::stopSocket()
{
    d_ptr->tm.stop();
    stateChanged(BotSocket::ENSS_FALL);
}

BotSocket::TSocketState CBotSocketImitator::socketState() const
{
    return d_ptr->tm.isActive() ? BotSocket::ENSS_OK : BotSocket::ENSS_FALL;
}

void CBotSocketImitator::slTmTimeout()
{
    d_ptr->tx += .5;
    if (d_ptr->tx >= 50.)
        d_ptr->tx = 0;
    transformModel(d_ptr->tx, d_ptr->ty, d_ptr->tz,
                   d_ptr->rx, d_ptr->ry, d_ptr->rz);
}

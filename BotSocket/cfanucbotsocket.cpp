#include "cfanucbotsocket.h"

BotSocket::TSocketError CFanucBotSocket::startSocket()
{
    active_ = true;
    // connect signals so translateModel/rotateModel are called
    QObject::connect(&fanuc_socket_, &FanucSocket::position_received, [&](struct FanucSocket::xyzwpr pos){
        transformModel(pos.x, pos.y, pos.z, pos.w, pos.p, pos.r);
    });
    QObject::connect(&fanuc_socket_, &FanucSocket::connection_state_changed, [&](bool ok){
        stateChanged(ok ? BotSocket::ENSS_OK : BotSocket::ENSS_FALL);
    });
    stateChanged(socketState());
    return BotSocket::ENSE_NO;
}

void CFanucBotSocket::stopSocket()
{
    active_ = false;
    // disconnect all signals
    fanuc_socket_.disconnect();
    stateChanged(socketState());
}

BotSocket::TSocketState CFanucBotSocket::socketState() const
{
    return (fanuc_socket_.connected() && active_) ? BotSocket::ENSS_OK : BotSocket::ENSS_FALL;
}

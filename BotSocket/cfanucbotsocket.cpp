#include "cfanucbotsocket.h"

BotSocket::TSocketError CFanucBotSocket::startSocket()
{
    // connect signals so translateModel/rotateModel are called
    QObject::connect(&fanuc_socket_, &FanucSocket::position_received, [&](BotSocket::TDistance x, BotSocket::TDistance y, BotSocket::TDistance z){
        translateModel(x,y,z);
    });
    QObject::connect(&fanuc_socket_, &FanucSocket::angle_received, [&](BotSocket::TAxisType axis_type, BotSocket::TDegree degree){
        rotateModel(axis_type, degree);
    });
    return 0;
}

void CFanucBotSocket::stopSocket()
{
    // disconnect all signals
    fanuc_socket_.disconnect();
}

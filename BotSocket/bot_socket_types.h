#ifndef BOT_SOCKET_TYPES_H
#define BOT_SOCKET_TYPES_H

namespace BotSocket
{

typedef double TDistance;
typedef double TRadianDivisor; //e.g. 1 radian == 1 TRadianDivisor / 2pi;
typedef double TDegree;

enum EN_AxisType
{
    ENAT_X,
    ENAT_Y,
    ENAT_Z
};
typedef int TAxisType;

enum EN_SocketErrors
{
    ENSE_NO
};
typedef int TSocketError;

}

#endif // BOT_SOCKET_TYPES_H

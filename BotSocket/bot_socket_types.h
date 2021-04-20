#ifndef BOT_SOCKET_TYPES_H
#define BOT_SOCKET_TYPES_H

namespace BotSocket
{

typedef double TDistance;
typedef double TDegree;

enum EN_SocketErrors
{
    ENSE_NO = 0
};
typedef int TSocketError;

enum EN_SocketState
{
    ENSS_OK = 0,
    ENSS_FALL
};
typedef int TSocketState;

}

#endif // BOT_SOCKET_TYPES_H

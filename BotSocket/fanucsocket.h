#ifndef FANUCSOCKET_H
#define FANUCSOCKET_H

#include <QObject>
#include <QtNetwork/QTcpSocket>

#include "bot_socket_types.h"

class FanucSocket : public QObject
{
    Q_OBJECT
public:

    explicit FanucSocket(QObject *parent = nullptr);

    bool connected() const;

signals:
    void position_received(BotSocket::TDistance x, BotSocket::TDistance y, BotSocket::TDistance z);
    void angle_received(BotSocket::TAxisType axis_type, BotSocket::TDegree degree);

    void connection_state_changed(bool connected);

private slots:
    void on_readyread();
    void on_connected();
    void on_disconnected();
    void on_error(QAbstractSocket::SocketError error);

private:
    void start_connection();

    QTcpSocket socket_;
};

#endif // FANUCSOCKET_H

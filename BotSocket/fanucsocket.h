#ifndef FANUCSOCKET_H
#define FANUCSOCKET_H

#include <QObject>
#include <QtNetwork/QTcpSocket>

class FanucSocket : public QObject
{
    Q_OBJECT
public:
    struct position{
        double x,y,z; // mm
        double w,p,r; // radians
    };

    explicit FanucSocket(QObject *parent = nullptr);

    bool connected() const;

signals:
    void position_received(struct position pos);
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

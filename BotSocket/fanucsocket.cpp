#include "fanucsocket.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <QTimer>
#include <QSettings>

// fanuc structures
#pragma pack(push, 1)
    struct xyzwpr{
        uint32_t prefix1, prefix2;
        float x, y, z, w, p, r;
        uint32_t config; // T1 | T2 | T3 | flags (need htonl)
    };

    struct xyzwprext{
        uint32_t prefix1, prefix2;
        float x, y, z, w, p, r;
        uint32_t config;
        float ext1, ext2, ext3;
    };

    struct jointpos{
        uint32_t prefix1, prefix2;
        float j[9];
    };

    struct statepos{
        uint32_t time;
        struct xyzwpr w;
        struct xyzwpr u;
        struct jointpos j;
    };
#pragma pack(pop)

FanucSocket::FanucSocket(QObject *parent):
    QObject(parent)
{
    assert(sizeof(struct xyzwpr) == 36);
    assert(sizeof(struct xyzwprext) == 48);

    connect(&socket_, &QAbstractSocket::connected, this, &FanucSocket::on_connected);
    connect(&socket_, &QAbstractSocket::disconnected, this, &FanucSocket::on_disconnected);
    connect(&socket_, &QIODevice::readyRead, this, &FanucSocket::on_readyread);

#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 15
    connect(&socket_, &QAbstractSocket::errorOccurred, this, &FanucSocket::on_error);
#else
    connect(&socket_, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(on_error(QAbstractSocket::SocketError)));
#endif

    start_connection();
}

bool FanucSocket::connected() const
{
    return socket_.state() == QAbstractSocket::ConnectedState;
}

void FanucSocket::on_connected()
{
    qDebug("Connected");
    emit connection_state_changed(true);
}

void FanucSocket::on_disconnected()
{
    qDebug("Disconnected, restarting connection");
    QTimer::singleShot(1000, this, &FanucSocket::start_connection);
    emit connection_state_changed(false);
}

void FanucSocket::on_error(QAbstractSocket::SocketError error)
{
    qDebug("Error %d", error);
    socket_.disconnectFromHost();
    QTimer::singleShot(1000, this, &FanucSocket::start_connection);
    emit connection_state_changed(false);
}

void FanucSocket::on_readyread()
{
    struct statepos s;

    while(socket_.bytesAvailable() >= (qint64)sizeof(s))
    {
        size_t bytes_read = socket_.read(reinterpret_cast<char*>(&s), sizeof(s));
        if(bytes_read != sizeof(s))
        {
            qDebug("Failed to recv");
        }
        assert(bytes_read == sizeof(s));

        for(int i=0; i<9; i++)
            s.j.j[i] = s.j.j[i] * 180/M_PI;
//        qDebug("Pos: t=%f u=(%f, %f, %f, %f, %f, %f, %d) w=(%f, %f, %f, %f, %f, %f, %d) j=(%f, %f, %f, %f, %f, %f, %f, %f, %f)\n",
//                     ((double)s.time)/1e6,
//                     s.u.x, s.u.y, s.u.z, s.u.w, s.u.p, s.u.r, s.u.config,
//                     s.w.x, s.w.y, s.w.z, s.w.w, s.w.p, s.w.r, s.w.config,
//                     s.j.j[0], s.j.j[1], s.j.j[2], s.j.j[3], s.j.j[4], s.j.j[5], s.j.j[6], s.j.j[7], s.j.j[8]);

        FanucSocket::position pos{s.w.x - offset_.x,
                                  s.w.y - offset_.y,
                                  s.w.z - offset_.z,
                                  s.w.w - offset_.w,
                                  s.w.p - offset_.p,
                                  s.w.r - offset_.r};
        qDebug("Pos: t=%f pos=(%f, %f, %f, %f, %f, %f)\n",
               ((double)s.time)/1e6, pos.x, pos.y, pos.z, pos.w, pos.p, pos.r);
        emit position_received(pos);
    }
}

void FanucSocket::start_connection()
{
    if(socket_.state() != QAbstractSocket::ConnectingState &&
       socket_.state() != QAbstractSocket::ConnectedState)
    {
        QSettings settings("fanuc.ini", QSettings::IniFormat);

        offset_.x = settings.value("offset_x",0).toFloat();
        offset_.y = settings.value("offset_y",0).toFloat();
        offset_.z = settings.value("offset_z",0).toFloat();
        offset_.w = settings.value("offset_w",0).toFloat();
        offset_.p = settings.value("offset_p",0).toFloat();
        offset_.r = settings.value("offset_r",0).toFloat();

        QString host = settings.value("server_ip", "127.0.0.1").toString();
        int port = settings.value("server_port", 59002).toInt();
        qDebug() << "Connecting to" << host << ":" << port;

        socket_.connectToHost(host, port);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* This project may also contain libraries licensed under the open source software license GNU GPL v.3.
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef LIFX_H
#define LIFX_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QUdpSocket>

#include "network/networkaccessmanager.h"
#include "devices/device.h"

#include <QColor>

class Lifx : public QObject
{
    Q_OBJECT
public:

#pragma pack(push, 1)
    typedef struct {
        /* frame */
        uint16_t size;
        uint16_t protocol:12;
        uint8_t  addressable:1;
        uint8_t  tagged:1;
        uint8_t  origin:2;
        uint32_t source;
        /* frame address */
        uint8_t  target[8];
        uint8_t  reserved[6];
        uint8_t  res_required:1;
        uint8_t  ack_required:1;
        uint8_t  :6;
        uint8_t  sequence;
        /* protocol header */
        uint64_t :64;
        uint16_t type;
        uint16_t :16;
        /* variable length payload follows */
    } ProtocolHeader_t;
#pragma pack(pop)

    struct Frame {
        //quint16 Size;       //Size of entire message in bytes including this field
        //quint16 Protocol;   //Protocol number: must be 1024 (decimal)
        //bool Addressable;   //Message includes a target address: must be one (1)
        bool Tagged;        //Determines usage of the Frame Address target field
        //quint8 Origin;      //Message origin indicator: must be zero (0)
        quint32 Source;     //Source identifier: unique value set by the client, used by responses
    };

    struct FrameAddress {
        quint64 Target;         //6 byte device address (MAC address) or zero (0) means all devices. The last two bytes should be 0 bytes.
        bool ResponseRequired;  //Response message required
        bool AckRequired;       //Acknowledgement message required
        quint8 Sequence;        //Wrap around message sequence number
    };

    struct ProtocolHeader {
        quint16 Type; //Message type determines the payload being used
    };

    struct Message {
        Frame frame;
        FrameAddress frameAddress;
        ProtocolHeader protocolHeader;
        QByteArray payload;
    };

    enum LightMessages {
        Get         = 101,
        SetColor    = 102,
        SetWaveform = 103,
        SetWaveformOptional = 119,
        State       = 107,
        GetPower    = 116,
        SetPower    = 117,
        StatePower  = 118,
        GetInfrared = 120,
        StateInfrared = 121,
        SetInfrared = 122
    };

    explicit Lifx(QObject *parent = nullptr);
    ~Lifx();
    bool enable();

    void discoverDevices();
    int setColorTemperature(int kelvin, int msFadeTime=500);
    int setColor(QColor color, int msFadeTime = 500);
    int setBrightness(int percentage, int msFadeTime = 500);
    int setPower(bool power, int msFadeTime = 500);
    int flash();
    int flash15s();

private:
    QTimer *m_reconnectTimer = nullptr;
    QUdpSocket *m_socket = nullptr;
    QHostAddress m_host;
    quint16 m_port;
    quint8 m_sequenceNumber = 0;

    void sendMessage(const Message &message);

private slots:
    void onStateChanged(QAbstractSocket::SocketState state);
    void onReadyRead();
    void onReconnectTimer();

signals:
    void connectionChanged(bool connected);
    void devicesDiscovered(QHostAddress address, int port);
    //void requestExecuted(int requestId, bool success);
    //void errorReceived(int code, const QString &message);

    //void powerNotificationReceived(bool status);
    //void brightnessNotificationReceived(int percentage);
    //void colorTemperatureNotificationReceived(int kelvin);
    //void rgbNotificationReceived(QRgb rgbColor);
    //void hueNotificationReceived(int hueColor);
    //void nameNotificationReceived(const QString &name);
    //void saturationNotificationReceived(int percentage);
};
#endif // LIFX_H

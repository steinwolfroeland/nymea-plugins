/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "tcpserver.h"
#include "extern-plugininfo.h"
#include <QNetworkInterface>


TcpServer::TcpServer(const QHostAddress address, const quint16 &port, QObject *parent) :
    QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::newConnection);
    qDebug(dcTCPCommander()) << "TCP Server on Port: " << port << "Address: " << address.toString();
    if (!m_tcpServer->listen(address, port)) {
        qWarning(dcTCPCommander()) << "Unable to start the server: " << m_tcpServer->errorString();
        return;
    }
}

TcpServer::TcpServer(const quint16 &port, QObject *parent) :
    QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::newConnection);
    qDebug(dcTCPCommander()) << "TCP Server on Port: " << port;
    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        qWarning(dcTCPCommander()) << "Unable to start the server: " << m_tcpServer->errorString();
        return;
    }
}

TcpServer::~TcpServer()
{
}

bool TcpServer::isValid()
{
    return m_tcpServer->isListening();
}

QHostAddress TcpServer::serverAddress()
{
    return m_tcpServer->serverAddress();
}

int TcpServer::serverPort()
{
    return m_tcpServer->serverPort();
}

int TcpServer::connectionCount()
{
    return m_connectionCount;
}

void TcpServer::newConnection()
{
    qDebug(dcTCPCommander()) << "TCP Server new Connection request";
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    socket->flush();

    m_connectionCount++;
    emit connectionCountChanged(m_connectionCount);
    connect(socket, &QTcpSocket::disconnected, this, &TcpServer::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpServer::readData);
    // Note: error signal will be interpreted as function, not as signal in C++11
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}

void TcpServer::onDisconnected()
{
    qDebug(dcTCPCommander()) << "TCP Server connection aborted";
    m_connectionCount--;
    if (m_connectionCount < 0)
        m_connectionCount = 0;

    emit connectionCountChanged(m_connectionCount);
}

void TcpServer::readData()
{
    QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
    QByteArray data = socket->readAll();
    qDebug(dcTCPCommander()) << "TCP Server data received: " << data;
    socket->write("OK\n");
    emit commandReceived(data);
}

void TcpServer::onError(QAbstractSocket::SocketError error)
{
    QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
    qWarning(dcTCPCommander()) << "Socket Error" << socket->errorString() << error;
}

/*
    SPDX-FileCopyrightText: 2000 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "smtp.h"
#include "../systeminformation_p.h"

#include <cstdio>

#include <QHostInfo>
#include <QSslSocket>

SMTP::SMTP(char *serverhost, unsigned short int port, int timeout)
    : serverHost(QString::fromUtf8(serverhost))
    , hostPort(port)
    , timeOut(timeout * 1000)
    , connected(false)
    , finished(false)
    , senderAddress(QStringLiteral("user@example.net"))
    , recipientAddress(QStringLiteral("user@example.net"))
    , messageSubject(QStringLiteral("(no subject)"))
    , messageBody(QStringLiteral("empty"))
    , messageHeader(QLatin1String(""))
    , state(Init)
    , serverState(None)
    , domainName(QHostInfo::localDomainName())
    , sock(nullptr)
{
    if (domainName.isEmpty()) {
        domainName = QStringLiteral("somemachine.example.net");
    }

    // qCDebug(DEBUG_KXMLGUI) << "SMTP object created";

    connect(&connectTimer, &QTimer::timeout, this, &SMTP::connectTimerTick);
    connect(&timeOutTimer, &QTimer::timeout, this, &SMTP::connectTimedOut);
    connect(&interactTimer, &QTimer::timeout, this, &SMTP::interactTimedOut);

    // some sendmail will give 'duplicate helo' error, quick fix for now
    connect(this, &SMTP::messageSent, this, &SMTP::closeConnection);
}

SMTP::~SMTP()
{
    delete sock;
    sock = nullptr;
    connectTimer.stop();
    timeOutTimer.stop();
}

void SMTP::setServerHost(const QString &serverhost)
{
    serverHost = serverhost;
}

void SMTP::setPort(unsigned short int port)
{
    hostPort = port;
}

void SMTP::setTimeOut(int timeout)
{
    timeOut = timeout;
}

void SMTP::setSenderAddress(const QString &sender)
{
    senderAddress = sender;
    int index = senderAddress.indexOf(QLatin1Char('<'));
    if (index == -1) {
        return;
    }
    senderAddress.remove(0, index + 1);
    index = senderAddress.indexOf(QLatin1Char('>'));
    if (index != -1) {
        senderAddress.truncate(index);
    }
    senderAddress = senderAddress.simplified();
    while (1) {
        index = senderAddress.indexOf(QLatin1Char(' '));
        if (index != -1) {
            senderAddress.remove(0, index + 1); // take one side
        } else {
            break;
        }
    }
    index = senderAddress.indexOf(QLatin1Char('@'));
    if (index == -1) {
        senderAddress.append(QLatin1String("@localhost")); // won't go through without a local mail system
    }
}

void SMTP::setRecipientAddress(const QString &recipient)
{
    recipientAddress = recipient;
}

void SMTP::setMessageSubject(const QString &subject)
{
    messageSubject = subject;
}

void SMTP::setMessageBody(const QString &message)
{
    messageBody = message;
}

void SMTP::setMessageHeader(const QString &header)
{
    messageHeader = header;
}

void SMTP::openConnection()
{
    // qCDebug(DEBUG_KXMLGUI) << "started connect timer";
    connectTimer.setSingleShot(true);
    connectTimer.start(100);
}

void SMTP::closeConnection()
{
    socketClosed();
}

void SMTP::sendMessage()
{
    if (!connected) {
        connectTimerTick();
    }
    if (state == Finished && connected) {
        // qCDebug(DEBUG_KXMLGUI) << "state was == Finished\n";
        finished = false;
        state = In;
        writeString = QStringLiteral("helo %1\r\n").arg(domainName);
        sock->write(writeString.toLatin1().constData(), writeString.length());
    }
    if (connected) {
        // qCDebug(DEBUG_KXMLGUI) << "enabling read on sock...\n";
        interactTimer.setSingleShot(true);
        interactTimer.start(timeOut);
    }
}

void SMTP::connectTimerTick()
{
    connectTimer.stop();
    //    timeOutTimer.start(timeOut, true);

    // qCDebug(DEBUG_KXMLGUI) << "connectTimerTick called...";

    delete sock;
    sock = nullptr;

    // qCDebug(DEBUG_KXMLGUI) << "connecting to " << serverHost << ":" << hostPort << " ..... ";
    sock = new QSslSocket(this);
    sock->connectToHost(serverHost, hostPort);

    connected = true;
    finished = false;
    state = Init;
    serverState = None;

    connect(sock, &QIODevice::readyRead, this, &SMTP::socketReadyToRead);
    connect(sock, &QAbstractSocket::errorOccurred, this, &SMTP::socketError);
    connect(sock, &QAbstractSocket::disconnected, this, &SMTP::socketClosed);
    timeOutTimer.stop();
    // qCDebug(DEBUG_KXMLGUI) << "connected";
}

void SMTP::connectTimedOut()
{
    timeOutTimer.stop();

    // qCDebug(DEBUG_KXMLGUI) << "socket connection timed out";
    socketClosed();
    Q_EMIT error(ConnectTimeout);
}

void SMTP::interactTimedOut()
{
    interactTimer.stop();

    // qCDebug(DEBUG_KXMLGUI) << "time out waiting for server interaction";
    socketClosed();
    Q_EMIT error(InteractTimeout);
}

void SMTP::socketReadyToRead()
{
    int n;
    int nl;

    // qCDebug(DEBUG_KXMLGUI) << "socketRead() called...";
    interactTimer.stop();

    if (!sock) {
        return;
    }

    n = sock->read(readBuffer, SMTP_READ_BUFFER_SIZE - 1);
    if (n < 0) {
        return;
    }
    readBuffer[n] = 0;
    lineBuffer += QByteArray(readBuffer);
    nl = lineBuffer.indexOf('\n');
    if (nl == -1) {
        return;
    }
    lastLine = lineBuffer.left(nl);
    lineBuffer = lineBuffer.right(lineBuffer.length() - nl - 1);
    processLine(&lastLine);
    if (connected) {
        interactTimer.setSingleShot(true);
        interactTimer.start(timeOut);
    }
}

void SMTP::socketError(QAbstractSocket::SocketError socketError)
{
    // qCDebug(DEBUG_KXMLGUI) << socketError << sock->errorString();
    Q_UNUSED(socketError);
    Q_EMIT error(ConnectError);
    socketClosed();
}

void SMTP::socketClosed()
{
    timeOutTimer.stop();
    // qCDebug(DEBUG_KXMLGUI) << "connection terminated";
    connected = false;
    if (sock) {
        sock->deleteLater();
    }
    sock = nullptr;
    Q_EMIT connectionClosed();
}

void SMTP::processLine(QByteArray *line)
{
    int i;
    int stat;
    QByteArray tmpstr;

    i = line->indexOf(' ');
    tmpstr = line->left(i);
    if (i > 3) {
        // qCDebug(DEBUG_KXMLGUI) << "warning: SMTP status code longer than 3 digits: " << tmpstr;
    }
    stat = tmpstr.toInt();
    serverState = static_cast<SMTPServerStatus>(stat);
    lastState = state;

    // qCDebug(DEBUG_KXMLGUI) << "smtp state: [" << stat << "][" << *line << "]";

    switch (stat) {
    case Greet: // 220
        state = In;
        writeString = QStringLiteral("helo %1\r\n").arg(domainName);
        // qCDebug(DEBUG_KXMLGUI) << "out: " << writeString;
        sock->write(writeString.toLatin1().constData(), writeString.length());
        break;
    case Goodbye: // 221
        state = Quit;
        break;
    case Successful: // 250
        switch (state) {
        case In:
            state = Ready;
            writeString = QStringLiteral("mail from: %1\r\n").arg(senderAddress);
            // qCDebug(DEBUG_KXMLGUI) << "out: " << writeString;
            sock->write(writeString.toLatin1().constData(), writeString.length());
            break;
        case Ready:
            state = SentFrom;
            writeString = QStringLiteral("rcpt to: %1\r\n").arg(recipientAddress);
            // qCDebug(DEBUG_KXMLGUI) << "out: " << writeString;
            sock->write(writeString.toLatin1().constData(), writeString.length());
            break;
        case SentFrom:
            state = SentTo;
            writeString = QStringLiteral("data\r\n");
            // qCDebug(DEBUG_KXMLGUI) << "out: " << writeString;
            sock->write(writeString.toLatin1().constData(), writeString.length());
            break;
        case Data:
            state = Finished;
            finished = true;
            Q_EMIT messageSent();
            break;
        default:
            state = CError;
            // qCDebug(DEBUG_KXMLGUI) << "smtp error (state error): [" << lastState << "]:[" << stat << "][" << *line << "]";
            socketClosed();
            Q_EMIT error(Command);
            break;
        }
        break;
    case ReadyData: // 354
        state = Data;
        writeString = QStringLiteral("Subject: %1\r\n").arg(messageSubject);
        writeString += messageHeader;
        writeString += QLatin1String("\r\n");
        writeString += messageBody;
        writeString += QLatin1String(".\r\n");
        // qCDebug(DEBUG_KXMLGUI) << "out: " << writeString;
        sock->write(writeString.toLatin1().constData(), writeString.length());
        break;
    case Error: // 501
        state = CError;
        // qCDebug(DEBUG_KXMLGUI) << "smtp error (command error): [" << lastState << "]:[" << stat << "][" << *line << "]\n";
        socketClosed();
        Q_EMIT error(Command);
        break;
    case Unknown: // 550
        state = CError;
        // qCDebug(DEBUG_KXMLGUI) << "smtp error (unknown user): [" << lastState << "]:[" << stat << "][" << *line << "]";
        socketClosed();
        Q_EMIT error(UnknownUser);
        break;
    default:
        state = CError;
        // qCDebug(DEBUG_KXMLGUI) << "unknown response: [" << lastState << "]:[" << stat << "][" << *line << "]";
        socketClosed();
        Q_EMIT error(UnknownResponse);
    }
}

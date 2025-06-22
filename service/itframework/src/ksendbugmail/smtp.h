/*
    SPDX-FileCopyrightText: 2000 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

/*int SMTPServerStatus[] = {
    220,  // greeting from server
    221,  // server acknolages goodbye
    250,  // command successful
    354,  // ready to receive data
    501,  // error
    550,  // user unknown
    0     // null
};

int SMTPClientStatus[] = {
    50,   // not logged in yet.
    100,  // logged in, got 220
    150,  // sent helo, got 250
    200,  // sent mail from, got 250
    250,  // sent rctp to, got 250
    300,  // data sent, got 354
    350,  // sent data/., got 250
    400,  // send quit, got 221
    450,  // finished, logged out
    0     // null
};
*/

#define DEFAULT_SMTP_PORT 25
#define DEFAULT_SMTP_SERVER localhost
#define DEFAULT_SMTP_TIMEOUT 60

#define SMTP_READ_BUFFER_SIZE 256

class SMTP : public QObject
{
    Q_OBJECT
public:
    explicit SMTP(char *serverhost = nullptr, unsigned short int port = 0, int timeout = DEFAULT_SMTP_TIMEOUT);
    ~SMTP() override;

    void setServerHost(const QString &serverhost);
    void setPort(unsigned short int port);
    void setTimeOut(int timeout);

    bool isConnected() const
    {
        return connected;
    }
    bool isFinished() const
    {
        return finished;
    }
    QByteArray getLastLine() const
    {
        return lastLine;
    }

    void setSenderAddress(const QString &sender);
    void setRecipientAddress(const QString &recipient);
    void setMessageSubject(const QString &subject);
    void setMessageBody(const QString &message);
    void setMessageHeader(const QString &header);

    typedef enum {
        None = 0, // null
        Greet = 220, // greeting from server
        Goodbye = 221, // server acknolages quit
        Successful = 250, // command successful
        ReadyData = 354, // server ready to receive data
        Error = 501, // error
        Unknown = 550, // user unknown
    } SMTPServerStatus;

    typedef enum {
        Init = 50, // not logged in yet
        In = 100, // logged in, got 220
        Ready = 150, // sent HELO, got 250
        SentFrom = 200, // sent MAIL FROM:, got 250
        SentTo = 250, // sent RCTP TO:, got 250
        Data = 300, // Data sent, got 354
        Finished = 350, // finished sending data, got 250
        Quit = 400, // sent Quit, got 221
        Out = 450, // finished, logged out
        CError = 500, // didn't finish, had error or connection drop
    } SMTPClientStatus;

    typedef enum {
        NoError = 0,
        ConnectError = 10,
        NotConnected = 11,
        ConnectTimeout = 15,
        InteractTimeout = 16,
        UnknownResponse = 20,
        UnknownUser = 30,
        Command = 40,
    } SMTPError;

protected:
    void processLine(QByteArray *line);

public Q_SLOTS:
    void openConnection();
    void sendMessage();
    void closeConnection();

    void connectTimerTick();
    void connectTimedOut();
    void interactTimedOut();

    void socketReadyToRead();
    void socketClosed();
    void socketError(QAbstractSocket::SocketError);

Q_SIGNALS:
    void connectionClosed();
    void messageSent();
    void error(int);

private:
    QString serverHost;
    unsigned short int hostPort;
    int timeOut;

    bool connected;
    bool finished;

    QString senderAddress;
    QString recipientAddress;
    QString messageSubject;
    QString messageBody, messageHeader;

    SMTPClientStatus state;
    SMTPClientStatus lastState;
    SMTPServerStatus serverState;

    QString domainName;

    QTcpSocket *sock;
    QTimer connectTimer;
    QTimer timeOutTimer;
    QTimer interactTimer;

    char readBuffer[SMTP_READ_BUFFER_SIZE];
    QByteArray lineBuffer;
    QByteArray lastLine;
    QString writeString;
};
#endif

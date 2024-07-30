/*
    SPDX-FileCopyrightText: 2007 Jeff Cooper <weirdsox11@gmail.com>
    SPDX-FileCopyrightText: 2007 Thomas Georgiou <TAGeorgiou@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dictengine.h"

#include <chrono>
#include <ranges>

#include <KLocalizedString>
#include <QDebug>
#include <QRegularExpression>
#include <QUrl>

using namespace std::chrono_literals;

DictEngine::DictEngine(QObject *parent)
    : QObject(parent)
    , m_dictNames{QByteArrayLiteral("wn")} // In case we need to switch it later
    , m_serverName(QStringLiteral("dict.org")) // Default, good dictionary
    , m_definitionResponses{
          QByteArrayLiteral("250"), /**< ok (optional timing information here) */
          QByteArrayLiteral("550"), /**< Invalid database */
          QByteArrayLiteral("501"), /**< Syntax error, illegal parameters */
          QByteArrayLiteral("503"), /**< Command parameter not implemented */
      }
{
    m_definitionTimer.setInterval(30s);
    m_definitionTimer.setSingleShot(true);
    connect(&m_definitionTimer, &QTimer::timeout, this, &DictEngine::slotDefinitionReadFinished);
}

DictEngine::~DictEngine()
{
}

void DictEngine::setDict(const QByteArray &dict)
{
    m_dictNames = dict.split(',');
}

void DictEngine::setServer(const QString &server)
{
    m_serverName = server;
}

static QString wnToHtml(const QString &word, QByteArray &text)
{
    QList<QByteArray> splitText = text.split('\n');
    QString def;
    def += QLatin1String("<dl>\n");
    static QRegularExpression linkRx(QStringLiteral("{(.*?)}"));

    bool isFirst = true;
    while (!splitText.empty()) {
        // 150 n definitions retrieved - definitions follow
        // 151 word database name - text follows
        // 250 ok (optional timing information here)
        // 552 No match
        QString currentLine = splitText.takeFirst();
        if (currentLine.startsWith(QLatin1String("151"))) {
            isFirst = true;
            continue;
        }

        if (currentLine.startsWith('.')) {
            def += QLatin1String("</dd>");
            continue;
        }

        // Don't early return if there are multiple dicts
        if (currentLine.startsWith("552") || currentLine.startsWith("501")) {
            def += QStringLiteral("<dt><b>%1</b></dt>\n<dd>%2</dd>").arg(word, i18n("No match found for %1", word));
            continue;
        }

        if (!(currentLine.startsWith(QLatin1String("150")) || currentLine.startsWith(QLatin1String("151")) || currentLine.startsWith(QLatin1String("250")))) {
            // Handle links
            int offset = 0;
            QRegularExpressionMatchIterator it = linkRx.globalMatch(currentLine);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QUrl url;
                url.setScheme("dict");
                url.setPath(match.captured(1));
                const QString linkText = QStringLiteral("<a href=\"%1\">%2</a>").arg(url.toString(), match.captured(1));
                currentLine.replace(match.capturedStart(0) + offset, match.capturedLength(0), linkText);
                offset += linkText.length() - match.capturedLength(0);
            }

            if (isFirst) {
                def += "<dt><b>" + currentLine + "</b></dt>\n<dd>";
                isFirst = false;
                continue;
            } else {
                static QRegularExpression newLineRx(QStringLiteral("([1-9]{1,2}:)"));
                if (currentLine.contains(newLineRx)) {
                    def += QLatin1String("\n<br>\n");
                }
                static QRegularExpression makeMeBoldRx(QStringLiteral("^([\\s\\S]*[1-9]{1,2}:)"));
                currentLine.replace(makeMeBoldRx, QLatin1String("<b>\\1</b>"));
                def += currentLine;
                continue;
            }
        }
    }

    def += QLatin1String("</dl>");
    return def;
}

void DictEngine::getDefinition()
{
    // Clear the old data to prepare for a new lookup
    m_definitionData.clear();

    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &DictEngine::slotDefinitionReadyRead);

    m_tcpSocket->readAll();

    // Command Pipelining: https://datatracker.ietf.org/doc/html/rfc2229#section-4
    QByteArray command;
    for (const QByteArray &dictName : std::as_const(m_dictNames)) {
        command += QByteArrayLiteral("DEFINE ") + dictName + QByteArrayLiteral(" \"") + m_currentWord.toUtf8() + QByteArrayLiteral("\"\n");
    }

    m_tcpSocket->write(command);
    m_tcpSocket->flush();

    m_definitionTimer.start();
}

void DictEngine::getDicts()
{
    m_tcpSocket->readAll();
    QByteArray ret;

    m_tcpSocket->write(QByteArray("SHOW DB\n"));
    m_tcpSocket->flush();

    if (m_tcpSocket->waitForReadyRead()) {
        while (!ret.contains("250") && !ret.contains("420") && !ret.contains("421") && m_tcpSocket->waitForReadyRead()) {
            ret += m_tcpSocket->readAll();
        }
    }

    QMap<QString, QString> availableDicts;
#if (defined(__GNUC__) && __GNUC__ >= 12) || !defined(__GNUC__)
    for (const QByteArrayView curr : QByteArrayView(ret) | std::views::split('\n')) {
#else
    for (const QByteArrayView curr : ret.split('\n')) {
#endif
        if (curr.endsWith("420") || curr.startsWith("421")) {
            // TODO: what happens if the server is down
        }
        if (curr.startsWith("554")) {
            // TODO: What happens if no DB available?
            // TODO: Eventually there will be functionality to change the server...
            break;
        }

        // ignore status code and empty lines
        if (curr.startsWith("250") || curr.startsWith("110") || curr.empty()) {
            continue;
        }

        if (!curr.startsWith('-') && !curr.startsWith('.')) {
            const QString line = QString::fromUtf8(curr).trimmed();
            const QString id = line.section(' ', 0, 0);
            QString description = line.section(' ', 1);
            if (description.startsWith('"') && description.endsWith('"')) {
                description.remove(0, 1);
                description.chop(1);
            }
            availableDicts.insert(id, description);
        }
    }

    m_tcpSocket->disconnectFromHost();
    m_availableDictsCache.insert(m_serverName, availableDicts);
    Q_EMIT dictsRecieved(availableDicts);
    Q_EMIT dictLoadingChanged(false);
}

void DictEngine::requestDicts()
{
    if (m_availableDictsCache.contains(m_serverName)) {
        Q_EMIT dictsRecieved(m_availableDictsCache.value(m_serverName));
        return;
    }
    if (m_tcpSocket) {
        m_tcpSocket->abort(); // stop if lookup is in progress and new query is requested
        m_tcpSocket->deleteLater();
        m_tcpSocket = nullptr;
    }

    Q_EMIT dictLoadingChanged(true);
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &DictEngine::socketClosed);
    connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, [this] {
        Q_EMIT dictErrorOccurred(m_tcpSocket->error(), m_tcpSocket->errorString());
        socketClosed();
    });
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &DictEngine::getDicts);
    m_tcpSocket->connectToHost(m_serverName, 2628);
}

void DictEngine::requestDefinition(const QString &query)
{
    if (m_tcpSocket) {
        m_definitionTimer.stop();
        m_tcpSocket->abort(); // stop if lookup is in progress and new query is requested
        // Delete now to fix "Unexpected null receiver"
        delete m_tcpSocket;
        m_tcpSocket = nullptr;
    }

    m_currentWord = query;

    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &DictEngine::socketClosed);
    connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, [this] {
        Q_EMIT dictErrorOccurred(m_tcpSocket->error(), m_tcpSocket->errorString());
        socketClosed();
    });
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &DictEngine::getDefinition, Qt::SingleShotConnection);
    m_tcpSocket->connectToHost(m_serverName, 2628);
}

void DictEngine::requestDefinition(const QString &query, const QString &server)
{
    setServer(server);
    requestDefinition(query);
}

void DictEngine::requestDefinition(const QString &query, const QByteArray &dictionary)
{
    setDict(dictionary);
    requestDefinition(query);
}

void DictEngine::requestDefinition(const QString &query, const QString &server, const QByteArray &dictionary)
{
    setServer(server);
    setDict(dictionary);
    requestDefinition(query);
}

void DictEngine::slotDefinitionReadyRead()
{
    m_definitionData += m_tcpSocket->readAll();

    const bool finished = std::any_of(m_definitionResponses.cbegin(), m_definitionResponses.cend(), [this](const QByteArray &code) {
        return m_definitionData.contains(code);
    });

    if (finished) {
        slotDefinitionReadFinished();
        return;
    }

    // Close the socket after 30s inactivity
    m_definitionTimer.start();
}

void DictEngine::slotDefinitionReadFinished()
{
    m_definitionTimer.stop();

    const QString html = wnToHtml(m_currentWord, m_definitionData);
    Q_EMIT definitionRecieved(html);

    m_tcpSocket->disconnectFromHost();
    socketClosed();
}

void DictEngine::socketClosed()
{
    Q_EMIT dictLoadingChanged(false);

    if (m_tcpSocket) {
        m_tcpSocket->deleteLater();
    }
    m_tcpSocket = nullptr;
}

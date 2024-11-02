#include "kgeoipinfo.h"
#include <QtXml/QDomDocument>
#include <QDebug>

namespace KServer {

KGeoIPInfo::KGeoIPInfo(QObject *parent) : QObject(parent)
{

}

KGeoIPInfo::~KGeoIPInfo()
{

}

QString KGeoIPInfo::getGeoIpInfo(QString url)
{
    //读取网页信息
    m_manager = new QNetworkAccessManager();
    m_geoIPReadTimer = new QTimer(this);

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(m_geoIPReadTimer, SIGNAL(timeout()), this, SLOT(geoIPReadTimeOut()));

    m_geoIPReadTimer->start(5000);

    m_reply = m_manager->get(QNetworkRequest(QUrl(url)));

    return QString();

}

void KGeoIPInfo::quitEventLoop()
{
    m_geoIPReadTimer->stop();
    m_reply->deleteLater();
    m_manager->deleteLater();
}

void KGeoIPInfo::replyFinished(QNetworkReply *reply)
{
    m_geoIPReadTimer->stop();

    m_responseData = reply->readAll();
    reply->deleteLater();

    emit finished(m_responseData);
}

void KGeoIPInfo::geoIPReadTimeOut()
{
    quitEventLoop();
    emit ReadTimeOut();
}

QStringList KGeoIPInfo::getAttributes(QString attr)
{
    QStringList elements;

    QString domError;
    int errorLine, errorColumn;

    QDomDocument doc;
    if ( doc.setContent( m_responseData, false, &domError, &errorLine, &errorColumn ) )
    {
        const auto tzElements = doc.elementsByTagName( attr );
        qDebug() << "KGeoIPInfo" << tzElements.length() << "elements";
        for ( int it = 0; it < tzElements.length(); ++it )
        {
            auto e = tzElements.at( it ).toElement();
            auto e_text = e.text();
            if ( !e_text.isEmpty() )
            {
                elements.append( e_text );
            }
        }
    }
    else
    {
        qDebug() << "KGeoIPInfo data error:" << domError << "(line" << errorLine << errorColumn << ')';
    }

    if ( elements.count() < 1 )
    {
        qDebug() << "KGeoIPInfo had no non-empty elements" << attr;
    }

    return elements;
}
}

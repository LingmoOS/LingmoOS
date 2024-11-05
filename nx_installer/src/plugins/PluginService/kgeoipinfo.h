#ifndef KGEOIPINFO_H
#define KGEOIPINFO_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace KServer {
const char htmlUrl[] = "http://geoip.ubuntu.com/lookup";

class KGeoIPInfo : public QObject
{
    Q_OBJECT
public:
    explicit KGeoIPInfo(QObject *parent = nullptr);
    ~KGeoIPInfo();
    QString getGeoIpInfo(QString url);      //读取网页信息
    void quitEventLoop();                   //释放对象

    QStringList getAttributes(QString attr);                //根据给定属性获取对应的值

signals:
    void finished(QString s);
    void ReadTimeOut();

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_reply;
    QTimer* m_geoIPReadTimer;
    QString m_responseData;

public slots:
    void replyFinished(QNetworkReply* reply);
    void geoIPReadTimeOut();
};
}
#endif // KGEOIPINFO_H

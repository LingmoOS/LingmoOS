#ifndef TRANSFERUTIL_H
#define TRANSFERUTIL_H

#include <QObject>

class TransferUtil : public QObject
{
    Q_OBJECT

public:
    TransferUtil();
    ~TransferUtil();

    static TransferUtil *instance();

    //linux
    static QString generateRandomNumber();
    static QString tempCacheDir();
    static QString getJsonfile(const QJsonObject &jsonData, const QString &save);
    static int getRemainSize();
    static bool isUnfinishedJob(QString &content, const QString &connectIP);
    static bool checkSize(const QString &filepath);
    static QString DownLoadDir(bool isComplete);

    //win
    static QString getTransferJson(QStringList appList, QStringList fileList, QStringList browserList, QString bookmarksName, QString wallPaperName, QString tempSavePath);

private:
    void initOnlineState();
    bool online = true;
};

#endif

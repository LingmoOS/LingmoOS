#ifndef UNZIPWORKER_H
#define UNZIPWORKER_H

#include <QJsonObject>
#include <QString>
#include <QThread>
#include <QTimer>

class UnzipWorker : public QThread
{
    Q_OBJECT
public:
    UnzipWorker(QString filepath);
    ~UnzipWorker() override;

    void run() override;

    bool extract();
    bool set();

    static int getNumFiles(QString filepath);

    static bool isValid(QString filepath);

private:
    bool setUesrFile(QJsonObject jsonObj);

private:
    QString filepath;
    QString targetDir;
    QTimer *timer = nullptr;

    //Number of decompression completed in the previous second
    int previousTotal = 0;

    // Number of files that have been decompressed right now
    int currentTotal = 0;

    // Number of files decompressed per second
    int speed = -1;

    // Number of zip files
    int count = 0;
};

#endif

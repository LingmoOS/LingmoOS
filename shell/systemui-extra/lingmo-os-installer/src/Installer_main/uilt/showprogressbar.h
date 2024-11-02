#ifndef SHOWPROGRESSBAR_H
#define SHOWPROGRESSBAR_H

#include <QObject>
#include <QTimer>
namespace KInstaller {

class ShowProgressBar : public QObject
{
    Q_OBJECT
public:
    explicit ShowProgressBar(QObject *parent = nullptr);
    void start();

    quint64 getDirSize(const QString filepath);
    bool copyDir(const QString &source, const QString &destination, bool override);
signals:
    void signelProgressValue(int value);
public slots:
    void flushProgressBar();
public:

    QTimer* timer;
    int m_value;
};

}
#endif // SHOWPROGRESSBAR_H

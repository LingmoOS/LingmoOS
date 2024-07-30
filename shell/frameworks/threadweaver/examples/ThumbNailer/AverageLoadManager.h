#ifndef AVERAGELOADMANAGER_H
#define AVERAGELOADMANAGER_H

#include <QObject>

class QTimer;

class AverageLoadManager : public QObject
{
    Q_OBJECT
public:
    explicit AverageLoadManager(QObject *parent = nullptr);

    void activate(bool enabled);
    bool available() const;
    QPair<int, int> workersRange() const;

Q_SIGNALS:
    void recommendedWorkerCount(int);

private Q_SLOTS:
    void update();

private:
    QTimer *m_timer;
    int m_min, m_max;
};

#endif // AVERAGELOADMANAGER_H

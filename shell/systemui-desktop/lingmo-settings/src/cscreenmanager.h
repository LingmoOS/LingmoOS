#ifndef CSCREENMANAGER_H
#define CSCREENMANAGER_H

#include <QObject>
#include <QScreen>
#include <QMap>

#include "cscreenoutput.h"

class CScreenManager : public QObject
{
    Q_OBJECT

public:
    explicit CScreenManager(QObject *parent = nullptr);

    QMap<int, CScreenOutput *> outputMap() const;
    int outputId(const QScreen *qscreen);

private slots:
    void screenAdded(const QScreen *qscreen);
    void screenRemoved(QScreen *qscreen);

private:
    QMap<int, CScreenOutput *> m_outputMap;
    int m_lastOutputId;
};

#endif // CSCREENMANAGER_H

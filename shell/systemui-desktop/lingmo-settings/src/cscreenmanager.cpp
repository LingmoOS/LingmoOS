#include "cscreenmanager.h"

#include <QDebug>
#include <QGuiApplication>

CScreenManager::CScreenManager(QObject *parent)
    : QObject(parent)
    , m_lastOutputId(-1)
{
    for (const QScreen *qscreen : QGuiApplication::screens()) {
        screenAdded(qscreen);
    }

    connect(qApp, &QGuiApplication::screenAdded, this, &CScreenManager::screenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &CScreenManager::screenRemoved);
}

QMap<int, CScreenOutput *> CScreenManager::outputMap() const
{
    return m_outputMap;
}

int CScreenManager::outputId(const QScreen *qscreen)
{
    QList<int> ids;
    for (auto output : qAsConst(m_outputMap)) {
        if (qscreen == output->qscreen()) {
            return output->id();
        }
    }
    m_lastOutputId++;
    return m_lastOutputId;
}

void CScreenManager::screenAdded(const QScreen *qscreen)
{
    CScreenOutput *output = new CScreenOutput;
    output->setId(outputId(qscreen));
    output->setName(qscreen->name());
    m_outputMap.insert(output->id(), output);

//    if (!m_blockSignals) {
//        Q_EMIT configChanged(toKScreenConfig());
//    }
}

void CScreenManager::screenRemoved(QScreen *qscreen)
{
    int removedOutputId = -1;

    for (auto output : m_outputMap) {
        if (output->qscreen() == qscreen) {
            removedOutputId = output->id();
            m_outputMap.remove(removedOutputId);
            delete output;
        }
    }

    // Q_EMIT configChanged(toKScreenConfig());
}

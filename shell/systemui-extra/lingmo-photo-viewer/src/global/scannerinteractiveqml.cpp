#include "scannerinteractiveqml.h"

ScannerInteractiveQml *ScannerInteractiveQml::m_interactionQml(nullptr);
ScannerInteractiveQml::ScannerInteractiveQml(QObject *parent) : QObject(parent) {}
ScannerInteractiveQml *ScannerInteractiveQml::getInstance()
{
    if (m_interactionQml == nullptr) {
        m_interactionQml = new ScannerInteractiveQml;
    }
    return m_interactionQml;
}

int ScannerInteractiveQml::getOperateWay()
{
    return m_operateWay;
}

void ScannerInteractiveQml::setOperateWay(int way)
{
    m_operateWay = way;
    Q_EMIT operateWayChanged(m_operateWay);
}

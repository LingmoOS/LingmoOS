#include "mattinginteractiveqml.h"

MattingInteractiveQml *MattingInteractiveQml::m_mattingInteractionQml(nullptr);
MattingInteractiveQml::MattingInteractiveQml(QObject *parent) : QObject(parent) {}
MattingInteractiveQml *MattingInteractiveQml::getInstance()
{
    if (m_mattingInteractionQml == nullptr) {
        m_mattingInteractionQml = new MattingInteractiveQml;
    }
    return m_mattingInteractionQml;
}

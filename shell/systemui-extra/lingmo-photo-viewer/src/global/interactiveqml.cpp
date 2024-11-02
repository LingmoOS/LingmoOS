#include "interactiveqml.h"
InteractiveQml *InteractiveQml::m_interactionQml(nullptr);
InteractiveQml::InteractiveQml(QObject *parent) : QObject(parent) {}
InteractiveQml *InteractiveQml::getInstance()
{
    if (m_interactionQml == nullptr) {
        m_interactionQml = new InteractiveQml;
    }
    return m_interactionQml;
}

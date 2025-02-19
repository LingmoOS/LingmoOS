#include "translator.h"
#include <QApplication>
#include <QDebug>

QScopedPointer<Translator> Translator::m_instance;

Translator::Translator(QObject *parent) : QObject(parent)
{
}

Translator* Translator::instance()
{
    if (!m_instance) {
        m_instance.reset(new Translator(qApp));
    }
    return m_instance.data();
}

void Translator::switchLanguage(const QString &language)
{
    QString qmFile;
    if (language == "简体中文") {
        qmFile = ":/translations/lingmo-preic_zh_CN.qm";
    } else if (language == "Español") {
        qmFile = ":/translations/lingmo-preic_es.qm";
    }
    
    if (!qmFile.isEmpty()) {
        qApp->removeTranslator(&m_translator);
        if (m_translator.load(qmFile)) {
            qApp->installTranslator(&m_translator);
        }
    } else {
        qApp->removeTranslator(&m_translator);
    }
    
    emit languageChanged();
} 
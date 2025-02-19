#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QTranslator>
#include <QScopedPointer>

class Translator : public QObject
{
    Q_OBJECT
public:
    static Translator* instance();
    void switchLanguage(const QString &language);
    virtual ~Translator() {}

signals:
    void languageChanged();

private:
    explicit Translator(QObject *parent = nullptr);
    static QScopedPointer<Translator> m_instance;
    QTranslator m_translator;
};

#endif // TRANSLATOR_H 
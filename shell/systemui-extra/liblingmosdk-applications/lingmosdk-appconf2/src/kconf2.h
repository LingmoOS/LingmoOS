#ifndef KCONF2_H
#define KCONF2_H

#include <libkysettings.h>
#include <QObject>
#include <QVariant>
namespace kdk {

class KConf2Private;

class KConf2 : public QObject
{
    Q_OBJECT
public:

    KConf2(const QByteArray &schema_id);

    ~KConf2();

    static KConf2 *globalInstance();

    QVariant get(const QString &key) const;

    void set(const QString &key, const QVariant &value);

    bool trySet(const QString &key, const QVariant &value);

    QStringList keys() const;

    void reset(const QString &key);

    static bool isSettingsAvailable(const QString &schema_id);

    QStringList schemas(const QString &app, const QString &version) const;

    QString getDescrition(QString &key);

    QString getSummary(QString &key);

    QString getType(QString &key);

    bool isWritable(QString &key);

    QString getId();

    QString getVersion();

    KSettings *getChild(QString &name);

    QStringList children() const;

Q_SIGNALS:
    void changed(const QString &key);

private:
    Q_DECLARE_PRIVATE(KConf2)
    KConf2Private* const d_ptr;
};

}
#endif // KCONF2_H

#ifndef DESKTOPPROPERTIES_H
#define DESKTOPPROPERTIES_H

#include <QObject>
#include <QVariant>
#include <QMap>

/**
 * @class DesktopProperties
 * @brief Read property files
 * @author Michal Rost
 * @date 26.1.2013
 */

class DesktopProperties
{
public:
    DesktopProperties(const QString &fileName = "", const QString &group = "");
    ~DesktopProperties();

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
    bool load(const QString &fileName, const QString &group = "");
    bool save(const QString &fileName, const QString &group = "");
    void set(const QString &key, const QVariant &value);
    bool contains(const QString &key) const;
    QStringList allKeys() const;

protected:
    QMap<QString, QVariant> data;
};

#endif

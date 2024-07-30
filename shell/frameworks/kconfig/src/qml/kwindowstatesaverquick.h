// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef KWINDOWSTATESAVER_QUICK_H
#define KWINDOWSTATESAVER_QUICK_H

#include <QQmlEngine>

/**
 * @brief Creates a @c KWindowStateSaver in QML, and assigns it to the window it's parented to.
 *
 * Functions exactly as KWindowStateSaver in C++, as it's a small wrapper around it.
 *
 * @code
 * import org.kde.config as KConfig
 *
 * Kirigami.ApplicationWindow {
 *     id: root
 *
 *     title: i18n("My Window")
 *
 *     KConfig.WindowStateSaver {
 *         configGroupName: "Main"
 *     }
 * }
 * @endcode
 * @since 6.5
 *
 * @sa KWindowStateSaver
 */
class KWindowStateSaverQuick : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(WindowStateSaver)
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString configGroupName READ configGroupName WRITE setConfigGroupName NOTIFY configGroupNameChanged REQUIRED)

public:
    void classBegin() override;
    void componentComplete() override;

    void setConfigGroupName(const QString &name);
    QString configGroupName() const;

Q_SIGNALS:
    void configGroupNameChanged();

private:
    QString m_configGroupName;
};

#endif

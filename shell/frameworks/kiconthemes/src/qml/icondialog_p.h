/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ICONDIALOG_H
#define ICONDIALOG_H

#include <QObject>
#include <QString>
#include <qqmlregistration.h>

class KIconDialog;

class IconDialog : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * The name or path of the icon the user has selected
     */
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    /**
     * The desired size of icons
     */
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
    /**
     * The title to use for the dialog
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    /**
     * Begin with the "user icons" instead of "system icons"
     */
    Q_PROPERTY(bool user READ user WRITE setUser NOTIFY userChanged)
    /**
     * Use a custom location, only local directory paths are allowed
     */
    Q_PROPERTY(QString customLocation READ customLocation WRITE setCustomLocation NOTIFY customLocationChanged)
    /**
     * Window modality, default is Qt.NonModal
     */
    Q_PROPERTY(Qt::WindowModality modality READ modality WRITE setModality NOTIFY modalityChanged)
    /**
     * Whether the dialog is currently visible, setting this property to true
     * is the same as calling show()
     */
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

public:
    explicit IconDialog(QObject *parent = nullptr);
    ~IconDialog() override;

    QString iconName() const;

    int iconSize() const;
    void setIconSize(int iconSize);

    QString title() const;
    void setTitle(const QString &title);

    bool user() const;
    void setUser(bool user);

    QString customLocation() const;
    void setCustomLocation(const QString &customLocation);

    Qt::WindowModality modality() const;
    void setModality(Qt::WindowModality modality);

    bool visible() const;
    void setVisible(bool visible);

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

Q_SIGNALS:
    void iconNameChanged(const QString &iconName);
    void iconSizeChanged(int iconSize);
    void titleChanged(const QString &title);
    void userChanged(bool user);
    void customLocationChanged(const QString &customLocation);
    void modalityChanged(Qt::WindowModality modality);
    void visibleChanged();

private:
    bool eventFilter(QObject *watched, QEvent *event) override;

    QScopedPointer<KIconDialog> m_dialog;

    QString m_iconName;
    int m_iconSize;
    bool m_user;
    QString m_customLocation;
    Qt::WindowModality m_modality;
};

#endif // ICONDIALOG_H

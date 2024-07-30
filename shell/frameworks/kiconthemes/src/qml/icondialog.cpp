/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "icondialog_p.h"

#include <QQuickItem>
#include <QQuickWindow>

#include <KIconDialog>
#include <KIconLoader>
#include <QApplication>

IconDialog::IconDialog(QObject *parent)
    : QObject(parent)
    , m_dialog(nullptr)
    , m_iconSize(0)
    , m_user(false)
    , m_modality(Qt::WindowModal)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        m_dialog.reset(new KIconDialog());
        connect(m_dialog.data(), &KIconDialog::newIconName, this, [this](const QString &newIconName) {
            if (m_iconName != newIconName) {
                m_iconName = newIconName;
                Q_EMIT iconNameChanged(newIconName);
            }
        });

        m_dialog->installEventFilter(this);
    }
}

IconDialog::~IconDialog()
{
    if (m_dialog) {
        m_dialog->close();
    }
}

QString IconDialog::iconName() const
{
    return m_iconName;
}

int IconDialog::iconSize() const
{
    return m_iconSize;
}

void IconDialog::setIconSize(int iconSize)
{
    if (m_dialog->iconSize() != iconSize) {
        m_iconSize = iconSize;
        Q_EMIT iconSizeChanged(iconSize);
    }
}

QString IconDialog::title() const
{
    return m_dialog->windowTitle();
}

void IconDialog::setTitle(const QString &title)
{
    if (m_dialog->windowTitle() != title) {
        m_dialog->setWindowTitle(title);
        Q_EMIT titleChanged(title);
    }
}

bool IconDialog::user() const
{
    return m_user;
}

void IconDialog::setUser(bool user)
{
    if (m_user != user) {
        m_user = user;
        Q_EMIT userChanged(user);
    }
}

QString IconDialog::customLocation() const
{
    return m_customLocation;
}

void IconDialog::setCustomLocation(const QString &customLocation)
{
    if (m_customLocation != customLocation) {
        m_dialog->setCustomLocation(customLocation);

        m_customLocation = customLocation;
        Q_EMIT customLocationChanged(customLocation);
    }
}

Qt::WindowModality IconDialog::modality() const
{
    return m_modality;
}

void IconDialog::setModality(Qt::WindowModality modality)
{
    if (m_modality != modality) {
        m_modality = modality;
        Q_EMIT modalityChanged(modality);
    }
}

bool IconDialog::visible() const
{
    return m_dialog->isVisible();
}

void IconDialog::setVisible(bool visible)
{
    if (visible) {
        open();
    } else {
        close();
    }
}

void IconDialog::open()
{
    if (m_dialog->isVisible()) {
        return;
    }

    QQuickItem *parentItem = qobject_cast<QQuickItem *>(parent());
    QQuickWindow *parentWindow = (parentItem ? parentItem->window() : qobject_cast<QQuickWindow *>(parent()));

    if (m_modality == Qt::NonModal) {
        m_dialog->setModal(false);
    } else if (m_modality == Qt::WindowModal) {
        m_dialog->winId(); // needed to get the windowHandle prior to showing
        m_dialog->windowHandle()->setTransientParent(parentWindow);
        m_dialog->setModal(false); // WindowModal does not unset the overall modality
    } else if (m_modality == Qt::ApplicationModal) {
        m_dialog->setModal(true);
    }

    m_dialog->setWindowModality(m_modality);

    m_dialog->setup(KIconLoader::Desktop, KIconLoader::Application, false, m_iconSize, m_user);

    m_dialog->show();
}

void IconDialog::close()
{
    if (!m_dialog->isVisible()) {
        return;
    }

    m_dialog->hide();
}

bool IconDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_dialog.data() && (event->type() == QEvent::Show || event->type() == QEvent::Hide)) {
        Q_EMIT visibleChanged();
    }

    return false;
}

#include "moc_icondialog_p.cpp"

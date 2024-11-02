/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kdialog.h"
#include <QBoxLayout>
#include <QApplication>
#include "xatom-helper.h"
#include "parmscontroller.h"

#include "lingmostylehelper/lingmo-decoration-manager.h"
#include "lingmostylehelper/lingmo-shell-manager.h"
namespace kdk
{
class KDialogPrivate:public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KDialog)
public:
    KDialogPrivate(KDialog*parent);
private:
    KDialog *q_ptr;
    KIconBar *m_pIconBar;
    KWindowButtonBar* m_pWindowButtonBar;
    QHBoxLayout* m_pTitleLayout;
    QVBoxLayout* m_pMainLayout;
    QWidget* m_pMainWidget;
    QString m_IconName;
};

KDialog::KDialog(QWidget *parent)
    :QDialog(parent),
     d_ptr(new KDialogPrivate(this))
{
    Q_D(KDialog);
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("Kdialog");

    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("xcb"),Qt::CaseInsensitive))
    {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }

    connect(d->m_pWindowButtonBar->minimumButton(),&QPushButton::clicked,this,&KDialog::showMinimized);
    connect(d->m_pWindowButtonBar->maximumButton(),&QPushButton::clicked,this,[=]()
        {
            if(this->isMaximized())
                showNormal();
            else
                showMaximized();
        });
    connect(d->m_pWindowButtonBar->closeButton(),&QPushButton::clicked,this,&KDialog::close);
    connect(d->m_pWindowButtonBar,&KWindowButtonBar::doubleClick,this,[=]()
        {
            if(this->isMaximized())
                showNormal();
            else
                showMaximized();
        });
    connect(d->m_pIconBar,&KIconBar::doubleClick,this,[=]()
        {
            if(this->isMaximized())
                showNormal();
            else
                showMaximized();
        });

    changeIconStyle();
    connect(m_gsetting,&QGSettings::changed,this,[=](){changeIconStyle();});
    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,&KDialog::changeTheme);
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](){
        updateGeometry();
    });
    installEventFilter(this);
    resize(600,480);
}

KDialog::~KDialog()
{

}

void KDialog::setWindowIcon(const QIcon &icon)
{
    Q_D(KDialog);
    d->m_pIconBar->setIcon(icon);
    d->m_IconName = icon.name();
    QWidget::setWindowIcon(icon.pixmap(QSize(36,36)));
}

void KDialog::setWindowIcon(const QString &iconName)
{
    Q_D(KDialog);
    d->m_IconName = iconName;
    d->m_pIconBar->setIcon(iconName);
    QWidget::setWindowIcon(QIcon::fromTheme(iconName).pixmap(QSize(36,36)));
}

void KDialog::setWindowTitle(const QString &title)
{
    Q_D(KDialog);
    QVariant value = this->property("isAboutDialog");
    if(value.isNull() || !value.toBool())
    {
        d->m_pIconBar->setWidgetName(title);
    }
    QDialog::setWindowTitle(title);
}

QPushButton *KDialog::maximumButton()
{
    Q_D(KDialog);
    return d->m_pWindowButtonBar->maximumButton();
}

QPushButton *KDialog::minimumButton()
{
    Q_D(KDialog);
    return d->m_pWindowButtonBar->minimumButton();
}

QPushButton *KDialog::closeButton()
{
    Q_D(KDialog);
    return d->m_pWindowButtonBar->closeButton();
}

KMenuButton *KDialog::menuButton()
{
    Q_D(KDialog);
    return d->m_pWindowButtonBar->menuButton();
}


QWidget *KDialog::mainWidget()
{
    Q_D(KDialog);
    if(d->m_pMainWidget)
        return d->m_pMainWidget;
}

bool KDialog::eventFilter(QObject *target, QEvent *event)
{
    Q_D(KDialog);
    if(target == this && (event->type() == QEvent::WindowStateChange || event->type() == QEvent::Show))
    {
        if(this->isMaximized())
            d->m_pWindowButtonBar->setMaximumButtonState(MaximumButtonState::Restore);
        else
            d->m_pWindowButtonBar->setMaximumButtonState(MaximumButtonState::Maximum);
    }
    if(target == this && (event->type() == QEvent::WindowActivate|| event->type() == QEvent::WindowDeactivate))
    {
        changeTheme();
    }
    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        if((event->type() == QEvent::PlatformSurface || event->type() == QEvent::Show || event->type() == QEvent::UpdateRequest || event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildPolished))
        {
            #ifdef USE_LINGMO_PROTOCOL
            if(LingmoUIShellManager::getInstance()->lingmoshellReady())
                LingmoUIShellManager::getInstance()->removeHeaderBar(this->windowHandle());
            else
                LINGMODecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
            #else
                LINGMODecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
            #endif
        }
    }
    if(target == this && (event->type() == QEvent::Close || event->type() == QEvent::Hide))
    {
        d->m_pWindowButtonBar->closeButton()->setAttribute(Qt::WA_UnderMouse, false);
        QHoverEvent hoverEvent(QEvent::HoverLeave, QPoint(10, 10), QPoint(0, 0));
        QCoreApplication::sendEvent(d->m_pWindowButtonBar->closeButton(), &hoverEvent);
    }
    return QDialog::eventFilter(target, event);
}

void KDialog::changeTheme()
{
    Q_D(KDialog);
    initThemeStyle();
    QPalette p = this->palette();
    QColor color = p.color(QPalette::Base);
    p.setColor(QPalette::Window,color);
    this->setPalette(p);
    if(!d->m_IconName.isEmpty())
        this->setWindowIcon(d->m_IconName);
    QPalette palette=this->palette();
    d->m_pWindowButtonBar->setAutoFillBackground(true);
    d->m_pWindowButtonBar->setPalette(palette);
    d->m_pIconBar->setAutoFillBackground(true);
    d->m_pIconBar->setPalette(palette);
}

void KDialog::changeIconStyle()
{
    Q_D(KDialog);
    initThemeStyle();
    setWindowIcon(d->m_IconName);
}

QBoxLayout *KDialog::mainLayout()
{
    Q_D(KDialog);
    return d->m_pMainLayout;
}

KDialogPrivate::KDialogPrivate(KDialog *parent)
    :q_ptr(parent)
{
    Q_Q(KDialog);
    m_pMainLayout = new QVBoxLayout(q);
    m_pTitleLayout = new QHBoxLayout();
    m_pTitleLayout->setContentsMargins(0,0,0,0);
    m_pTitleLayout->setSpacing(0);
    m_pTitleLayout = new QHBoxLayout;
    m_pIconBar = new KIconBar(q);
    m_pWindowButtonBar = new KWindowButtonBar(q);
    m_pTitleLayout->addWidget(m_pIconBar);
    m_pTitleLayout->addWidget(m_pWindowButtonBar);

    m_pMainWidget = new QWidget(q);
    m_pMainLayout->setSpacing(0);
    m_pMainLayout->setContentsMargins(0,0,0,0);
    m_pMainLayout->addLayout(m_pTitleLayout,Qt::AlignTop);
    m_pMainLayout->addWidget(m_pMainWidget);

    //暂时先默认只显示关闭按钮
    m_pWindowButtonBar->maximumButton()->hide();
    m_pWindowButtonBar->menuButton()->hide();
    m_pWindowButtonBar->minimumButton()->hide();

    connect(m_pWindowButtonBar->minimumButton(),&QPushButton::clicked,q,&KDialog::showMinimized);
    connect(m_pWindowButtonBar->closeButton(),&QPushButton::clicked,q,&KDialog::close);
    setParent(parent);
}
}

#include "kdialog.moc"
#include "moc_kdialog.cpp"

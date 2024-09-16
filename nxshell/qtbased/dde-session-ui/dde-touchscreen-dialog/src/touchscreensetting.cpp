// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "touchscreensetting.h"
#include "recognizedialog.h"
#include "monitorindicator.h"

#include <QGuiApplication>
#include <QScreen>
#include <QCursor>
#include <QTimer>

DCORE_USE_NAMESPACE

const QString DisplayServer = "org.deepin.dde.Display1";
const QString DisplayServerPath = "/org/deepin/dde/Display1";

TouchscreenSetting::TouchscreenSetting(const QString &touchscreen, QWidget *parent)
    : DDialog(parent)
    , m_displayInter(new Display1(DisplayServer, DisplayServerPath, QDBusConnection::sessionBus()))
    , m_touchscreenUUID(touchscreen)
    , m_listCombo(new DComboBox)
    , m_monitorIndicator(new MonitorIndicator)
{
    setAccessibleName("TouchscreenSetting");
    m_listCombo->setAccessibleName("ListCombo");
    setTitle(tr("Select your touch screen"));
    setIcon(QIcon::fromTheme("dcc_touchscreen"));

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Tool);

    for (auto monitor : m_displayInter->monitors()) {
        m_monitors << new Monitor(DisplayServer, monitor.path(), QDBusConnection::sessionBus());
    }
    addSpacing(10);
    addContent(m_listCombo);
    addButton(tr("Cancel"));
    addButton(tr("Confirm"), false, ButtonRecommend);

    connect(m_displayInter, &Display1::MonitorsChanged, this, &TouchscreenSetting::onMonitorChanged);
    connect(this, &DDialog::buttonClicked, this, &TouchscreenSetting::onButtonClicked);
    connect(this, &TouchscreenSetting::requestAssociateTouch, m_displayInter, &Display1::AssociateTouchByUUID);
    connect(m_listCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(markDisplay(int)));
    onMonitorChanged();

    showRecognizeDialog();
}

void TouchscreenSetting::onMonitorChanged()
{
    m_listCombo->clear();

    auto monitorPathList = m_displayInter->monitors();
    for (const QDBusObjectPath &m : monitorPathList) {
        display1::Monitor monitor(DisplayServer, m.path(), QDBusConnection::sessionBus());
        m_listCombo->addItem(monitor.name());
    }
}

void TouchscreenSetting::onButtonClicked(int index, const QString &)
{
    if (index != 1) {
        close();
        return;
    }

    qDebug() << "confirm clicked";

    QString monitor = m_listCombo->currentText();
    Q_EMIT requestAssociateTouch(monitor, m_touchscreenUUID);

    close();
}

void TouchscreenSetting::showRecognizeDialog()
{
    QString text = m_monitors[0]->name();
    if (monitorsIsIntersect()) { // 复制模式
        for (int i = 1; i < m_monitors.size(); i++) {
            text += QString(" = %1").arg(m_monitors[i]->name());
        }
        RecognizeDialog *dialog = new RecognizeDialog(m_monitors[0], this);
        dialog->setAccessibleName("RecognizeDialog");
        connect(m_displayInter, &Display1::ScreenHeightChanged, dialog, &RecognizeDialog::onScreenRectChanged);
        dialog->setText(text);
        dialog->show();
    } else { // 拓展模式
        for (auto monitor : m_monitors) {
            RecognizeDialog *dialog = new RecognizeDialog(monitor, this);
            dialog->setAccessibleName("RecognizeDialog");
            connect(m_displayInter, &Display1::ScreenHeightChanged, dialog, &RecognizeDialog::onScreenRectChanged);
            dialog->setText(monitor->name());
            dialog->show();
        }
    }
}

bool TouchscreenSetting::monitorsIsIntersect() const
{
    if (m_monitors.size() < 2)
        return false;

    // only support 2 screens
    Q_ASSERT(m_monitors.size() == 2);

    auto first = m_monitors.first();
    QRect firstRect(first->x(), first->y(), first->width(), first->height());

    auto last = m_monitors.last();
    QRect lastRect(last->x(), last->y(), last->width(), last->height());

    return firstRect.intersects(lastRect);
}

void TouchscreenSetting::markDisplay(int index)
{
    m_monitorIndicator->setGeometry(QRect(m_monitors[index]->x(), m_monitors[index]->y(), m_monitors[index]->width(), m_monitors[0]->height()));
    m_monitorIndicator->show();
    QTimer::singleShot(300, this, [ = ] {
        m_monitorIndicator->hide();
    });
}

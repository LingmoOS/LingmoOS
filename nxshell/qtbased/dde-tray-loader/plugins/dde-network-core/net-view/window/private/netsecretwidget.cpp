// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "netsecretwidget.h"

#include <DPasswordEdit>
#include <DPushButton>
#include <DSuggestButton>

#include <QLayout>
#include <QApplication>

DWIDGET_USE_NAMESPACE

namespace dde {
namespace network {

NetSecretWidget::NetSecretWidget(QWidget *parent)
    : QWidget(parent)
    , m_connectButton(nullptr)
    , m_secretKeyStrMap({
              { "psk", tr("Password") },
              { "wep-key0", tr("Password") },
              { "wep-key1", tr("Password") },
              { "wep-key2", tr("Password") },
              { "wep-key3", tr("Password") },
              { "leap-password", "unknown" },
              { "password", tr("Password") },
              { "password-raw", "unknown" },
              { "ca-cert-password", "unknown" },
              { "client-cert-password", "unknown" },
              { "phase2-ca-cert-password", "unknown" },
              { "phase2-client-cert-password", "unknown" },
              { "private-key-password", tr("Private Pwd") },
              { "phase2-private-key-password", tr("key") },
              { "pin", "unknown" },
              { "proxy-password", tr("Proxy Password") },
              { "IPSec secret", tr("Password") },
              { "Xauth password", tr("Group Password") },
      })
    , m_propMap({
              { "identity", tr("Username") },
              { "ssid", tr("Name (SSID)") },
      })
{
}

NetSecretWidget::~NetSecretWidget() = default;

void NetSecretWidget::initUI(const QVariantMap &param)
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout *>(layout());
    if (mainLayout) {
        while (!mainLayout->isEmpty()) {
            QLayoutItem *item = mainLayout->takeAt(0);
            if (item->widget())
                delete item->widget();
            delete item;
        }
    } else {
        mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 10);
    }
    QMap<QString, QString> prop = param.value("prop").value<QMap<QString, QString>>();
    QWidget *focusWidget = nullptr; // 焦点窗口为第一个未填充的窗口
    for (auto key : param.value("secrets").toStringList()) {
        DLineEdit *lineEdit = nullptr;
        if (m_propMap.contains(key)) {
            lineEdit = new DLineEdit(this);
            lineEdit->setPlaceholderText(m_propMap.value(key));
            if (prop.contains(key)) {
                lineEdit->setText(prop.value(key));
            }
        } else {
            lineEdit = new DPasswordEdit(this);
            lineEdit->setCopyEnabled(false);
            lineEdit->setCutEnabled(false);
            lineEdit->setPlaceholderText(m_secretKeyStrMap.value(key));
        }
        lineEdit->lineEdit()->setMaxLength(256);
        lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
        if (!focusWidget && lineEdit->text().isEmpty())
            focusWidget = lineEdit;
        lineEdit->setProperty("key", key);
        mainLayout->addWidget(lineEdit);

        connect(lineEdit, &DLineEdit::textChanged, this, &NetSecretWidget::onTextChanged);
        connect(lineEdit, &DLineEdit::returnPressed, this, &NetSecretWidget::onReturnPressed);
    }
    DPushButton *cancelButtion = new DPushButton(tr("Cancel", "button"), this);     // 取消
    DPushButton *connectButton = new DSuggestButton(tr("Connect", "button"), this); // 连接
    QHBoxLayout *butLayout = new QHBoxLayout;
    cancelButtion->setAutoDefault(true);
    connectButton->setAutoDefault(true);
    connectButton->setEnabled(false);
    butLayout->addWidget(cancelButtion);
    butLayout->addWidget(connectButton);
    mainLayout->addLayout(butLayout);

    connect(cancelButtion, &DPushButton::clicked, this, &NetSecretWidget::onCancel);
    connect(connectButton, &DPushButton::clicked, this, &NetSecretWidget::onConnect);
    m_connectButton = connectButton;
    onTextChanged();
    m_passwordWidget = focusWidget;
    if (focusWidget)
        QMetaObject::invokeMethod(this, "setFocusToEdit", Qt::QueuedConnection);
}

void NetSecretWidget::setFocusToEdit()
{
    if (!m_passwordWidget) {
        return;
    }

    QWidget *tlw = m_passwordWidget->window();
    const QWindow *w = tlw ? tlw->windowHandle() : 0;
    if (qApp->focusWindow() == w && qApp->activeWindow() != m_passwordWidget->window()) {
        qApp->setActiveWindow(m_passwordWidget);
        m_passwordWidget->activateWindow();
    }

    m_passwordWidget->setFocus();
}

void NetSecretWidget::showError(const QVariantMap &param)
{
    QLayout *lyt = layout();
    QWidget *focusWidget = nullptr;
    for (int i = 0; i < lyt->count(); i++) {
        QLayoutItem *item = lyt->itemAt(i);
        if (item && item->widget()) {
            DLineEdit *lineEdit = qobject_cast<DLineEdit *>(item->widget());
            if (lineEdit) {
                QString key = lineEdit->property("key").toString();
                if (param.contains(key)) {
                    if (!param.value(key).toString().isEmpty())
                        lineEdit->showAlertMessage(param.value(key).toString(), lineEdit);
                    lineEdit->setAlert(true);
                    if (!focusWidget)
                        focusWidget = lineEdit;
                }
            }
        }
    }
    if (focusWidget)
        focusWidget->setFocus();
}

void NetSecretWidget::updateInputValid(const QVariantMap &param)
{
    m_connectButton->setEnabled(param.isEmpty());
}

void NetSecretWidget::onCancel()
{
    submit({ { "input", false } });
}

void NetSecretWidget::onConnect()
{
    QVariantMap param;
    QLayout *lyt = layout();
    for (int i = 0; i < lyt->count(); i++) {
        QLayoutItem *item = lyt->itemAt(i);
        if (item && item->widget()) {
            DLineEdit *lineEdit = qobject_cast<DLineEdit *>(item->widget());
            if (lineEdit) {
                param.insert(lineEdit->property("key").toString(), lineEdit->text());
            }
        }
    }
    Q_EMIT submit(param);
}

void NetSecretWidget::onTextChanged()
{
    QVariantMap param;
    QLayout *lyt = layout();
    for (int i = 0; i < lyt->count(); i++) {
        QLayoutItem *item = lyt->itemAt(i);
        if (item && item->widget()) {
            DLineEdit *lineEdit = qobject_cast<DLineEdit *>(item->widget());
            if (lineEdit) {
                param.insert(lineEdit->property("key").toString(), lineEdit->text());
                lineEdit->setAlert(false);
            }
        }
    }
    Q_EMIT requestCheckInput(param);
}

void NetSecretWidget::onReturnPressed()
{
    DLineEdit *currentEdit = qobject_cast<DLineEdit *>(sender());
    if (!currentEdit) {
        return;
    }
    bool findCurrent = false;
    QLayout *lyt = layout();
    for (int i = 0; i < lyt->count(); i++) {
        QLayoutItem *item = lyt->itemAt(i);
        if (item && item->widget()) {
            DLineEdit *lineEdit = qobject_cast<DLineEdit *>(item->widget());
            if (lineEdit) {
                if (findCurrent && lineEdit->text().isEmpty()) {
                    lineEdit->setFocus();
                    return;
                }
                findCurrent = currentEdit == lineEdit;
            }
        }
    }
    onConnect();
}

} // namespace network
} // namespace dde

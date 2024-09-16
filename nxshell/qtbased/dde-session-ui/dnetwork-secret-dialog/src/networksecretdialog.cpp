// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networksecretdialog.h"

#include <QApplication>
#include <QJsonArray>
#include <QGridLayout>
#include <QDebug>
#include <QFile>
#include <QPushButton>
#include <QKeyEvent>
#include <QLabel>

DWIDGET_USE_NAMESPACE

const QMap<QString, QString> ConnTypeIconKeyMap {
    {"802-3-ethernet", "network-wired"},
    {"802-11-wireless", "notification-network-wireless-full"},
    {"vpn", "network-vpn"},
    {"pppoe", "network-wired"},
};

QMap<QString, QString> secretKeyMap()
{
    static const QMap<QString, QString> SECRET_KEY_STR_MAP = {
        {"psk", "Password"},
        {"wep-key0", "Key"},
        {"wep-key1", "Key"},
        {"wep-key2", "Key"},
        {"wep-key3", "key"},
        {"leap-password", "unknown"},
        {"password", QObject::tr("Password")},
        {"password-raw", "unknown"},
        {"ca-cert-password", "unknown"},
        {"client-cert-password", "unknown"},
        {"phase2-ca-cert-password", "unknown"},
        {"phase2-client-cert-password", "unknown"},
        {"private-key-password", QObject::tr("Private Pwd")},
        {"phase2-private-key-password", "unknown"},
        {"pin", "unknown"},
        {"proxy-password", QObject::tr("Proxy Password")},
        {"IPSec secret", QObject::tr("Password")},
        {"Xauth password", QObject::tr("Group Password")},
    };
    return SECRET_KEY_STR_MAP;
}

NetworkSecretDialog::NetworkSecretDialog(QJsonDocument jsonDoc, QWidget *parent)
    : DDialog(parent)
{
    m_secretKeyStrMap = secretKeyMap();
    m_allInputValid = false;
    parseJsonData(jsonDoc);
    initUI();
}

void NetworkSecretDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        // override the return code when ESC key pressed
        qApp->exit(1);
    }

    DDialog::keyPressEvent(event);
}

void NetworkSecretDialog::parseJsonData(const QJsonDocument &jsonDoc)
{
    m_jsonObj = jsonDoc.object();
    m_connName = m_jsonObj.value("connId").toString();
    m_connType = m_jsonObj.value("connType").toString();
    m_connSettingName = m_jsonObj.value("settingName").toString();
    for (auto secret : m_jsonObj.value("secrets").toArray()) {
        m_secretKeyList.append(secret.toString());
    }
}

void NetworkSecretDialog::initUI()
{
    setAccessibleName("NetworkSecretDialog");
    auto envType = qEnvironmentVariable("XDG_SESSION_TYPE");
    bool bWayland = envType.contains("wayland");
    if (bWayland) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Dialog);
    } else {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Dialog);
    }

    setOnButtonClickedClose(false);
    setCloseButtonVisible(false);
    setTitle(tr("Password required to connect <font color=\"%1\">%2</font>").arg(palette().highlight().color().name()).arg(m_connName));
    addSpacing(10);

    const auto ratio = qApp->devicePixelRatio();
    QPixmap iconPix = QIcon::fromTheme(
                ConnTypeIconKeyMap.value(m_connType, "network-wired")).pixmap(QSize(64, 64) / ratio);
    iconPix.setDevicePixelRatio(ratio);
    setIcon(iconPix);

    addButtons(QStringList() << tr("Cancel") << tr("Connect"));
    setDefaultButton(1);
    getButton(1)->setEnabled(false);

    QWidget *widget = new QWidget;
    widget->setAccessibleName("DialogContentWidget");
    QGridLayout *gridLayout = new QGridLayout;
    widget->setLayout(gridLayout);

    addContent(widget);

    for (int row = 0; row < m_secretKeyList.size(); ++row) {
        const QString &secret = m_secretKeyList.at(row);
        DPasswordEdit *lineEdit = new DPasswordEdit();
        lineEdit->setClearButtonEnabled(true);
        lineEdit->setCopyEnabled(false);
        lineEdit->setCutEnabled(false);
        gridLayout->addWidget(new QLabel(m_secretKeyStrMap.value(secret) + ":"), row, 0);
        gridLayout->addWidget(lineEdit, row, 1);
        m_lineEditList.append(lineEdit);

        if (row == 0) {
            lineEdit->lineEdit()->setFocus();
        }

        connect(lineEdit, &DLineEdit::textChanged, this, &NetworkSecretDialog::checkInputValid);
    }

    connect(this, &NetworkSecretDialog::buttonClicked, this, &NetworkSecretDialog::onButtonsClicked);
}

void NetworkSecretDialog::onButtonsClicked(int index, const QString &text)
{
    Q_UNUSED(text);

    if (index == 0) {
        qApp->exit(1);
    } else {
        submit();
    }
}

void NetworkSecretDialog::submit()
{
    if (m_allInputValid) {
        QJsonObject resultJsonObj;
        QJsonArray secretsJsonArray;
        for (auto lineEdit : m_lineEditList) {
            secretsJsonArray.append(lineEdit->text());
        }
        resultJsonObj.insert("secrets", secretsJsonArray);

        QFile file;
        if (!file.open(stdout, QFile::WriteOnly)) {
            qDebug() << "open STDOUT failed";
            qApp->exit(-4);
        }
        file.write(QJsonDocument(resultJsonObj).toJson());
        file.flush();
        file.close();
        reject();
    } else {
        qDebug() << "some input is invalid!";
        m_lineEditList.at(0)->showAlertMessage(tr("Wrong password, please enter again!"), -1);
        m_lineEditList.at(0)->setAlert(true);
    }
}

void NetworkSecretDialog::checkInputValid()
{
    bool allValid = true;

    m_lineEditList.at(0)->hideAlertMessage();
    for (int i = 0; i < m_secretKeyList.size(); ++i) {
        DPasswordEdit * const lineEdit = m_lineEditList.at(i);
        if (!passwordIsValid(lineEdit->text(), m_secretKeyList.at(i))) {
            allValid = false;
            lineEdit->setAlert(true);
        } else {
            lineEdit->setAlert(false);
        }
    }

    m_allInputValid = allValid;

    getButton(1)->setEnabled(m_allInputValid);
}

bool NetworkSecretDialog::passwordIsValid(const QString &text, const QString &secretKey)
{
    bool valid = true;

    if (secretKey.startsWith("wep")) {
        valid = wepKeyIsValid(text, WepKeyType::Hex);
    } else if (secretKey == "psk") {
        valid = wpaPskIsValid(text);
    } else {
        valid = !text.isEmpty();
    }

    return valid;
}

bool NetworkSecretDialog::wepKeyIsValid(const QString &key, WepKeyType type)
{
    if (key.isEmpty()) {
        return false;
    }

    const int keylen = key.length();

    if (type != WepKeyType::NotSpecified) {
        if (type == WepKeyType::Hex) {
            return true;
        } else if (type == WepKeyType::Passphrase) {
            if (!keylen || keylen > 64) {
                return false;
            }

            return true;
        }
    }

    return false;
}

bool NetworkSecretDialog::wpaPskIsValid(const QString &psk)
{
    if (psk.isEmpty()) {
        return false;
    }

    const int psklen = psk.length();

    if (psklen < 8 || psklen > 64) {
        return false;
    }

    if (psklen == 64) {
        /* Hex PSK */
        for (int i = 0; i < psklen; ++i) {
            if (!psk.at(i).isLetterOrNumber()) {
                return false;
            }
        }
    }

    return true;
}

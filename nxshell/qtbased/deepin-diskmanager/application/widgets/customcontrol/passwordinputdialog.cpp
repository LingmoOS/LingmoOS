// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "passwordinputdialog.h"

#include <DMessageManager>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

PasswordInputDialog::PasswordInputDialog(QWidget *parent)
    : DDBase(parent)
{
    initUi();
    initConnection();
}

void PasswordInputDialog::initUi()
{
    setFixedWidth(406);

    DPalette palette1;
    QColor color1("#000000");
    color1.setAlphaF(0.9);
    palette1.setColor(DPalette::ToolTipText, color1);

    DPalette palette2;
    QColor color2("#000000");
    color2.setAlphaF(0.7);
    palette2.setColor(DPalette::ToolTipText, color2);

    DPalette palette3;
    palette3.setColor(DPalette::TextTitle, QColor("#001A2E"));

    QFont font1;
    font1.setWeight(QFont::Medium);
    font1.setFamily("Source Han Sans");
    font1.setPixelSize(14);

    QFont font2;
    font2.setWeight(QFont::Normal);
    font2.setFamily("Source Han Sans");
    font2.setPixelSize(14);

    QFont font3;
    font3.setWeight(QFont::Medium);
    font3.setFamily("Source Han Sans");
    font3.setPixelSize(13);

    setTitle(tr("Set a password to encrypt %1").arg(""));

    DLabel *tipLabel = new DLabel(tr("The password cannot be reset or retrieved online"), this);
    tipLabel->setFont(font2);
    tipLabel->setPalette(palette2);

    DLabel *inputPasswordLabel = new DLabel(tr("Password"), this);
    inputPasswordLabel->setPalette(palette3);
    inputPasswordLabel->setFont(font3);
    inputPasswordLabel->setFixedHeight(36);

    DLabel *repeatPasswordLabel = new DLabel(tr("Repeat password"), this);
    repeatPasswordLabel->setPalette(palette3);
    repeatPasswordLabel->setFont(font3);
    repeatPasswordLabel->setFixedHeight(36);

    DLabel *hintPasswordLabel = new DLabel(tr("Password hint"), this);
    hintPasswordLabel->setPalette(palette3);
    hintPasswordLabel->setFont(font3);

    DLabel *recommendedLabel = new DLabel(tr("(Recommended)"), this);
    recommendedLabel->setPalette(palette3);
    recommendedLabel->setFont(font3);

    // 限制中文和中文字符
    QRegExp regExp("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\/ ]+$");
    QRegExpValidator *regExpValidator = new QRegExpValidator(regExp, this);

    m_inputPasswordEdit = new DPasswordEdit(this);
    m_inputPasswordEdit->setAccessibleName("Password");
    m_inputPasswordEdit->lineEdit()->setPlaceholderText(tr("Enter a password "));
    m_inputPasswordEdit->setFixedHeight(36);
    m_inputPasswordEdit->lineEdit()->setValidator(regExpValidator);

    m_checkPasswordEdit = new DPasswordEdit(this);
    m_checkPasswordEdit->setAccessibleName("Repeat password");
    m_checkPasswordEdit->lineEdit()->setPlaceholderText(tr("Enter the password again"));
    m_checkPasswordEdit->setFixedHeight(36);
    m_checkPasswordEdit->lineEdit()->setValidator(regExpValidator);

    m_textEdit = new DTextEdit(this);
    m_textEdit->setAccessibleName("Password hint");
    m_textEdit->setPlaceholderText(tr("Enter a password hint") + " (0/50)");
    m_textEdit->setFixedHeight(84);

    QVBoxLayout *hintPasswordLayout = new QVBoxLayout;
    hintPasswordLayout->addWidget(hintPasswordLabel);
    hintPasswordLayout->addWidget(recommendedLabel, 0, Qt::AlignCenter);
    hintPasswordLayout->addStretch();
    hintPasswordLayout->setSpacing(0);
    hintPasswordLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(inputPasswordLabel);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(repeatPasswordLabel);
    leftLayout->addSpacing(10);
    leftLayout->addLayout(hintPasswordLayout);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(m_inputPasswordEdit);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(m_checkPasswordEdit);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(m_textEdit);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *centerLayout = new QHBoxLayout;
    centerLayout->addLayout(leftLayout);
    centerLayout->addSpacing(10);
    centerLayout->addLayout(rightLayout);
    centerLayout->setSpacing(0);
    centerLayout->setContentsMargins(6, 0, 10, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(tipLabel, 0, Qt::AlignCenter);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(centerLayout);
    mainLayout->addSpacing(10);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    int index = addButton(tr("Cancel", "button"), true, ButtonNormal);
    m_okCode = addButton(tr("Confirm", "button"), false, ButtonRecommend);
    setOnButtonClickedClose(false);

    getButton(index)->setAccessibleName("passwordCancel");
    getButton(m_okCode)->setAccessibleName("passwordConfirm");
}

void PasswordInputDialog::initConnection()
{
    connect(m_inputPasswordEdit, &DLineEdit::textChanged, this, &PasswordInputDialog::onInputPasswordEditTextChanged);
    connect(m_checkPasswordEdit, &DLineEdit::textChanged, this, &PasswordInputDialog::onCheckPasswordEditTextChanged);
    connect(m_textEdit, &QTextEdit::textChanged, this, &PasswordInputDialog::passwordHintTextChanged);
    connect(this, &PasswordInputDialog::buttonClicked, this, &PasswordInputDialog::onButtonClicked);
}

void PasswordInputDialog::setDeviceName(const QString &devName)
{
    setTitle(tr("Set a password to encrypt %1").arg(devName));
}

void PasswordInputDialog::setTitleText(const QString &text)
{
}

QString PasswordInputDialog::getPassword()
{
    return m_inputPasswordEdit->text();
}

QString PasswordInputDialog::getPasswordHint()
{
    return m_textEdit->toPlainText();
}

void PasswordInputDialog::onInputPasswordEditTextChanged(const QString &)
{
    if (m_inputPasswordEdit->isAlert() && m_inputPasswordEdit->text().length() <= 256) {
        m_inputPasswordEdit->setAlert(false);
        m_inputPasswordEdit->hideAlertMessage();
    }

    // 密码超出最大长度
    if (m_inputPasswordEdit->text().length() > 256) {
        m_inputPasswordEdit->setAlert(true);
        m_inputPasswordEdit->showAlertMessage(tr("The password exceeds the maximum length"));
    }
}

void PasswordInputDialog::onCheckPasswordEditTextChanged(const QString &)
{
    if (m_checkPasswordEdit->isAlert() && m_checkPasswordEdit->text().length() <= 256) {
        m_checkPasswordEdit->setAlert(false);
        m_checkPasswordEdit->hideAlertMessage();
    }

    // 密码超出最大长度
    if (m_checkPasswordEdit->text().length() > 256) {
        m_checkPasswordEdit->setAlert(true);
        m_checkPasswordEdit->showAlertMessage(tr("The password exceeds the maximum length"));
    }
}

void PasswordInputDialog::passwordHintTextChanged()
{
    // 设置最大可输入字符
    QString textContent = m_textEdit->toPlainText();
    int length = textContent.count();
    int maxLength = 50; // 最大字符数
    if (length > maxLength) {
        int position = m_textEdit->textCursor().position();
        QTextCursor textCursor = m_textEdit->textCursor();
        textContent.remove(position - (length - maxLength), length - maxLength);
        m_textEdit->setText(textContent);
        textCursor.setPosition(position - (length - maxLength));
        m_textEdit->setTextCursor(textCursor);
    }
}

void PasswordInputDialog::onButtonClicked(int index, const QString &text)
{
    Q_UNUSED(text);
    if (index == m_okCode) {
        // 密码不能为空
        if (m_inputPasswordEdit->text().isEmpty()) {
            m_inputPasswordEdit->setAlert(true);
            m_inputPasswordEdit->showAlertMessage(tr("The password cannot be empty"));
            return;
        }

        // 密码超出最大长度
        if (m_inputPasswordEdit->text().length() > 256) {
            m_inputPasswordEdit->setAlert(true);
            m_inputPasswordEdit->showAlertMessage(tr("The password exceeds the maximum length"));
            return;
        }

        // 密码不一致
        if (m_inputPasswordEdit->text() != m_checkPasswordEdit->text()) {
            m_checkPasswordEdit->setAlert(true);
            m_checkPasswordEdit->showAlertMessage(tr("Passwords do not match"));
            return;
        }

        if (m_inputPasswordEdit->text() == m_textEdit->toPlainText()) {
            // 提示内容不得与密码相同
            DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("://icons/deepin/builtin/warning.svg"),
                                                     tr("The password hint should differ from the password"));
            DMessageManager::instance()->setContentMargens(this, QMargins(0, 0, 0, 43));
            return;
        }

        accept();
    } else {
        reject();
    }
}

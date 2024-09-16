// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialog.h"
#include <QKeyEvent>
#include <QApplication>
#include <QLabel>
#include <QIcon>
#ifdef USE_DTK
#define FATHERDIALOG DDialog
#else
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#define FATHERDIALOG QDialog
#endif

MessageDlg::MessageDlg(QWidget *parent) :
#ifdef USE_DTK
    DDialog(parent)
#else
    QDialog(parent)
#endif
{
    setModal(true);
#ifdef USE_DTK
    setWordWrapMessage(true);
#else
    auto lay = new QVBoxLayout(this);
    this->setLayout(lay);

    _messageLabel = new QLabel(this);
    _messageLabel->setWordWrap(true);
    _messageLabel->setAlignment(Qt::AlignCenter);
    _btnsLayout = new QHBoxLayout;

    lay->addWidget(_messageLabel);
    lay->addLayout(_btnsLayout);
#endif
    setMinimumSize(403, 163);
}

MessageDlg::MessageDlg(const SMessage &message, QWidget *parent): MessageDlg(parent)
{
    setMessage(message);
}

QWidget *defaultParentWindow()
{
    if (qApp->activeModalWidget() != nullptr) {
        return qApp->activeModalWidget();
    }
    return qApp->activeWindow();
}

int MessageDlg::execMessage(const SMessage &message, QWidget *parent)
{
    MessageDlg dialog(message, parent == nullptr ? defaultParentWindow() : parent);

    return dialog.exec();
}

int MessageDlg::execMessage(const QString &message, QWidget *parent)
{
    SMessage msg(message);
    return execMessage(msg, parent);
}

int MessageDlg::execMessage(const QString &message, EMessageType msgTp, QWidget *parent)
{
    SMessage msg(message, msgTp);
    return execMessage(msg, parent);
}

int MessageDlg::execMessage(const QString &message, EMessageType msgTp,
                            const QStringList &moreBtns,
                            const QList<EButtonType> &btntps,
                            QWidget *parent)
{
    SMessage msg(message, msgTp, moreBtns, btntps);
    return execMessage(msg, parent);
}


void MessageDlg::setMessage(const SMessage &message)
{
    _message = message;
    updateMessage();
}

SMessage MessageDlg::message() const
{
    return _message;
}

#ifndef USE_DTK
int MessageDlg::exec()
{
    return QDialog::exec();
}
#endif

void MessageDlg::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::Cancel)) {
        close();
        return;
    }
    FATHERDIALOG::keyPressEvent(e);
}

void MessageDlg::showEvent(QShowEvent *event)
{
    QMetaObject::invokeMethod(this, [ = ]() {
        QMetaObject::invokeMethod(this, [ = ]() {

            auto window = this->parentWidget() != nullptr ? this->parentWidget()->window() : defaultParentWindow();
            if (window != nullptr) {
                QPoint centerPos = window->geometry().center() - this->geometry().center();
                QRect parentWindowGem = window->geometry();

                centerPos = parentWindowGem.topLeft() + QPoint((parentWindowGem.width() - this->width()) / 2,
                                                               (parentWindowGem.height() - this->height()) / 2);

                this->move(centerPos);
            }

        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);

    FATHERDIALOG::showEvent(event);
}

void MessageDlg::updateMessage()
{
#ifdef USE_DTK
    clearButtons();
#endif

    auto showText = QFontMetrics(this->font()).elidedText(_message.message, Qt::ElideMiddle, _message.maxMsgPixelLenth);
#ifdef USE_DTK
    this->DDialog::setMessage(showText);
#endif

    QPixmap iconSvg;
    switch (_message.messageType) {
    case ENormalMsg:
        iconSvg = QIcon::fromTheme("deepin-draw").pixmap(QSize());
        break;
    case EWarningMsg:
        iconSvg = QPixmap(":/icons/deepin/builtin/texts/Bullet_window_warning.svg");
        break;
    case EQuestionMsg:
        iconSvg = QPixmap(":/icons/deepin/builtin/texts/Bullet_window_warning.svg");
        break;
    }
#ifdef USE_DTK
    this->setIcon(iconSvg);

    if (_message.btns.size() == _message.btnType.size())
        for (int i = 0; i < _message.btns.size(); ++i)
            this->insertButton(i, _message.btns.at(i), false, DDialog::ButtonType(_message.btnType.at(i)));
#else
    this->setWindowIcon(QIcon(iconSvg));
    _messageLabel->setText(showText);
    while (_btnsLayout->count() > 0) {
        delete _btnsLayout->takeAt(0);
    }

    for (int i = 0; i < _message.btns.count(); ++i) {
        auto btnText = _message.btns.at(i);
        auto btn = new QPushButton(btnText, this);
        connect(btn, &QPushButton::clicked, this, [ = ]() {
            done(i);
        });
        _btnsLayout->addWidget(btn, Qt::AlignCenter);
    }

#endif
}

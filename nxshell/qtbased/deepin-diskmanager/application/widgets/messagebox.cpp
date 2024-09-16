// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only



#include "messagebox.h"

#include <DLabel>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QAbstractButton>
#include <QKeyEvent>

MessageBox::MessageBox(QWidget *parent) : DDialog(parent)
{

}

void MessageBox::setWarings(const QString &title, const QString &warningMsg, const QString &sureBtnText, const QString &sureAccessibleName, const QString &cancalBtnText, const QString &cancelAccessibleName)
{
    setIcon(QIcon::fromTheme("://icons/deepin/builtin/exception-logo.svg"));

    if (!title.isEmpty()) {
        setTitle(title);
        addSpacing(10);
    }

    if (!warningMsg.isEmpty()) {
        addLabel(warningMsg);
        addSpacing(10);
    }

    if (!cancalBtnText.isEmpty()) {
        int index = addButton(cancalBtnText);
        getButton(index)->setAccessibleName(cancelAccessibleName);
    }

    int index = addButton(sureBtnText);
    getButton(index)->setAccessibleName(sureAccessibleName);
    connect(this, &MessageBox::buttonClicked, this, [=]() {
        close();
    });
}

void MessageBox::setWarings(const QString &title, const QString &warningMsg, const QString &sureBtnText, ButtonType sureBtnType, const QString &sureAccessibleName, const QString &cancalBtnText, const QString &cancelAccessibleName)
{
    setIcon(QIcon::fromTheme("://icons/deepin/builtin/exception-logo.svg"));

    if (!title.isEmpty()) {
        setTitle(title);
        addSpacing(10);
    }

    if (!warningMsg.isEmpty()) {
        addLabel(warningMsg);
        addSpacing(10);
    }

    if (!cancalBtnText.isEmpty()) {
        int index = addButton(cancalBtnText);
        getButton(index)->setAccessibleName(cancelAccessibleName);
    }

    int index = addButton(sureBtnText, false, sureBtnType);
    getButton(index)->setAccessibleName(sureAccessibleName);
    connect(this, &MessageBox::buttonClicked, this, [=]() {
        close();
    });
}

void MessageBox::setWarings(const QString &title, const QString &sureBtnText, ButtonType sureBtnType, const QString &sureAccessibleName,
                const QString &cancalBtnText, const QString &cancelAccessibleName)
{
    setIcon(QIcon::fromTheme("://icons/deepin/builtin/exception-logo.svg"));

    DLabel *label= new DLabel(this);
    label->setText(title);
    label->setObjectName("TitleLabel");
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T5, QFont::Medium);
    QPalette palette = label->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::BrightText), 0, 0, 0, 0, 0, 0, -10);
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);
    QFontMetrics fmDevpath = label->fontMetrics();
    int fmWidth = fmDevpath.width(label->text());
    int labelWidth = fmWidth / 3 * 2;
    if (labelWidth >= (width() - 20)) {
        labelWidth = width() - 20;
    }
    qDebug() << labelWidth << width();
    label->setFixedWidth(labelWidth);
    label->adjustSize();
    addContent(label, Qt::AlignHCenter);
    addSpacing(10);

    if (!cancalBtnText.isEmpty()) {
        int index = addButton(cancalBtnText);
        getButton(index)->setAccessibleName(cancelAccessibleName);
    }

    int index = addButton(sureBtnText, false, sureBtnType);
    getButton(index)->setAccessibleName(sureAccessibleName);
    connect(this, &MessageBox::buttonClicked, this, [=]() {
        close();
    });
}

void MessageBox::setProgressBar(const QString &title, const QString &cancalBtnText)
{
    setIcon(QIcon::fromTheme("://icons/deepin/builtin/exception-logo.svg"));

    if (!title.isEmpty()) {
        setTitle(title);
        addSpacing(10);
    }

    addProgressBar();
    addSpacing(10);

    addButton(cancalBtnText);
    connect(this, &MessageBox::buttonClicked, this, [=]() {
        close();
    });
}

void MessageBox::addLabel(QString text)
{
    DLabel *label= new DLabel(this);
    label->setText(text);
    label->adjustSize();
    label->setWordWrap(true);
    addContent(label, Qt::AlignHCenter);
}

void MessageBox::addProgressBar()
{
    m_progressBar = new DProgressBar(this);
    m_progressBar->setValue(50);
    m_progressBar->setFixedSize(330, 10);
    addContent(m_progressBar, Qt::AlignHCenter);
}

void MessageBox::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Escape) {
        event->ignore();
    } else {
        DDialog::keyPressEvent(event);
    }
}



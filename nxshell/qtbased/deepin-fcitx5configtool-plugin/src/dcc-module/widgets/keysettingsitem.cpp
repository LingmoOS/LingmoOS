// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keysettingsitem.h"

// #include "publisher/publisherdef.h"

#include <DCommandLinkButton>
#include <DFontSizeManager>

#include <QBrush>
#include <QCheckBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainterPath>
#include <QProcess>

namespace dcc_fcitx_configtool {
namespace widgets {
FcitxKeyLabelWidget::FcitxKeyLabelWidget(fcitx::Key list, QWidget *p)
    : QWidget(p), m_curlist(list) {
    m_eidtFlag = true;
    m_keyEdit = new QLineEdit(this);
    m_keyEdit->installEventFilter(this);
    m_keyEdit->setReadOnly(true);
    m_keyEdit->hide();
    m_keyEdit->setPlaceholderText(tr("Enter a new shortcut"));
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 9, 0, 9);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_keyEdit);
    setLayout(m_mainLayout);
    setList(m_curlist);
    setShortcutShow(true);
}

FcitxKeyLabelWidget::~FcitxKeyLabelWidget() { clearShortcutKey(); }

void FcitxKeyLabelWidget::setList(const fcitx::Key &list) {
    m_curlist = list;
    initLableList(m_curlist);
}

void FcitxKeyLabelWidget::initLableList(const fcitx::Key &list) {
    // qDebug() << "initLableList " << list;
    clearShortcutKey();
    for (const QString &key :
         QString::fromStdString(
             list.toString(fcitx::KeyStringFormat::Localized))
             .split("+")) {
        FcitxKeyLabel *label = new FcitxKeyLabel(key);
        label->setAccessibleName(key);
        label->setBackgroundRole(DPalette::DarkLively);
        m_list << label;
        m_mainLayout->addWidget(label);
    }
    // qDebug() << "m_curlist " << m_curlist;
    // qDebug() << "m_newlist " << m_newlist;
}

QString FcitxKeyLabelWidget::getKeyToStr() {
    return QString::fromStdString(m_curlist.toString());
}

void FcitxKeyLabelWidget::setEnableEdit(bool flag) { m_eidtFlag = flag; }

void FcitxKeyLabelWidget::enableSingleKey() { m_isSingle = true; }

void FcitxKeyLabelWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_eidtFlag)
        return;
    setShortcutShow(!m_keyEdit->isHidden());
    QWidget::mousePressEvent(event);
}

bool FcitxKeyLabelWidget::eventFilter(QObject *watched, QEvent *event) {
    if (m_keyEdit == watched) {
        if (event->type() == QEvent::Hide ||
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::FocusOut) {
            setShortcutShow(true);
            return true;
        }
        if (event->type() == QEvent::Show) {
            setShortcutShow(false);
            return true;
        }
    }
    return false;
}

void FcitxKeyLabelWidget::keyPressEvent(QKeyEvent *event) {
    if (!m_eidtFlag) {
        return;
    }

    bool done = true;
    auto newlist =
        fcitx::Key(static_cast<fcitx::KeySym>(event->nativeVirtualKey()),
                   fcitx::KeyStates(event->nativeModifiers()),
                   event->nativeScanCode())
            .normalize();
    if (newlist.toString() == "") {
        done = false;
    }

    auto modifiers =
        event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier |
                              Qt::AltModifier | Qt::MetaModifier);
    if (modifiers == 0) {
        done = false;
    }

    if (done) {
        m_curlist = newlist;
    }

    initLableList(m_curlist);
    setShortcutShow(true);
    Q_EMIT editedFinish();
    QWidget::keyPressEvent(event);
}

void FcitxKeyLabelWidget::keyReleaseEvent(QKeyEvent *event) {
    // qDebug() << "keyReleaseEvent";
    // if (!m_eidtFlag)
    //     return;
    // if ((m_newlist.count() < 2 && !m_isSingle) || !checkNewKey(true)) {
    //     m_curlist.removeDuplicates();
    //     initLableList(m_curlist);
    // }
    // setShortcutShow(true);
    //    QWidget::keyReleaseEvent(event);
}

void FcitxKeyLabelWidget::clearShortcutKey() {
    for (FcitxKeyLabel *label : m_list) {
        m_mainLayout->removeWidget(label);
        label->deleteLater();
    }
    m_list.clear();
}

void FcitxKeyLabelWidget::setShortcutShow(bool flag) {
    if (flag) {
        m_mainLayout->setContentsMargins(0, 9, 0, 9);
        m_keyEdit->hide();
        int w = 0;
        for (FcitxKeyLabel *label : m_list) {
            w += label->width() + 9;
            label->show();
        }
        setMaximumWidth(w);
        setFocus();
    } else {
        for (FcitxKeyLabel *label : m_list) {
            label->hide();
        }
        m_mainLayout->setContentsMargins(0, 0, 0, 0);
        m_keyEdit->show();
        m_keyEdit->setFocus();
        m_keyEdit->clear();
        setMaximumWidth(9999);
    }
    update();
}

FcitxKeySettingsItem::FcitxKeySettingsItem(const QString &text,
                                           const fcitx::Key &list,
                                           QFrame *parent)
    : SettingsItem(parent) {
    m_label = new FcitxShortenLabel(text, this);
    m_keyWidget = new FcitxKeyLabelWidget(list, parent);
    m_hLayout = new QHBoxLayout(this);
    m_hLayout->setContentsMargins(10, 0, 10, 0);
    m_hLayout->addWidget(m_label);
    m_hLayout->addWidget(m_keyWidget);
    m_hLayout->setAlignment(m_label, Qt::AlignLeft);
    m_hLayout->addWidget(m_keyWidget, 0, Qt::AlignVCenter | Qt::AlignRight);
    setFixedHeight(48);
    setAccessibleName(text);
    setLayout(m_hLayout);
    connect(m_keyWidget, &FcitxKeyLabelWidget::editedFinish, this,
            &FcitxKeySettingsItem::editedFinish);
    connect(m_keyWidget, &FcitxKeyLabelWidget::shortCutError, this,
            &FcitxKeySettingsItem::doShortCutError);
}

void FcitxKeySettingsItem::setText(const QString &text) {
    m_label->setShortenText(text);
}

void FcitxKeySettingsItem::enableSingleKey() { m_keyWidget->enableSingleKey(); }

QString FcitxKeySettingsItem::getLabelText() { return m_label->text(); }

void FcitxKeySettingsItem::setEnableEdit(bool flag) {
    m_keyWidget->setEnableEdit(flag);
}

void FcitxKeySettingsItem::setList(const fcitx::Key &list) {
    m_keyWidget->setList(list);
}

void FcitxKeySettingsItem::doShortCutError(const QStringList &list,
                                           QString &name) {
    Q_EMIT FcitxKeySettingsItem::shortCutError(m_label->text(), list, name);
}

FcitxHotKeySettingsItem::FcitxHotKeySettingsItem(const QString &text,
                                                 const fcitx::Key &list,
                                                 QFrame *parent)
    : SettingsItem(parent) {
    m_label = new FcitxShortenLabel(text, this);
    m_keyWidget = new FcitxKeyLabelWidget(list, parent);
    m_hLayout = new QHBoxLayout(this);
    m_hLayout->setContentsMargins(10, 0, 10, 0);
    m_hLayout->addWidget(m_label);
    m_hLayout->addWidget(m_keyWidget);
    m_hLayout->setAlignment(m_label, Qt::AlignLeft);
    m_hLayout->addWidget(m_keyWidget, 0, Qt::AlignVCenter | Qt::AlignRight);
    setFixedHeight(48);
    setAccessibleName(text);
    setLayout(m_hLayout);
    connect(m_keyWidget, &FcitxKeyLabelWidget::editedFinish, this,
            &FcitxHotKeySettingsItem::editedFinish);
    connect(m_keyWidget, &FcitxKeyLabelWidget::shortCutError, this,
            &FcitxHotKeySettingsItem::doShortCutError);
}

void FcitxHotKeySettingsItem::setText(const QString &text) {
    m_label->setShortenText(text);
}

void FcitxHotKeySettingsItem::enableSingleKey() {
    m_keyWidget->enableSingleKey();
}

QString FcitxHotKeySettingsItem::getLabelText() { return m_label->text(); }

void FcitxHotKeySettingsItem::setEnableEdit(bool flag) {
    m_keyWidget->setEnableEdit(flag);
}

void FcitxHotKeySettingsItem::setList(const fcitx::Key &list) {
    m_keyWidget->setList(list);
}

void FcitxHotKeySettingsItem::doShortCutError(const QStringList &list,
                                              QString &name) {
    Q_EMIT FcitxHotKeySettingsItem::shortCutError(m_label->text(), list, name);
}

} // namespace widgets
} // namespace dcc_fcitx_configtool

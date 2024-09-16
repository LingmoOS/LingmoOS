// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keysettingsitem.h"

#include <DFontSizeManager>
#include <DCommandLinkButton>
#include <QComboBox>
#include <QMouseEvent>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QBrush>
#include <QProcess>
#include <QPainterPath>

#include "fcitxInterface/global.h"
#include "settingsgroup.h"
#include "window/immodel/imconfig.h"
#include "publisher/publisherdef.h"

namespace dcc_fcitx_configtool {
namespace widgets {
FcitxKeyLabelWidget::FcitxKeyLabelWidget(QStringList list, QWidget *p)
    : QWidget(p)
    , m_curlist(list)
{
    m_eidtFlag = true;
    if (m_curlist.isEmpty()) {
        m_curlist << tr("None");
    }
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

FcitxKeyLabelWidget::~FcitxKeyLabelWidget()
{
    clearShortcutKey();
}

void FcitxKeyLabelWidget::setKeyId(const QString &id)
{
    m_id = id;
}

void FcitxKeyLabelWidget::setList(const QStringList &list)
{
    m_curlist = list;
    m_curlist.removeDuplicates();
    initLableList(m_curlist);
}

void FcitxKeyLabelWidget::initLableList(const QStringList &list)
{
    clearShortcutKey();
    for (const QString &key : list) {
        QString tmpKey = key.toLower();
        if(tmpKey.compare("control") == 0){
            tmpKey = "ctrl";
        }
        if (!tmpKey.isEmpty()) {
            tmpKey[0] = tmpKey[0].toUpper();
        }
        FcitxKeyLabel *label = new FcitxKeyLabel(tmpKey);
        label->setAccessibleName(tmpKey);
        label->setBackgroundRole(DPalette::DarkLively);
        m_list << label;
        m_mainLayout->addWidget(label);
    }
}

QString FcitxKeyLabelWidget::getKeyToStr()
{
    QString key;
    for (int i = 0; i < m_list.count(); ++i) {
        if (i == m_list.count() - 1) {
            key += m_list[i]->text();
        } else {
            key += (m_list[i]->text() + "_");
        }
    }
    return key.toUpper();
}

void FcitxKeyLabelWidget::setEnableEdit(bool flag)
{
    m_eidtFlag = flag;
}

void FcitxKeyLabelWidget::enableSingleKey()
{
    m_isSingle = true;
}

void FcitxKeyLabelWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_eidtFlag)
        return;
    setShortcutShow(!m_keyEdit->isHidden());
    QWidget::mousePressEvent(event);
}

void FcitxKeyLabelWidget::resizeEvent(QResizeEvent *event)
{
    if (!m_eidtFlag)
        return;
    setShortcutShow(m_keyEdit->isHidden());
    QWidget::resizeEvent(event);
}

bool FcitxKeyLabelWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (m_keyEdit == watched) {
        if (event->type() == QEvent::Hide || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusOut) {
            setShortcutShow(true);
            return true;
        }
        if (event->type() == QEvent::Show) {
            setShortcutShow(false);
            return true;
        }
        if (event->type() == QEvent::KeyPress) {
            Dynamic_Cast(QKeyEvent, e, event);

            auto func = [=](QStringList &list, const QString &key) {
                clearShortcutKey();
                list.clear();
//                if((key == "Ctrl" && m_curlist.contains("CTRL", Qt::CaseInsensitive)) || (key == "Alt" && m_curlist.contains("ALT", Qt::CaseInsensitive))){
//                    return;
//                }
                list << key;
                qDebug() << "func: " << m_curlist;
                initLableList(list);
                setShortcutShow(true);
            };

            if (e) {
                if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
                    func(m_curlist, tr("None"));

                } else if (e->key() == Qt::Key_Control || e->key() == Qt::Key_Alt || m_isSingle) {
                    setFocus();
                    func(m_newlist, publisherFunc::getKeyValue(e->key()));
                } else {
                    setShortcutShow(true);
                }
                return true;
            }
            return false;
        }
    }
    return false;
}

void FcitxKeyLabelWidget::keyPressEvent(QKeyEvent *event)
{
    if (!m_eidtFlag)
        return;
    quint32 tmpScanCode = event->nativeScanCode();
    if(tmpScanCode == 64){
        m_newlist << publisherFunc::getKeyValue( Qt::Key_Alt);
    }
    else{
        m_newlist << publisherFunc::getKeyValue( event->key());
    }
    initLableList(m_newlist);
    if (m_newlist.count() >= 2 && !checkNewKey()) {
        initLableList(m_curlist);
    }
    setShortcutShow(true);
    QWidget::keyPressEvent(event);
}

void FcitxKeyLabelWidget::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "keyReleaseEvent";
    if (!m_eidtFlag)
        return;
    if ((m_newlist.count() < 2 && !m_isSingle) || !checkNewKey(true)) {
        m_curlist.removeDuplicates();
        initLableList(m_curlist);
    }
    setShortcutShow(true);
    //    QWidget::keyReleaseEvent(event);
}

void FcitxKeyLabelWidget::clearShortcutKey()
{
    for (FcitxKeyLabel *label : m_list) {
        m_mainLayout->removeWidget(label);
        label->deleteLater();
    }
    m_list.clear();
}

void FcitxKeyLabelWidget::setShortcutShow(bool flag)
{
    if (flag) {
        m_mainLayout->setContentsMargins(0, 9, 0, 9);
        m_keyEdit->hide();
        int w = 0;
        for (FcitxKeyLabel *label : m_list) {
            w += label->width() + 9;
            label->show();
        }
        setMaximumWidth(w);

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

bool FcitxKeyLabelWidget::checkNewKey(bool isRelease)
{
    QStringList list {publisherFunc::getKeyValue(Qt::Key_Control),
                      publisherFunc::getKeyValue(Qt::Key_Alt),
                      publisherFunc::getKeyValue(Qt::Key_Shift),
                      publisherFunc::getKeyValue(Qt::Key_Super_L)};

    if (m_newlist.count() == 2) {
        for (int i = 0; i < list.count(); ++i) {
            if (m_newlist.at(0) == list.at(i)) {
                if (list.indexOf(m_newlist[1]) != -1) {
                    if (m_newlist[1] != m_newlist[0]) {
                        return !isRelease;
                    }
                    return false;
                }
                if (list.indexOf(m_newlist[1]) == -1) {
                    QStringList tmpList;
                    for (const QString &key : m_newlist) {
                        QString tmpKey = key.toUpper();
                        tmpList.append(tmpKey);
                    }

                    QString configName;
                    if (m_curlist != tmpList && !IMConfig::checkShortKey(m_newlist, configName)) {
                        emit shortCutError(m_newlist, configName);
                        return false;
                    }
                    setList(m_newlist);
                    focusNextChild();
                    emit editedFinish();
                    return true;
                }
            }
        }
    }
    if (m_newlist.count() >= 3) {
        if (list.indexOf(m_newlist[0]) == -1 || list.indexOf(m_newlist[1]) == -1 || list.indexOf(m_newlist[2]) != -1) {
            focusNextChild();
            return false;
        }
        QStringList tmpList;
        for (const QString &key : m_newlist) {
            QString tmpKey = key.toUpper();
            tmpList.append(tmpKey);
        }
        QString configName;
        if (m_curlist != tmpList && !IMConfig::checkShortKey(m_newlist, configName)) {
            emit shortCutError(m_newlist, configName);
            return false;
        }
        setList(m_newlist);
        focusNextChild();
        emit editedFinish();
        return true;
    }
    if(m_newlist.count() == 1 && m_isSingle) {
        emit editedFinish();
    }
    return true;
}

FcitxKeySettingsItem::FcitxKeySettingsItem(const QString &text, const QStringList &list, QFrame *parent)
    : FcitxSettingsItem(parent)
{
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
    connect(m_keyWidget, &FcitxKeyLabelWidget::editedFinish, this, &FcitxKeySettingsItem::editedFinish);
    connect(m_keyWidget, &FcitxKeyLabelWidget::shortCutError, this, &FcitxKeySettingsItem::doShortCutError);
}

void FcitxKeySettingsItem::setText(const QString &text)
{
    m_label->setShortenText(text);
}

void FcitxKeySettingsItem::enableSingleKey()
{
    m_keyWidget->enableSingleKey();
}

void FcitxKeySettingsItem::paintEvent(QPaintEvent *event)
{
    QPainter painter( this);
    const int radius = 8;
    QRect paintRect = this->rect();
    QPainterPath path;
    path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
    path.lineTo(paintRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                     QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                     QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                     QSize(radius * 2, radius * 2)), 270, 90);


    DPalette p;
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
    if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        color = QColor("#323232");
        color.setAlpha(230);
    } else {
        color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
    }
    if(isDraged()) {
        color = DGuiApplicationHelper::instance()->applicationPalette().light().color();
        color.setAlpha(80);
    }
    painter.fillPath(path, color);
    return FcitxSettingsItem::paintEvent(event);
}

QString FcitxKeySettingsItem::getLabelText()
{
    return m_label->text();
}

void FcitxKeySettingsItem::setEnableEdit(bool flag)
{
    m_keyWidget->setEnableEdit(flag);
}

void FcitxKeySettingsItem::setKeyId(const QString &id)
{
    m_keyWidget->setKeyId(id);
}

void FcitxKeySettingsItem::setList(const QStringList &list)
{
    m_keyWidget->setList(list);
}

void FcitxKeySettingsItem::resizeEvent(QResizeEvent *event)
{
    updateSize();
    FcitxSettingsItem::resizeEvent(event);
}

//void FcitxKeySettingsItem::paintEvent(QPaintEvent *event)
//{
//    updateSize();
//    FcitxSettingsItem::paintEvent(event);
//}

void FcitxKeySettingsItem::doShortCutError(const QStringList &list, QString &name)
{
    emit FcitxKeySettingsItem::shortCutError(m_label->text(), list, name);
}

void FcitxKeySettingsItem::updateSize()
{
    int v = width() - m_keyWidget->width() - 32;
    int titleWidth = publisherFunc::fontSize(m_label->text());
    if (titleWidth <= v) {
        m_label->setFixedWidth(titleWidth);
    } else {
        m_label->setFixedWidth(v);
    }
}

FcitxHotKeySettingsItem::FcitxHotKeySettingsItem(const QString &text, const QStringList &list, QFrame *parent)
    : FcitxSettingsItem(parent)
{
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
    connect(m_keyWidget, &FcitxKeyLabelWidget::editedFinish, this, &FcitxHotKeySettingsItem::editedFinish);
    connect(m_keyWidget, &FcitxKeyLabelWidget::shortCutError, this, &FcitxHotKeySettingsItem::doShortCutError);
}

void FcitxHotKeySettingsItem::setText(const QString &text)
{
    m_label->setShortenText(text);
}

void FcitxHotKeySettingsItem::enableSingleKey()
{
    m_keyWidget->enableSingleKey();
}

QString FcitxHotKeySettingsItem::getLabelText()
{
    return m_label->text();
}

void FcitxHotKeySettingsItem::setEnableEdit(bool flag)
{
    m_keyWidget->setEnableEdit(flag);
}

void FcitxHotKeySettingsItem::setKeyId(const QString &id)
{
    m_keyWidget->setKeyId(id);
}

void FcitxHotKeySettingsItem::setList(const QStringList &list)
{
    m_keyWidget->setList(list);
}

void FcitxHotKeySettingsItem::resizeEvent(QResizeEvent *event)
{
    updateSize();
    FcitxSettingsItem::resizeEvent(event);
}

void FcitxHotKeySettingsItem::doShortCutError(const QStringList &list, QString &name)
{
    emit FcitxHotKeySettingsItem::shortCutError(m_label->text(), list, name);
}

void FcitxHotKeySettingsItem::updateSize()
{
    int v = width() - m_keyWidget->width() - 32;
    int titleWidth = publisherFunc::fontSize(m_label->text());
    if (titleWidth <= v) {
        m_label->setFixedWidth(titleWidth);
    } else {
        m_label->setFixedWidth(v);
    }
}

FcitxComBoboxSettingsItem::FcitxComBoboxSettingsItem(const QString &text, const QStringList &list, QFrame *parent)
    : FcitxSettingsItem(parent)
{
    m_combox = new QComboBox(this);
    m_combox->setFixedHeight(36);
    m_combox->addItems(list);
    m_label = new FcitxShortenLabel(text, this);
    DFontSizeManager::instance()->bind(m_label, DFontSizeManager::T6);
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->addWidget(m_label);
    m_mainLayout->addWidget(m_combox);
    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    setLayout(m_mainLayout);
    setFixedHeight(48);
}

QString FcitxComBoboxSettingsItem::getLabelText()
{
    return m_label->text();
}

void FcitxComBoboxSettingsItem::paintEvent(QPaintEvent *event)
{
    QPainter painter( this);
    const int radius = 8;
    QRect paintRect = this->rect();
    QPainterPath path;
    path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
    path.lineTo(paintRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                     QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                     QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                     QSize(radius * 2, radius * 2)), 270, 90);


    DPalette p;
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
    if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        color = QColor("#323232");
        color.setAlpha(230);
    } else {
        color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
    }
    if(isDraged()) {
        color = DGuiApplicationHelper::instance()->applicationPalette().light().color();
        color.setAlpha(80);
    }
    painter.fillPath(path, color);
    return FcitxSettingsItem::paintEvent(event);
}

FcitxComBoboxSettingsItem::~FcitxComBoboxSettingsItem()
{
}

FcitxCheckBoxSettingsItem::FcitxCheckBoxSettingsItem(FcitxAddon *addon, QWidget *parent)
    : FcitxSettingsItem(parent)
{
    QHBoxLayout* horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    QCheckBox *checkbox = new QCheckBox();
    checkbox->setAccessibleName(addon->name);
    checkbox->setMaximumWidth(20);
    horizontalLayout->addSpacing(10);
    horizontalLayout->addWidget(checkbox, Qt::AlignLeft);
    horizontalLayout->addSpacing(10);
    checkbox->setCheckState(addon->bEnabled ? Qt::Checked : Qt::Unchecked);
    connect(checkbox, &QCheckBox::clicked, this, [=](bool value){
        addon->bEnabled = value;
        QString buf = QString("%1.conf").arg(addon->name);
        FILE* fp = FcitxXDGGetFileUserWithPrefix("addon", buf.toLocal8Bit().constData(), "w", nullptr);
        if (fp) {
            fprintf(fp, "[Addon]\nEnabled=%s\n", addon->bEnabled ? "True" : "False");
            fclose(fp);
//            if(Fcitx::Global::instance()->inputMethodProxy() != nullptr) {
//                Fcitx::Global::instance()->inputMethodProxy()->ReloadConfig();
//            }
            emit onChecked();
        }
    });

    QHBoxLayout* hLayout2 = new QHBoxLayout();
    hLayout2->setContentsMargins(0, 0, 0, 0);
    hLayout2->setSpacing(0);
    QLabel* label2 = new QLabel();
    label2->setText(addon->generalname);
    QFont f;
    f.setPixelSize(13);
    f.setFamily("SourceHanSansSC");
    f.setWeight(QFont::DemiBold);
    label2->setFont(f);
    hLayout2->addWidget(label2, Qt::AlignLeft);

    QString configDescName = QString(addon->name) + ".desc";
    QByteArray configDescNamestr = configDescName.toLocal8Bit();
    FcitxConfigFileDesc* cfdesc = getConfigDesc(configDescNamestr.data());
    bool configurable = (cfdesc != nullptr || strlen(addon->subconfig) != 0);
    if(!QString(addon->subconfig).isEmpty() || configurable) {
        DCommandLinkButton* label = new DCommandLinkButton(tr("Configure"));
        DFontSizeManager::instance()->bind(label, DFontSizeManager::T8);
        hLayout2->addStretch();
        hLayout2->addWidget(label);
        hLayout2->addSpacing(8);
        connect(label, &DCommandLinkButton::clicked, this, [=](){
            if (QString(addon->name).contains("iflyime")) {
                QProcess::startDetached("sh -c " + IMConfig::IMPluginKey("iflyime"));
                return;
            }
            QProcess::startDetached("sh -c \"fcitx-config-gtk3 " + QString(addon->name) + "\"");
        });
    }
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(8);
    vlayout->addLayout(hLayout2);
    PushLable* label3 = new PushLable();
    label3->setOriginText(addon->comment);
    QFont f2;
    f2.setPixelSize(12);
    label3->setFont(f2);
    label3->setMidLineWidth(350);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText, QColor("#526a7f"));
    label3->setPalette(pal2);
    vlayout->addWidget(label3);
    vlayout->addSpacing(8);
    horizontalLayout->addLayout(vlayout);
    this->setLayout(horizontalLayout);
}

FcitxConfigFileDesc *FcitxCheckBoxSettingsItem::getConfigDesc(char *filename)
{
    ConfigDescSet *desc = nullptr;
    //HASH_FIND_STR(m_configDescSet, filename, desc);
    if (!desc) {
        FILE * tmpfp = FcitxXDGGetFileWithPrefix("configdesc", filename, "r", nullptr);
        if (tmpfp) {
            desc = static_cast<ConfigDescSet *>(malloc(sizeof(ConfigDescSet))) ;
            memset(desc, 0 , sizeof(ConfigDescSet));
            desc->filename = strdup(filename);
            desc->cfdesc = FcitxConfigParseConfigFileDescFp(tmpfp);
            fclose(tmpfp);

           // HASH_ADD_KEYPTR(hh, m_configDescSet, desc->filename, strlen(desc->filename), desc);
        } else {
            return nullptr;
        }
    }

    return desc->cfdesc;
}

FcitxCheckBoxSettingsItem::~FcitxCheckBoxSettingsItem()
{

}

PushLable::PushLable(QWidget *parent)
    : QLabel (parent)
{

}

void PushLable::setOriginText(const QString &text)
{
    setText(text);
    m_originText = text;
}

void PushLable::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    emit clicked();
}

void PushLable::resizeEvent(QResizeEvent *event)
{
    if(!m_originText.isEmpty()) {
        QString name = fontMetrics().elidedText(m_originText, Qt::ElideRight, width());
        setText(name);
        if(name != m_originText) {
            setToolTip(m_originText);
        } else {
            setToolTip("");
        }
    }
    QLabel::resizeEvent(event);
}

FcitxGlobalSettingsItem::FcitxGlobalSettingsItem(QFrame *parent)
    : FcitxSettingsItem(parent)
{

}

FcitxGlobalSettingsItem::~FcitxGlobalSettingsItem()
{

}

void FcitxGlobalSettingsItem::paintEvent(QPaintEvent *event)
{
    QPainter painter( this);
    const int radius = 8;
    QRect paintRect = this->rect();
    QPainterPath path;
    if(m_index == firstItem) {
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight() + QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                         QSize(radius * 2, radius * 2)), 0, 90);
        path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
    } if(m_index == lastItem) {
        path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                         QSize(radius * 2, radius * 2)), 180, 90);
        path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                         QSize(radius * 2, radius * 2)), 270, 90);
    } if(m_index == otherItem) {
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
    }
    DPalette p;

    QColor color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
    if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        color = QColor("#323232");
        color.setAlpha(230);
    } else {
        color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
        color.setAlphaF(0.04);
    }
    painter.fillPath(path, color);
    return FcitxSettingsItem::paintEvent(event);
}

void FcitxGlobalSettingsItem::mouseMoveEvent(QMouseEvent *e)
{
    update(rect());
    return FcitxSettingsItem::mouseMoveEvent(e);
}

void FcitxGlobalSettingsItem::resizeEvent(QResizeEvent *event)
{
    FcitxSettingsItem::resizeEvent(event);
}

} // namespace widgets
} // namespace dcc_fcitx_configtool

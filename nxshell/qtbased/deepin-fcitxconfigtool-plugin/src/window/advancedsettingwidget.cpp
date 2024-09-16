// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "advancedsettingwidget.h"
// Qt
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QStackedLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QPushButton>
#include <QDebug>
#include <QStandardItemModel>
#include <QListView>
#include <QComboBox>
#include <QLineEdit>
#include <QMatrix>
#include <QTimer>
#include <QGSettings>
#include <DButtonBox>
#include <DPushButton>
#include <DSwitchButton>
#include <KLocalizedString>
#include <DSpinBox>
// system
#include <libintl.h>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>
#include <fcitxqtkeysequencewidget.h>

// self
#include "fcitxInterface/config.h"
#include "fcitxInterface/dummyconfig.h"
#include "fcitxInterface/verticalscrollarea.h"
#include "widgets/keysettingsitem.h"
#include "window/immodel/imconfig.h"
#include "fcitxInterface/global.h"
#include "settingsdef.h"
#include "gsettingwatcher.h"

DWIDGET_USE_NAMESPACE
namespace Fcitx {

static void SyncFilterFunc(FcitxGenericConfig *gconfig, FcitxConfigGroup *group, FcitxConfigOption *option, void *value, FcitxConfigSync sync, void *arg);

const UT_icd addonicd = {sizeof(FcitxAddon), nullptr, nullptr, FcitxAddonFree};

AdvancedSettingWidget::AdvancedSettingWidget(QWidget *parent)
    : QWidget(parent)
    , m_hash(new QHash<QString, FcitxConfigFileDesc *>)
    , m_prefix("")
    , m_name("config")
    , m_addonName("global")
    , m_globalSettingsLayout(new QVBoxLayout)
    , m_addOnsLayout(new QVBoxLayout)
    , m_simpleUiType(CW_NoShow)
    , m_fullUiType(CW_NoShow)
{
    getConfigDesc("config.desc");
    setupConfigUi();
    connect(Global::instance()->inputMethodProxy(), &FcitxQtInputMethodProxy::ReloadConfigUI, this, [ & ]() {
        if (m_config.isValid() && (!m_isSelfSend)) {
            FILE *fp;
            fp = FcitxXDGGetFileWithPrefix(m_prefix.toLocal8Bit().constData(), m_name.toLocal8Bit().constData(), "r", nullptr);
            if (fp) {
                m_config.load(fp);
                m_config.configSyncToUi();
                fclose(fp);
            }
        }
    });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AdvancedSettingWidget::onPalettetypechanged);
}

void AdvancedSettingWidget::setupConfigUi()
{
    DPalette p;
    p.setBrush(DPalette::Background, DGuiApplicationHelper::instance()->applicationPalette().base());
    this->setPalette(p);
    DButtonBoxButton *btnGlobalSettings = new DButtonBoxButton(tr("Global Config"));
    GSettingWatcher::instance()->bind(GSETTINGS_ADVANCESETTING_GLOBALCONFIG, btnGlobalSettings);
    btnGlobalSettings->setAccessibleName("globalSettings");
    DButtonBoxButton *btnAddOns = new DButtonBoxButton(tr("Add-ons"));
    GSettingWatcher::instance()->bind(GSETTINGS_ADVANCESETTING_ADDONS, btnAddOns);
    btnAddOns->setAccessibleName("addons");
    QList<DButtonBoxButton *> pBtnlist;
    pBtnlist.append(btnGlobalSettings);
    pBtnlist.append(btnAddOns);
    DButtonBox *pTopSwitchWidgetBtn = new DButtonBox;
    pTopSwitchWidgetBtn->setButtonList(pBtnlist, true);
    pBtnlist.first()->setChecked(true);
    pTopSwitchWidgetBtn->setId(btnGlobalSettings, 0);
    pTopSwitchWidgetBtn->setId(btnAddOns, 1);
    pTopSwitchWidgetBtn->setMinimumSize(240, 36);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(pTopSwitchWidgetBtn,  0, Qt::AlignHCenter);
    QStackedLayout *stackLayout = new QStackedLayout;
    stackLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *globalSettingsWidget = new QWidget;
    GSettingWatcher::instance()->bind(GSETTINGS_ADVANCESETTING_GLOBALCONFIG, globalSettingsWidget);
    globalSettingsWidget->setLayout(m_globalSettingsLayout);
    QWidget *addOnsWidget = new QWidget;
    //GSettingWatcher::instance()->bind(GSETTINGS_ADVANCESETTING_ADDONS, addOnsWidget);
    addOnsWidget->setLayout(m_addOnsLayout);
    stackLayout->addWidget(globalSettingsWidget);
    stackLayout->addWidget(addOnsWidget);
    layout->addSpacing(10);
    layout->addLayout(stackLayout);
    connect(btnGlobalSettings, &DButtonBoxButton::clicked, this, [ = ]() {
        stackLayout->setCurrentIndex(0);
    });
    connect(btnAddOns, &DButtonBoxButton::clicked, this, [ = ]() {
        stackLayout->setCurrentIndex(1);
    });
    setLayout(layout);

    if (m_cfdesc) {
        bindtextdomain(m_cfdesc->domain, LOCALEDIR);
        bind_textdomain_codeset(m_cfdesc->domain, "UTF-8");
        FILE *fp;
        fp = FcitxXDGGetFileWithPrefix(m_prefix.toLocal8Bit().constData(), m_name.toLocal8Bit().constData(), "r", nullptr);
        if (fp) {
            m_config.load(fp);
            fclose(fp);
        }
    }
    m_globalSettingsLayout->setContentsMargins(0, 0, 0, 0);
    m_addOnsLayout->setContentsMargins(0, 0, 0, 0);
    m_globalSettingsLayout->addWidget(createglobalSettingsUi());
    m_addOnsLayout->addWidget(createAddOnsUi());

    if (m_config.isValid()) {
        m_config.configSyncToUi();
    }
}

AdvancedSettingWidget::~AdvancedSettingWidget()
{
}

void AdvancedSettingWidget::createConfigOptionWidget(FcitxConfigGroupDesc *cgdesc, FcitxConfigOptionDesc *codesc, QString &label,
                                                     QString &tooltip, QWidget *&inputWidget, void *&newarg)
{
    FcitxConfigOptionDesc2 *codesc2 = (FcitxConfigOptionDesc2 *) codesc;

    QString name(QString("%1/%2").arg(cgdesc->groupName).arg(codesc->optionName));
    if (m_argsMap.contains(name)) {
    }

    if (codesc->desc && codesc->desc[0]) {
        label = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->desc));
    } else {
        label = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->optionName));
    }
    if (codesc2->longDesc && codesc2->longDesc[0]) {
        tooltip = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc2->longDesc));
    }

    switch (codesc->type) {
    case T_Integer: {
        DSpinBox *spinbox = new DSpinBox(this);
        spinbox->setAccessibleName(name.split('/').last());
        spinbox->setEnabledEmbedStyle(true);
        spinbox->setMinimumWidth(50);
        spinbox->setMaximumHeight(48);
        spinbox->setMaximum(codesc2->constrain.integerConstrain.max);
        spinbox->setMinimum(codesc2->constrain.integerConstrain.min);
        inputWidget = spinbox;
        connect(spinbox,  QOverload<int>::of(&DSpinBox::valueChanged), this, [ = ](int value) {
            IMConfig::setIMvalue(codesc->optionName, QString().number(value));
            sendReloadMessage();
        });
        break;
    }
    case T_Boolean: {
        DSwitchButton *pSwitchBtn = new DSwitchButton;
        pSwitchBtn->setAccessibleName(name.split('/').last());
        if (QString(codesc->rawDefaultValue).contains("True")) {
            pSwitchBtn->setChecked(true);
        }
        inputWidget = pSwitchBtn;
        connect(pSwitchBtn, &DSwitchButton::clicked, this, [ = ](bool checked) {
            IMConfig::setIMvalue(codesc->optionName, checked ? "True" : "False");
            sendReloadMessage();
        });
        break;
    }
    case T_Enum: {
        int i;
        FcitxConfigEnum *e = &codesc->configEnum;
        QComboBox *combobox = new QComboBox(this);
        combobox->setAccessibleName(name.split('/').last());
        combobox->setMinimumWidth(150);
        combobox->setMaximumHeight(48);
        inputWidget = combobox;

        for (i = 0; i < e->enumCount; i ++) {
            combobox->addItem(QString::fromUtf8(dgettext(m_cfdesc->domain, e->enumDesc[i])));
            combobox->setItemData(i, e->enumDesc[i]);
        }
        connect(combobox, &QComboBox::currentTextChanged, this, [ = ](const QString & text) {
            int index = combobox->findText(text);
            IMConfig::setIMvalue(codesc->optionName, combobox->itemData(index).toString());
            sendReloadMessage();
        });
        break;
    }
    case T_Hotkey: {
        FcitxHotKeySettingsItem *item = new FcitxHotKeySettingsItem;
        item->setAccessibleName(name.split('/').last());
        item->setMaximumWidth(150);
        item->enableSingleKey();
        item->setList(QString(codesc->rawDefaultValue).split(' ').first().split('_'));
        if (QString(codesc->rawDefaultValue).isEmpty()) {
            item->setList(QString(tr("None")).split('_'));
        }
        inputWidget = item;
        connect(item, &FcitxHotKeySettingsItem::editedFinish, [ = ]() {
            QString str = item->getKeyToStr();
            IMConfig::setIMvalue(codesc->optionName, item->getKeyToStr());
            item->setList(item->getKeyToStr().split("_"));
            sendReloadMessage();
        });
        break;
    };
    case T_Char:
    case T_String: {
        QLineEdit *lineEdit = new QLineEdit(this);
        lineEdit->setMinimumWidth(150);
        inputWidget = lineEdit;
        connect(lineEdit, &QLineEdit::editingFinished, [ = ]() {
            IMConfig::setIMvalue(codesc->optionName, lineEdit->text());
            sendReloadMessage();
        });
        break;
    }
    case T_I18NString:
        inputWidget = nullptr;
        break;
    default:
        break;
    }


    if (inputWidget && !tooltip.isEmpty())
        inputWidget->setToolTip(tooltip);

    if (inputWidget) {
        m_argsMap[name] = inputWidget;
        newarg = inputWidget;
    }
}

QWidget *AdvancedSettingWidget::createglobalSettingsUi()
{
    int row = 0;
    VerticalScrollArea *scrollarea = new VerticalScrollArea;
    scrollarea->setFrameStyle(QFrame::NoFrame);
    scrollarea->setWidgetResizable(true);

    QWidget *form = new QWidget;
    QVBoxLayout *vgLayout = new QVBoxLayout;
    vgLayout->setContentsMargins(20, 0, 20, 0);
    scrollarea->setWidget(form);
    form->setLayout(vgLayout);

    do {
        if (!m_cfdesc)
            break;

        if (!m_config.isValid())
            break;

        HASH_FOREACH(cgdesc, m_cfdesc->groupsDesc, FcitxConfigGroupDesc) {
            ArrowButton *grouplabel = new ArrowButton();
            grouplabel->setText(QString("%1").arg(QString::fromUtf8(dgettext(m_cfdesc->domain, cgdesc->groupName))));
            grouplabel->setAccessibleName(cgdesc->groupName);
            QFont f;
            f.setPixelSize(17);
            f.setWeight(QFont::DemiBold);
            grouplabel->setFont(f);
            ArrowButton *button = new ArrowButton();
            QPixmap pmap = QIcon::fromTheme("dm_arrow").pixmap(QSize(12, 8));
            button->setPixmap(pmap);
            button->setMaximumWidth(30);
            QHBoxLayout *hglayout = new QHBoxLayout;
            QGSettings *gsetting = new QGSettings("com.deepin.fcitx-config", QByteArray(), this);
            if (QString(cgdesc->groupName) == "Hotkey") {
                QString value = gsetting->get(GSETTINGS_GLOBALCONFIG_SHORTCUT).toString();
                if ("Enabled" == value) {
                    grouplabel->setEnabled(true);
                    button->setEnabled(true);
                } else if ("Disabled" == value) {
                    grouplabel->setEnabled(false);
                    button->setEnabled(false);
                }
                grouplabel->setVisible("Hidden" != value);
                button->setVisible("Hidden" != value);

                GSettingWatcher::instance()->bind(GSETTINGS_GLOBALCONFIG_SHORTCUT, grouplabel);
                GSettingWatcher::instance()->bind(GSETTINGS_GLOBALCONFIG_SHORTCUT, button);
            } else if (QString(cgdesc->groupName) == "Program") {
                QString value = gsetting->get(GSETTINGS_GLOBALCONFIG_PROGRAM).toString();
                if ("Enabled" == value) {
                    grouplabel->setEnabled(true);
                    button->setEnabled(true);
                } else if ("Disabled" == value) {
                    grouplabel->setEnabled(false);
                    button->setEnabled(false);
                }
                grouplabel->setVisible("Hidden" != value);
                button->setVisible("Hidden" != value);

                GSettingWatcher::instance()->bind(GSETTINGS_GLOBALCONFIG_PROGRAM, grouplabel);
                GSettingWatcher::instance()->bind(GSETTINGS_GLOBALCONFIG_PROGRAM, button);
            } else if (QString(cgdesc->groupName) == "Output") {
                QString value = gsetting->get(GSETTINGS_GLOBALCONFIG_OUTPUT).toString();
                if ("Enabled" == value) {
                    grouplabel->setEnabled(true);
                    button->setEnabled(true);
                } else if ("Disabled" == value) {
                    grouplabel->setEnabled(false);
                    button->setEnabled(false);
                }
                grouplabel->setVisible("Hidden" != value);
                button->setVisible("Hidden" != value);

                GSettingWatcher::instance()->bind(GSETTINGS_GLOBALCONFIG_OUTPUT, grouplabel);
                GSettingWatcher::instance()->bind(GSETTINGS_GLOBALCONFIG_OUTPUT, button);
            } else if (QString(cgdesc->groupName) == "Appearance") {
                QString value = gsetting->get(GSETTINGS_GLOBALCONFIG_APPEARANCE).toString();
                if ("Enabled" == value) {
                    grouplabel->setEnabled(true);
                    button->setEnabled(true);
                } else if ("Disabled" == value) {
                    grouplabel->setEnabled(false);
                    button->setEnabled(false);
                }
                grouplabel->setVisible("Hidden" != value);
                button->setVisible("Hidden" != value);

                GSettingWatcher::instance()->bind(GSETTINGS_GLOBALCONFIG_APPEARANCE, grouplabel);
                GSettingWatcher::instance()->bind(GSETTINGS_GLOBALCONFIG_APPEARANCE, button);
            }
            QString value = gsetting->get(GSETTINGS_ADVANCESETTING_GLOBALCONFIG).toString();
            if ("Enabled" == value) {
                grouplabel->setEnabled(true);
                button->setEnabled(true);
            } else if ("Disabled" == value) {
                grouplabel->setEnabled(false);
                button->setEnabled(false);
            }
            grouplabel->setVisible("Hidden" != value);
            button->setVisible("Hidden" != value);
            GSettingWatcher::instance()->bind(GSETTINGS_ADVANCESETTING_GLOBALCONFIG, grouplabel);
            GSettingWatcher::instance()->bind(GSETTINGS_ADVANCESETTING_GLOBALCONFIG, button);

            hglayout->addSpacing(10);
            hglayout->addWidget(grouplabel);
            hglayout->addWidget(button, Qt::AlignRight);
            vgLayout->addLayout(hglayout);
            QWidget *content = new QWidget;
            QSizePolicy policy;
            policy.setVerticalPolicy(QSizePolicy::Fixed);
            policy.setHorizontalPolicy(QSizePolicy::Expanding);
            content->setSizePolicy(policy);
            QVBoxLayout *vlayout = new QVBoxLayout;
            vlayout->setContentsMargins(0, 0, 0, 0);
            content->setLayout(vlayout);
            content->setHidden(true);
            connect(grouplabel, &ArrowButton::pressed, this, [ = ](bool isHidden) {
                content->setHidden(isHidden);
                button->setContentHidden(!isHidden);
                QMatrix matrix;
                if (!isHidden) {
                    matrix.rotate(180);
                } else {
                    matrix.rotate(0);
                }
                button->setPixmap(pmap.transformed(matrix, Qt::SmoothTransformation));
            });
            connect(button, &ArrowButton::pressed, this, [ = ](bool isHidden) {
                content->setHidden(isHidden);
                grouplabel->setContentHidden(!isHidden);
                QMatrix matrix;
                if (!isHidden) {
                    matrix.rotate(180);
                } else {
                    matrix.rotate(0);
                }
                button->setPixmap(pmap.transformed(matrix, Qt::SmoothTransformation));
            });
            vgLayout->addWidget(content, Qt::AlignLeft);
            vgLayout->addSpacing(7);
            QList<FcitxGlobalSettingsItem *> itemList;
            HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                QString s, tooltip;
                QWidget *inputWidget = nullptr;
                void *argument = nullptr;
                FcitxConfigOptionDesc2 *codesc2 = (FcitxConfigOptionDesc2 *) codesc;
                boolean shownInDeepin = codesc2->shownInDeepin;
                if (shownInDeepin) {
                    createConfigOptionWidget(cgdesc, codesc, s, tooltip, inputWidget, argument);
                }
                if (inputWidget) {
                    ArrowButton *label = new ArrowButton;
                    label->setOriginText(s);
                    QFont f;
                    f.setPixelSize(13);
                    //f.setWeight(QFont::DemiBold);
                    label->setFont(f);
                    label->setMinimumWidth(100);
                    if (!tooltip.isEmpty()) {
                        label->setToolTip(tooltip);
                    }
                    FcitxGlobalSettingsItem *pitem = new FcitxGlobalSettingsItem;
                    itemList.append(pitem);
                    pitem->setMinimumWidth(100);
                    QHBoxLayout *hlayout = new QHBoxLayout;
                    if (codesc->type == T_Hotkey) {
                        pitem->setMaximumHeight(46);
                        label->setAlignment(Qt::AlignVCenter);
                        hlayout->setContentsMargins(10, 0, 0, 10);
                    } else {
                        hlayout->setContentsMargins(10, 5, 5, 10);
                    }
                    hlayout->addWidget(label);
                    hlayout->addWidget(inputWidget, Qt::AlignCenter);
                    pitem->setLayout(hlayout);
                    vlayout->addWidget(pitem);
                    vlayout->addSpacing(-5);
                    if (argument) {
                        m_config.bind(cgdesc->groupName, codesc->optionName, SyncFilterFunc, argument);
                    }
                    row++;
                }
            }
            itemList.first()->setIndex(0);
            itemList.last()->setIndex(-1);
        }
    } while (0);


    //QSpacerItem* verticalSpacer = new QSpacerItem(20, 10);

//    if (row >= 2) {
//        QSpacerItem* horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
//        vgLayout->addItem(horizontalSpacer);
//    }

    vgLayout->addSpacing(10);
    vgLayout->addStretch();
    return scrollarea;
}

QWidget *AdvancedSettingWidget::createAddOnsUi()
{
    VerticalScrollArea *scrollarea = new VerticalScrollArea;
    scrollarea->setFrameStyle(QFrame::NoFrame);
    scrollarea->setWidgetResizable(true);
    QWidget *area = new QWidget(this);
    QSizePolicy policy;
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    area->setSizePolicy(policy);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(20, 0, 20, 0);
    area->setLayout(layout);
    scrollarea->setWidget(area);
    if (FcitxAddonGetConfigDesc() != nullptr) {
        utarray_new(m_addons, &addonicd);
        FcitxAddonsLoad(m_addons);
        if (m_addons) {
            for (FcitxAddon *addon = (FcitxAddon *) utarray_front(m_addons);
                    addon != nullptr; addon = (FcitxAddon *) utarray_next(m_addons, addon)) {
                if (addon->advance) {
                    continue;
                }
                FcitxCheckBoxSettingsItem *item = new FcitxCheckBoxSettingsItem(addon, area);
                item->addBackground();
                item->setMinimumWidth(200);
                layout->addWidget(item);
                layout->addSpacing(5);
                m_addonsList.append(item);
                connect(item, &FcitxCheckBoxSettingsItem::onChecked, this, [ = ]() {
                    QProcess p;
                    p.startDetached("sh -c \"fcitx -r\"");
                    p.waitForFinished();
                    //for (auto item : m_addonsList) {
                        //item->setEnabled(false);
                        area->setEnabled(false);
                        QTimer::singleShot(2000, this, [ = ]() {
                            //item->setEnabled(true);
                            area->setEnabled(true);
                        });
                    //}
                });
            }
        }
    }

    return scrollarea;
}

void AdvancedSettingWidget::getConfigDesc(const QString &name)
{
    if (m_hash->count(name) <= 0) {
        FILE *fp = FcitxXDGGetFileWithPrefix("configdesc", name.toLatin1().constData(), "r", nullptr);
        m_cfdesc = FcitxConfigParseConfigFileDescFp(fp);

        if (m_cfdesc) {
            m_hash->insert(name, m_cfdesc);
        }
    } else {
        m_cfdesc = (*m_hash)[name];
    }
    if (m_cfdesc) {
        m_config.setConfigDesc(m_cfdesc);
    }
    return;
}

void AdvancedSettingWidget::sendReloadMessage()
{
    if (Global::instance()->inputMethodProxy() != nullptr) {
        Global::instance()->inputMethodProxy()->ReloadConfig();
    }
    m_isSelfSend = true;
    QTimer::singleShot(200, this, [ = ]() {
        m_isSelfSend = false;
    });
}

void AdvancedSettingWidget::onPalettetypechanged()
{
    DPalette p;
    p.setBrush(DPalette::Background, DGuiApplicationHelper::instance()->applicationPalette().base());
    this->setPalette(p);
}

void SyncFilterFunc(FcitxGenericConfig *gconfig, FcitxConfigGroup *group, FcitxConfigOption *option,
                    void *value, FcitxConfigSync sync, void *arg)
{
    Q_UNUSED(gconfig);
    Q_UNUSED(group);
    Q_UNUSED(value);
    FcitxConfigOptionDesc *codesc = option->optionDesc;

    if (!codesc) {
        return;
    }

    if (sync == Raw2Value) {
        switch (codesc->type) {
        case T_I18NString:
            break;
        case T_Integer: {
            int i = *(int *) value;
            DSpinBox *spinbox = static_cast<DSpinBox *>(arg);
            spinbox->setValue(i);
            break;
        }
        case T_Boolean: {
            boolean *bl = static_cast<boolean *>(value);
            DSwitchButton *checkBox = static_cast<DSwitchButton *>(arg);
            checkBox->setChecked(*bl);
            break;
        }
        case T_Enum: {
            int index = *(int *) value;
            QComboBox *combobox = static_cast<QComboBox *>(arg);
            combobox->setCurrentIndex(index);
            break;
        }
        case T_Hotkey: {
            FcitxHotkey *hotkey = static_cast<FcitxHotkey *>(value);
            FcitxHotKeySettingsItem *item = static_cast<FcitxHotKeySettingsItem *>(arg);
            item->setList(QString(hotkey->desc).split("_"));
            if (QString(hotkey->desc).isEmpty()) {
                item->setList(QString(QObject::tr("None")).split('_'));
            }
            break;
        }
        case T_Char: {
            char *string = static_cast<char *>(value);
            char temp[2] = { *string, '\0' };
            QLineEdit *lineEdit = static_cast<QLineEdit *>(arg);
            lineEdit->setText(QString::fromUtf8(temp));
            break;
        }
        case T_String: {
            char **string = static_cast<char **>(value);
            QLineEdit *lineEdit = static_cast<QLineEdit *>(arg);
            lineEdit->setText(QString::fromUtf8(*string));
            break;
        }
        default:
            break;
        }
    } else {
        if (codesc->type != T_I18NString && option->rawValue) {
            free(option->rawValue);
            option->rawValue = nullptr;
        }

        switch (codesc->type) {
        case T_I18NString:
            break;
        case T_Integer: {
            int *i = static_cast<int *>(value);
            DSpinBox *spinbox = static_cast<DSpinBox *>(arg);
            *i = spinbox->value();
            break;
        }
        case T_Boolean: {
            QCheckBox *checkBox = static_cast<QCheckBox *>(arg);
            boolean *bl = static_cast<boolean *>(value);
            *bl = checkBox->isChecked();
            break;
        }
        case T_Enum: {
            QComboBox *combobox = static_cast<QComboBox *>(arg);
            int *index = static_cast<int *>(value);
            *index = combobox->currentIndex();
            break;
        }
        case T_Hotkey: {
            FcitxHotKeySettingsItem *item = static_cast<FcitxHotKeySettingsItem *>(arg);
            FcitxHotkey *hotkey = static_cast<FcitxHotkey *>(value);
            item->setList(QString(hotkey->desc).split("_"));
            if (QString(codesc->rawDefaultValue).isEmpty()) {
                item->setList(QString(QObject::tr("None")).split('_'));
            }
            break;
        }
        case T_Char: {
            QLineEdit *lineEdit = static_cast<QLineEdit *>(arg);
            char *c = static_cast<char *>(value);
            *c = lineEdit->text()[0].toLatin1();
            break;
        }
        case T_String: {
            QLineEdit *lineEdit = static_cast<QLineEdit *>(arg);
            char **string = static_cast<char **>(value);
            fcitx_utils_string_swap(string, lineEdit->text().toUtf8().constData());
            break;
        }
        default:
            break;
        }
    }
}

ArrowButton::ArrowButton(QWidget *parent)
    : QLabel(parent)
{

}

void ArrowButton::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    emit pressed(m_hide);
    m_hide = !m_hide;
}

void ArrowButton::resizeEvent(QResizeEvent *event)
{
    if (!m_originText.isEmpty()) {
        QString name = fontMetrics().elidedText(m_originText, Qt::ElideRight, width());
        setText(name);
        if (name != m_originText) {
            setToolTip(m_originText);
        } else {
            setToolTip("");
        }
    }
    QLabel::resizeEvent(event);
}

void ArrowButton::setContentHidden(bool hide)
{
    m_hide = hide;
}

void ArrowButton::setOriginText(const QString &text)
{
    setText(text);
    m_originText = text;
}

}

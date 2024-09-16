// SPDX-FileCopyrightText: 2011-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "settings.h"

#include "dthememanager.h"
#include "../controls/fontitemdelegate.h"
#include "../widgets/pathsettintwgt.h"
#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <DGuiApplicationHelper>

Settings *Settings::s_pSetting = nullptr;

CustemBackend::CustemBackend(const QString &filepath, QObject *parent)
    : DSettingsBackend (parent),
      m_settings (new QSettings(filepath, QSettings::IniFormat))
{

}

void CustemBackend::doSync()
{
    m_settings->sync();
}

void CustemBackend::doSetOption(const QString &key, const QVariant &value)
{
    /*
     * 将配置值写入config配置文件
     * 为了规避数据写入重复或者错乱，配置写入之前上锁，写入结束后开锁
     */
    m_writeLock.lock();
    m_settings->setValue(key, value);
    m_settings->sync();
    m_writeLock.unlock();
}

QStringList CustemBackend::keys() const
{
    QStringList keyList = m_settings->allKeys();

    return keyList;
}

QVariant CustemBackend::getOption(const QString &key) const
{
    return m_settings->value(key);
}

CustemBackend::~CustemBackend()
{}

Settings::Settings(QWidget *parent)
    : QObject(parent)
{
    QString strConfigPath = QString("%1/%2/%3/config.conf")
                            .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                            .arg(qApp->organizationName())
                            .arg(qApp->applicationName());

    removeLockFiles();
    m_backend = new QSettingBackend(strConfigPath);

    settings = DSettings::fromJsonFile(":/resources/settings.json");
    settings->setBackend(m_backend);

    auto fontFamliy = settings->option("base.font.family");
    connect(fontFamliy, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigAdjustFont);

    auto fontSize = settings->option("base.font.size");
    connect(fontSize, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigAdjustFontSize);

    auto wordWrap = settings->option("base.font.wordwrap");
    connect(wordWrap, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigAdjustWordWrap);

    auto showLineNumber = settings->option("base.font.showlinenumber");
    connect(showLineNumber, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigSetLineNumberShow);

    auto bookmark = settings->option("base.font.showbookmark");
    connect(bookmark, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigAdjustBookmark);

    auto codeFlod = settings->option("base.font.codeflod");
    connect(codeFlod, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigShowCodeFlodFlag);

    //添加显示空白符　梁卫东
    auto blankCharacter = settings->option("base.font.showblankcharacter");
    connect(blankCharacter, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigShowBlankCharacter);

    //hightlightcurrentline
    auto hightlightCurrentLine = settings->option("base.font.hightlightcurrentline");
    connect(hightlightCurrentLine, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigHightLightCurrentLine);

    /* 设置页面主题变更信息监听，当前暂时无用，暂且做屏蔽处理
    auto theme = settings->option("advance.editor.theme");
    connect(theme, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        //emit themeChanged(value.toString());
    }); */

    auto tabSpaceNumber = settings->option("advance.editor.tabspacenumber");
    connect(tabSpaceNumber, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotsigAdjustTabSpaceNumber);

    auto keymap = settings->option("shortcuts.keymap.keymap");
    QMap<QString, QVariant> keymapMap;
    keymapMap.insert("keys", QStringList() << "standard" << "emacs" << "customize");
    keymapMap.insert("values", QStringList() << tr("Standard") << "Emacs" << tr("Customize"));
    keymap->setData("items", keymapMap);
    connect(keymap, &Dtk::Core::DSettingsOption::valueChanged, this, &Settings::slotupdateAllKeysWithKeymap);

    //only used by new window
    auto windowState = settings->option("advance.window.windowstate");
    #if 0
    connect(windowState, &Dtk::Core::DSettingsOption::valueChanged, this, [=] (QVariant value) {
        emit sigChangeWindowSize(value.toString());
    });
    #endif
    QMap<QString, QVariant> windowStateMap;
    windowStateMap.insert("keys", QStringList() << "window_normal" << "window_maximum" << "fullscreen");
    windowStateMap.insert("values", QStringList() << tr("Normal") << tr("Maximum") << tr("Fullscreen"));
    windowState->setData("items", windowStateMap);

    connect(settings, &Dtk::Core::DSettings::valueChanged, this, &Settings::slotCustomshortcut);
}

void Settings::setSavePath(int id,const QString& path)
{
    switch (id) {
    case PathSettingWgt::LastOptBox:{
        auto opt = settings->option("advance.open_save_setting.open_save_lastopt_path");
        opt->setValue(path);
        break;
    }
    case PathSettingWgt::CurFileBox:{
        auto opt = settings->option("advance.open_save_setting.open_save_curfile_path");
        opt->setValue(path);
        break;
    }
    case PathSettingWgt::CustomBox:{
        auto opt = settings->option("advance.open_save_setting.open_save_custom_path");
        opt->setValue(path);
        break;
    }
    default:
        break;
    }
}


QString Settings::getSavePath(int id)
{
    QString path;
    switch (id) {
    case PathSettingWgt::LastOptBox:{
        path = settings->option("advance.open_save_setting.open_save_lastopt_path")->value().toString();
        break;
    }
    case PathSettingWgt::CurFileBox:{
        path = settings->option("advance.open_save_setting.open_save_curfile_path")->value().toString();
        break;
    }
    case PathSettingWgt::CustomBox:{
        path = settings->option("advance.open_save_setting.open_save_custom_path")->value().toString();
        break;
    }
    default:
        break;
    }

    return path;
}

void Settings::setSavePathId(int id)
{
    auto opt = settings->option("advance.open_save_setting.savingpathwgt");
    opt->setValue(QString::number(id));
}
int Settings::getSavePathId()
{
   return settings->option("advance.open_save_setting.savingpathwgt")->value().toInt();

}
Settings::~Settings()
{
    if (m_backend != nullptr) {
        delete m_backend;
        m_backend = nullptr;
    }
}

void Settings::setSettingDialog(DSettingsDialog *settingsDialog)
{
    m_pSettingsDialog = settingsDialog;
}

// This function is workaround, it will remove after DTK fixed SettingDialog theme bug.
void Settings::dtkThemeWorkaround(QWidget *parent, const QString &theme)
{
    parent->setStyle(QStyleFactory::create(theme));

    for (auto obj : parent->children()) {
        auto w = qobject_cast<QWidget *>(obj);
        if (!w) {
            continue;
        }

        dtkThemeWorkaround(w, theme);
    }
}

QPair<QWidget *, QWidget *> Settings::createFontComBoBoxHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QComboBox *comboBox = new QComboBox;
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, comboBox);

    QFontDatabase fontDatabase;
    comboBox->addItems(fontDatabase.families());
    // 设置最小宽度，以保持和下方 OptionDSpinBox 一样的长度
    comboBox->setMinimumSize(240, 36);

    if (option->value().toString().isEmpty()) {
        option->setValue(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    }

    // init.
    comboBox->setCurrentText(option->value().toString());

    connect(option, &DSettingsOption::valueChanged, comboBox, [ = ](QVariant var) {
        comboBox->setCurrentText(var.toString());
    });

    option->connect(comboBox, &QComboBox::currentTextChanged, option, [ = ](const QString & text) {
        option->setValue(text);
    });

#ifdef DTKWIDGET_CLASS_DSizeMode
    // 适配紧凑模式，不同布局下调整高度
    const int defaultHeight = 36;
    const int compactHeight = 24;
    comboBox->setFixedHeight(DGuiApplicationHelper::isCompactMode() ? compactHeight : defaultHeight);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, comboBox, [ = ](){
        comboBox->setFixedHeight(DGuiApplicationHelper::isCompactMode() ? compactHeight : defaultHeight);
    });
#endif

    return optionWidget;
}

QWidget* Settings::createSavingPathWgt(QObject* obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QString name = option->name();
    PathSettingWgt* pathwgt = new PathSettingWgt;
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, pathwgt);

    connect(option, &DSettingsOption::valueChanged, pathwgt, [ = ](QVariant var) {
        int id = var.toInt();
        pathwgt->onSaveIdChanged(id);
    });

    auto custompath = s_pSetting->settings->option("advance.open_save_setting.open_save_custom_path");
    connect(custompath, &Dtk::Core::DSettingsOption::valueChanged, [=](QVariant var){
        //pathwgt->setEditText(var.toString());
    });

    return optionWidget.second;
}

QPair<QWidget *, QWidget *> Settings::createKeySequenceEditHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    KeySequenceEdit *shortCutLineEdit = new KeySequenceEdit(option);

    shortCutLineEdit->ShortcutDirection(Qt::AlignLeft);
    shortCutLineEdit->setFocusPolicy(Qt::StrongFocus);

    // init.
    shortCutLineEdit->setKeySequence(QKeySequence(option->value().toString()));
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, shortCutLineEdit);

    option->connect(shortCutLineEdit, &DKeySequenceEdit::editingFinished, [ = ](const QKeySequence & sequence) {
        QString checkName = option->key();
        QString reason;
        bool bIsConflicts = false;
        auto keymap = instance()->settings->option("shortcuts.keymap.keymap");
        QStringList keySplitList = option->key().split(".");
        keySplitList[1] = QString("%1_keymap_%2").arg(keySplitList[1]).arg(keymap->value().toString());

        // 获取默认设置的组合键序列
        QString defaultKey;
        auto defaultOption = instance()->settings->option(keySplitList.join("."));
        if (defaultOption) {
            defaultKey = defaultOption->defaultValue().toString();
        }

        // 判断新设置的组合键序列是否允许
        if (!instance()->checkShortcutValid(checkName, sequence.toString(), reason, bIsConflicts, defaultKey)) {
            instance()->m_pDialog = instance()->createDialog(reason, "", bIsConflicts);
            instance()->m_pDialog->exec();
            // 恢复组合键序列
            shortCutLineEdit->setKeySequence(instance()->settings->value(keySplitList.join(".")).toString());
            keymap->setValue("emacs");
            keymap->setValue("customize");
            return;
        }

        bool bIsCustomize = false;
        QString conflictsKeys;
        QString originalKeys;

        if (keymap->value().toString() != "customize") {
            instance()->m_bUserChangeKey = true;

            for (auto loopOption : instance()->settings->group("shortcuts.window_keymap_customize")->options()) {
                QStringList loopKeySplitList = loopOption->key().split(".");
                loopKeySplitList[1] = QString("window_keymap_%1").arg(keymap->value().toString());

                if (loopOption->value().toString() == sequence.toString()) {

                    if (checkName.contains(loopKeySplitList.last())) {
                        keymap->setValue("customize");
                        //return;
                    } else {
                        bIsConflicts = true;
                        loopKeySplitList[1] = QString("window_keymap_%1").arg("customize");
                        conflictsKeys = loopKeySplitList.join(".");
                    }
                }
            }

            for (auto loopOption : instance()->settings->group("shortcuts.editor_keymap_customize")->options()) {
                QStringList loopKeySplitList = loopOption->key().split(".");
                loopKeySplitList[1] = QString("editor_keymap_%1").arg(keymap->value().toString());

                if (loopOption->value().toString() == sequence.toString()) {

                    if (checkName.contains(loopKeySplitList.last())) {
                        keymap->setValue("customize");
                        //return;
                    } else {
                        bIsConflicts = true;
                        loopKeySplitList[1] = QString("editor_keymap_%1").arg("customize");
                        conflictsKeys = loopKeySplitList.join(".");
                    }
                }
            }

            instance()->m_bUserChangeKey = false;
        }  else {
            bIsCustomize = true;
            instance()->m_bUserChangeKey = true;
            for (auto loopOption : instance()->settings->group("shortcuts.window_keymap_customize")->options()) {
                QStringList loopKeySplitList = loopOption->key().split(".");
                loopKeySplitList[1] = QString("window_keymap_%1").arg(keymap->value().toString());

                if (loopOption->value().toString() == sequence.toString()) {

                    if (checkName.contains(loopKeySplitList.last())) {
                        //return;
                    } else {
                        bIsConflicts = true;
                        conflictsKeys = loopKeySplitList.join(".");
                    }
                }
            }

            for (auto loopOption : instance()->settings->group("shortcuts.editor_keymap_customize")->options()) {
                QStringList loopKeySplitList = loopOption->key().split(".");
                loopKeySplitList[1] = QString("editor_keymap_%1").arg(keymap->value().toString());

                if (loopOption->value().toString() == sequence.toString()) {

                    if (checkName.contains(loopKeySplitList.last())) {
                        //return;
                    } else {
                        bIsConflicts = true;
                        conflictsKeys = loopKeySplitList.join(".");
                    }
                }
            }
            instance()->m_bUserChangeKey = false;
        }

        keySplitList = option->key().split(".");
        keySplitList[1] = QString("%1_keymap_%2").arg(keySplitList[1]).arg(keymap->value().toString());
        QString qstrSequence = sequence.toString();

        if (sequence.toString().contains("<")) {
            qstrSequence.replace(qstrSequence.indexOf("<"), 1, "&lt;");
        }

        if (sequence.toString().contains("Return")) {
            qstrSequence.replace(qstrSequence.indexOf("Return"), 6, "Enter");
        }

        QString style = QString("<span style=\"color: rgba(255, 87, 54, 1);\">[%1]</span>").arg(qstrSequence);

        if (bIsConflicts || sequence.toString() == "Alt+M") {
            if (sequence.toString() == "Alt+M") {
                instance()->m_pDialog = instance()->createDialog(tr("This shortcut conflicts with system shortcut %1").arg(style), "", bIsConflicts);
            } else {
                instance()->m_pDialog = instance()->createDialog(tr("This shortcut conflicts with %1, click on Replace to make this shortcut effective immediately").arg(style), "", bIsConflicts);
            }

            int mode = instance()->m_pDialog->exec();

            // click cancel button.
            if (mode == -1 || mode == 0) {
                shortCutLineEdit->setKeySequence(QKeySequence(instance()->settings->value(keySplitList.join(".")).toString()));
                keymap->setValue("emacs");
                keymap->setValue("customize");
                return;
            } else {
                keySplitList = option->key().split(".");
                keySplitList[1] = QString("%1_keymap_customize").arg(keySplitList[1]);

                if (!bIsCustomize) {
                    instance()->settings->option(keySplitList.join("."))->setValue(sequence.toString());
                    instance()->settings->option(conflictsKeys)->setValue("");
                    shortCutLineEdit->setKeySequence(QKeySequence(instance()->settings->value(checkName).toString()));
                } else {
                    instance()->settings->option(keySplitList.join("."))->setValue(sequence.toString());
                    instance()->settings->option(conflictsKeys)->setValue("");
                }
                keymap->setValue("emacs");
                keymap->setValue("customize");
                return;
            }
        }

        if (!bIsCustomize) {
            keySplitList = option->key().split(".");
            keySplitList[1] = QString("%1_keymap_customize").arg(keySplitList[1]);
            instance()->settings->option(keySplitList.join("."))->setValue(sequence.toString());
        } else {
            instance()->settings->option(keySplitList.join("."))->setValue(sequence.toString());
        }
        keymap->setValue("emacs");
        keymap->setValue("customize");
    });

    // 配置修改
    option->connect(option, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged, shortCutLineEdit, &KeySequenceEdit::slotDSettingsOptionvalueChanged);
    return optionWidget;
}

void KeySequenceEdit::slotDSettingsOptionvalueChanged(const QVariant & value)
{
    QString keyseq = value.toString();

    if (keyseq.isEmpty()) {
        this->clear();
        return;
    }
    this->setKeySequence(QKeySequence(keyseq));
}

bool KeySequenceEdit::eventFilter(QObject *object, QEvent *event)
{
    //设置界面　回车键和空格键　切换输入 梁卫东　２０２０－０８－２１　１６：２８：３１
    if (object == this) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

           //判断是否包含组合键　梁卫东　２０２０－０９－０２　１５：０３：５６
            Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
            bool bHasModifier = (modifiers & Qt::ShiftModifier || modifiers & Qt::ControlModifier ||
                                 modifiers & Qt::AltModifier);

            if (!bHasModifier && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Space)) {
                QRect rect = this->rect();
                QList<QLabel*> childern = findChildren<QLabel*>();

                for (int i =0; i < childern.size(); i++) {
                    QPoint pos(25,rect.height()/2);

                    QMouseEvent event0(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                    DApplication::sendEvent(childern[i], &event0);
                }

                return true;
            }
        }
    }

    return DKeySequenceEdit::eventFilter(object, event);
}
Settings *Settings::instance()
{
    if (s_pSetting == nullptr) {
        s_pSetting = new Settings;
    }
    return s_pSetting;
}

void Settings::updateAllKeysWithKeymap(QString keymap)
{
    m_bUserChangeKey = true;

    for (auto option : settings->group("shortcuts.window")->options()) {
        QStringList keySplitList = option->key().split(".");
        keySplitList[1] = QString("%1_keymap_%2").arg(keySplitList[1]).arg(keymap);
        auto opt = settings->option(keySplitList.join("."));
        if (opt) {
            option->setValue(opt->value().toString());
        } else {
            qWarning() << "Unknown shortcut key:" << keySplitList.join(".");
        }
    }

    for (auto option : settings->group("shortcuts.editor")->options()) {
        QStringList keySplitList = option->key().split(".");
        keySplitList[1] = QString("%1_keymap_%2").arg(keySplitList[1]).arg(keymap);
        auto opt = settings->option(keySplitList.join("."));
        if (opt) {
            option->setValue(opt->value().toString());
        } else {
            qWarning() << "Unknown shortcut key:" << keySplitList.join(".");
        }
    }

    m_bUserChangeKey = false;
}

void Settings::copyCustomizeKeysFromKeymap(QString keymap)
{
    m_bUserChangeKey = true;

    for (auto option : settings->group("shortcuts.window_keymap_customize")->options()) {
        QStringList keySplitList = option->key().split(".");
        keySplitList[1] = QString("window_keymap_%1").arg(keymap);
        option->setValue(settings->option(keySplitList.join("."))->value().toString());
    }

    for (auto option : settings->group("shortcuts.editor_keymap_customize")->options()) {
        QStringList keySplitList = option->key().split(".");
        keySplitList[1] = QString("editor_keymap_%1").arg(keymap);
        option->setValue(settings->option(keySplitList.join("."))->value().toString());
    }

    m_bUserChangeKey = false;
}

bool Settings::checkShortcutValid(const QString &Name, QString Key, QString &Reason, bool &bIsConflicts, QString defaultValue)
{
    Q_UNUSED(Name);

    if (Key.contains("<")) {
        Key.replace(Key.indexOf("<"), 1, "&lt;");
    }

    QString style = QString("<span style=\"color: rgba(255, 87, 54, 1);\">[%1]</span>").arg(Key);
    // 单键
    if (Key.count("+") == 0) {
        //F1-F12是允许的，这个正则不够精确，但是没关系。
        QRegExp regexp("^F[0-9]{1,2}$");
        if (!Key.contains(regexp)) {
            Reason = tr("The shortcut %1 is invalid, please set another one.").arg(style);
            bIsConflicts = false;
            return  false;
        }
    }
    // 小键盘单键都不允许
    QRegExp regexpNum("^Num+.*");
    if (Key.contains(regexpNum)) {
        Reason = tr("The shortcut %1 is invalid, please set another one.").arg(style);
        bIsConflicts = false;
        return  false;
    }

    // 屏蔽Shift组合键处理，但允许恢复为默认组合键序列
    if (Key.contains("Shift")
            && Key != defaultValue) {
        Reason = tr("The shortcut %1 is invalid, please set another one.").arg(style);
        bIsConflicts = false;
        return false;
    }

    return true;
}

bool Settings::isShortcutConflict(const QString &Name, const QString &Key)
{
    for (QString tmpKey : settings->keys()) {
        if (settings->value(tmpKey).toString() == Key/* && tmpKey.contains("customize")*/) {
            if (Name != tmpKey) {
                return  true;
            }
        }
    }
    return  false;
}

DDialog *Settings::createDialog(const QString &title, const QString &content, const bool &bIsConflicts)
{
    DDialog *dialog = new DDialog(title, content, m_pSettingsDialog);
    dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowStaysOnBottomHint);
    dialog->setIcon(QIcon::fromTheme("deepin-editor"));

    if (bIsConflicts) {
        dialog->addButton(QString(tr("Cancel")), true, DDialog::ButtonNormal);
        dialog->addButton(QString(tr("Replace")), false, DDialog::ButtonRecommend);
    } else {
        dialog->addButton(QString(tr("OK")), true, DDialog::ButtonRecommend);
    }

    return dialog;
}

//删除config.conf配置文件目录下的.lock文件和.rmlock文件
void Settings::removeLockFiles()
{
    QString configPath = QString("%1/%2/%3")
            .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
            .arg(qApp->organizationName())
            .arg(qApp->applicationName());

    QDir dir(configPath);
    if (!dir.exists()) {
        return;
    }

    dir.setFilter(QDir::Files);
    QStringList nameList = dir.entryList();
    for (auto name: nameList) {
        if (name.contains(".lock") || name.contains(".rmlock")) {
            QFile file(name);
            file.remove();
        }
    }
}

void Settings::slotCustomshortcut(const QString &key, const QVariant &value)
{
    auto keymap = settings->option("shortcuts.keymap.keymap");
    if (!m_bUserChangeKey && key.startsWith("shortcuts.") && key != "shortcuts.keymap.keymap" && !key.contains("_keymap_")) {
        m_bUserChangeKey = true;

        QString currentKeymap = settings->option("shortcuts.keymap.keymap")->value().toString();

        QStringList keySplitList = key.split(".");
        keySplitList[1] = QString("%1_keymap_customize").arg(keySplitList[1]);
        QString customizeKey = keySplitList.join(".");

        // Just update customize key user input, don't change keymap.
        if (currentKeymap == "customize") {
            settings->option(customizeKey)->setValue(value);
        }
        // If current kemap is not "customize".
        // Copy all customize keys from current keymap, and then update customize key just user input.
        // Then change keymap name.
        else {
            copyCustomizeKeysFromKeymap(currentKeymap);
            settings->option(customizeKey)->setValue(value);
            keymap->setValue("customize");
        }
        m_bUserChangeKey = false;
    }
}


void Settings::slotsigAdjustFont(QVariant value)
{
    emit sigAdjustFont(value.toString());
}

void Settings::slotsigAdjustFontSize(QVariant value)
{
    emit sigAdjustFontSize(value.toReal());
}

void Settings::slotsigAdjustWordWrap(QVariant value)
{
    emit sigAdjustWordWrap(value.toBool());
}

void Settings::slotsigSetLineNumberShow(QVariant value)
{
    emit sigSetLineNumberShow(value.toBool());
}

void Settings::slotsigAdjustBookmark(QVariant value)
{
    emit sigAdjustBookmark(value.toBool());
}

void Settings::slotsigShowCodeFlodFlag(QVariant value)
{
    emit sigShowCodeFlodFlag(value.toBool());
}

void Settings::slotsigShowBlankCharacter(QVariant value)
{
    emit sigShowBlankCharacter(value.toBool());
}

void Settings::slotsigHightLightCurrentLine(QVariant value)
{
    emit sigHightLightCurrentLine(value.toBool());
}

void Settings::slotsigAdjustTabSpaceNumber(QVariant value)
{
    emit sigAdjustTabSpaceNumber(value.toInt());
}

void Settings::slotupdateAllKeysWithKeymap(QVariant value)
{
    updateAllKeysWithKeymap(value.toString());
}

KeySequenceEdit::KeySequenceEdit(DTK_CORE_NAMESPACE::DSettingsOption *opt, QWidget *parent)
    : DKeySequenceEdit(parent)
{
    m_pOption = opt;
    this->installEventFilter(this);
}

DSettingsOption *KeySequenceEdit::option()
{
    return m_pOption;
}

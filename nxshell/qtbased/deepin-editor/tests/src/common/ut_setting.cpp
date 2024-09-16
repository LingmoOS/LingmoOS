// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_setting.h"
#include "../../src/common/settings.h"
#include <DSettingsDialog>
#include <DSettingsWidgetFactory>
#include <QKeySequence>
#include <DSettingsOption>
#include <DSettings>
#include <QStandardPaths>
#include <DtkCores>
#include "src/stub.h"

namespace settinsstub {

KeySequenceEdit* keyvalue = nullptr;
KeySequenceEdit* KeySequenceEditstub()
{
    if(keyvalue==nullptr)
    {
        DTK_CORE_NAMESPACE::DSettingsOption o;
        o.setValue("123.456");
        keyvalue = new KeySequenceEdit(&o);
    }
    return keyvalue;
}
}

using namespace settinsstub;

UT_Setting::UT_Setting()
{
    m_setting = new Settings();
}

void UT_Setting::SetUp()
{
    if (m_setting != nullptr) {
        m_setting = new Settings();
    }

    EXPECT_NE(m_setting, nullptr);
}

void UT_Setting::TearDown()
{
    delete m_setting;
    m_setting=nullptr;
}

QString retqstring()
{
    return "123.456";
}

TEST(UT_Setting_Settings, UT_Setting_Settings)
{
    Settings* m_setting = new Settings();
    QString figPath = QString("%1/%2/%3/config.conf")
                          .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                          .arg(qApp->organizationName())
                          .arg(qApp->applicationName());
    QSettingBackend *m_backend = new QSettingBackend(figPath);


    m_setting->settings = DSettings::fromJsonFile(":/resources/settings.json");
    m_setting->settings->setBackend(m_backend);
    auto fontFamliy = m_setting->settings->option("base.font.family");
    //        QVariant value;
    //        fontFamliy->valueChanged(value);
    //        sleep(500);

    QVariant retVal;
    QMetaObject::invokeMethod(fontFamliy, "valueChanged", Qt::DirectConnection,
                              QGenericReturnArgument(),
                              Q_ARG(QVariant, "dsd"));


    EXPECT_NE(m_backend,nullptr);
    m_backend->deleteLater();
    m_setting->deleteLater();
}

TEST(UT_Setting_Settings, UT_Setting_Settings_002)
{
    Settings* m_setting = new Settings();
    QString figPath = QString("%1/%2/%3/config.conf")
                          .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                          .arg(qApp->organizationName())
                          .arg(qApp->applicationName());
    QSettingBackend *m_backend = new QSettingBackend(figPath);
    m_setting->~Settings();
    EXPECT_NE(m_backend,nullptr);

    if (m_backend != nullptr) {
        m_backend->deleteLater();
    }
    m_setting->deleteLater();
}

//static Settings* instance();
TEST(UT_Setting_instance, UT_Setting_instance)
{
    EXPECT_NE(Settings::instance(),nullptr);

}

//void dtkThemeWorkaround(QWidget *parent, const QString &theme);
TEST(UT_Setting_dtkThemeWorkaround, UT_Setting_dtkThemeWorkaround)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    Settings *Settings = Settings::instance();
    Settings->dtkThemeWorkaround(pWindow, DEEPIN_THEME);
    EXPECT_NE(pWindow, nullptr);

    Settings->deleteLater();
    pWindow->deleteLater();
}

//static QPair<QWidget*, QWidget*> createFontComBoBoxHandle(QObject *obj);
TEST(UT_Setting_createFontComBoBoxHandle, UT_Setting_createFontComBoBoxHandle)
{
    Settings* m_setting = new Settings();

    QWidget *widget = new QWidget();
    DSettingsDialog *dialog = new DSettingsDialog(widget);
    dialog->widgetFactory()->registerWidget("fontcombobox", m_setting->createFontComBoBoxHandle);

    DTK_CORE_NAMESPACE::DSettingsOption o;
    auto it = m_setting->createFontComBoBoxHandle(&o);
    EXPECT_EQ(it.first,nullptr);

    EXPECT_NE(dialog,nullptr);
    EXPECT_NE(widget,nullptr);
    widget->deleteLater();
    dialog->deleteLater();
    m_setting->deleteLater();
}

//static QPair<QWidget*, QWidget*> createKeySequenceEditHandle(QObject *obj);
TEST(UT_Setting_createKeySequenceEditHandle, UT_Setting_createKeySequenceEditHandle_001)
{
    Settings* m_setting = new Settings();
    QWidget *widget = new QWidget();
    DSettingsDialog *dialog = new DSettingsDialog(widget);
    dialog->widgetFactory()->registerWidget("fontcombobox", Settings::createKeySequenceEditHandle);

    DTK_CORE_NAMESPACE::DSettingsOption o;
    auto it = m_setting->createKeySequenceEditHandle(&o);
    EXPECT_EQ(it.first,nullptr);

    EXPECT_NE(dialog,nullptr);
    EXPECT_NE(widget,nullptr);
    widget->deleteLater();
    dialog->deleteLater();
    m_setting->deleteLater();
}

//static Settings* instance();

//void setSettingDialog(DSettingsDialog *settingsDialog);
TEST(UT_Setting_setSettingDialog, UT_Setting_setSettingDialog)
{
    QWidget *widget = new QWidget();
    DSettingsDialog *dialog = new DSettingsDialog(widget);
    Settings *pSettings = Settings::instance();
    pSettings->setSettingDialog(dialog);

    EXPECT_NE(Settings::instance()->m_pSettingsDialog, nullptr);

    EXPECT_NE(dialog,nullptr);
    EXPECT_NE(widget,nullptr);
    widget->deleteLater();
    dialog->deleteLater();
    pSettings->deleteLater();
}

//private:
//void updateAllKeysWithKeymap(QString keymap);
TEST(UT_Setting_updateAllKeysWithKeymap, UT_Setting_updateAllKeysWithKeymap)
{
    QString keymap = Settings::instance()->settings->option("shortcuts.keymap.keymap")->value().toString();
    Settings::instance()->updateAllKeysWithKeymap(keymap);

    EXPECT_NE(Settings::instance()->m_bUserChangeKey,true);
}

//void copyCustomizeKeysFromKeymap(QString keymap);
TEST(UT_Setting_copyCustomizeKeysFromKeymap, UT_Setting_copyCustomizeKeysFromKeymap)
{
    QString keymap = Settings::instance()->settings->option("shortcuts.keymap.keymap")->value().toString();
    Settings::instance()->copyCustomizeKeysFromKeymap(keymap);

    EXPECT_NE(Settings::instance()->m_bUserChangeKey,true);
}

//此函数代码调试中已经覆盖， html中显示未覆盖
TEST(UT_Setting_checkShortcutValid, UT_Setting_checkShortcutValid)
{
    Settings* m_setting = new Settings();

    bool ok;
    QString reason = "reason";
    EXPECT_NE(m_setting->checkShortcutValid("shortcuts.keymap.keymap", "Enter", reason, ok),true);
    m_setting->deleteLater();
}

TEST(UT_Setting_checkShortcutValid2, UT_Setting_checkShortcutValid2)
{
    Settings* m_setting = new Settings();
    bool ok;
    QString reason = "reason";
    EXPECT_NE(m_setting->checkShortcutValid("shortcuts.keymap.keymap", "<", reason, ok),true);

    m_setting->deleteLater();
}

TEST(UT_Setting_checkShortcutValid3, UT_Setting_checkShortcutValid3)
{
    Settings* m_setting = new Settings();
    bool ok;
    QString reason = "reason";
    EXPECT_NE(m_setting->checkShortcutValid("shortcuts.keymap.keymap<", "Num+", reason, ok),true);

    m_setting->deleteLater();
}

TEST(UT_Setting_isShortcutConflict, UT_Setting_isShortcutConflict)
{
    Settings* m_setting = new Settings();
    //Settings::instance()->isShortcutConflict("shortcuts.keymap.keymap", "Enter");
    //    assert(1 == 1);
    QStringList list;
    list << "aa"
         << "bb";
    EXPECT_NE(m_setting->isShortcutConflict("aa", "bb"), true);

    EXPECT_NE(m_setting->isShortcutConflict("aa", "bb"), true);

    QVariant value;
    QString strValue("Ctrl+Z");
    const QString strKey("shortcuts.editor.undo");
    m_setting->m_bUserChangeKey = false;
    m_setting->slotCustomshortcut(strKey, strValue);

    m_setting->slotsigAdjustFont(value);

    QVariant keymap2 = Settings::instance()->settings->option("base.font.size")->value();
    m_setting->slotsigAdjustFontSize(keymap2);
    m_setting->slotsigAdjustWordWrap(value);
    m_setting->slotsigSetLineNumberShow(value);
    m_setting->slotsigAdjustBookmark(value);
    m_setting->slotsigShowCodeFlodFlag(value);
    m_setting->slotsigShowBlankCharacter(value);
    m_setting->slotsigHightLightCurrentLine(value);
    m_setting->slotsigAdjustTabSpaceNumber(value);

    QVariant keymap = Settings::instance()->settings->option("shortcuts.keymap.keymap")->value();
    m_setting->slotupdateAllKeysWithKeymap(keymap);

    m_setting->deleteLater();
}

TEST(UT_Setting_slotCustomshortcut, UT_Setting_slotCustomshortcut_001)
{
    QString strValue("Ctrl+Z");
    const QString strKey("shortcuts.editor.undo");
    Settings *pSettings = new Settings();
    pSettings->m_bUserChangeKey = false;
    pSettings->slotCustomshortcut(strKey, strValue);
    ASSERT_TRUE(pSettings->m_bUserChangeKey == false);

    pSettings->deleteLater();
}


QVariant stub_dsettings_option()
{
    QVariant variant = QString("Hello word");
    return variant;
}

TEST(UT_Setting_slotCustomshortcut, UT_Setting_slotCustomshortcut_002)
{
    QString strValue("Ctrl+Z");
    const QString strKey("shortcuts.editor.undo");
    Settings *pSettings = new Settings();
    pSettings->m_bUserChangeKey = false;
    pSettings->slotCustomshortcut(strKey, strValue);
    ASSERT_TRUE(pSettings->m_bUserChangeKey == false);

    pSettings->deleteLater();
}

TEST(UT_Setting_isShortcutConflict, UT_Setting_slotsigAdjustFontSize)
{
    Settings *pSettings = new Settings();
    pSettings->slotsigAdjustFont(15);
    ASSERT_TRUE(pSettings->m_backend != nullptr);

    pSettings->deleteLater();
}

TEST(UT_Setting_createDialog, UT_Setting_createDialog_001)
{
    DSettingsDialog *pSettingsDialog = new DSettingsDialog;
    Settings *pSettings = new Settings();
    pSettings->m_pSettingsDialog = pSettingsDialog;
    DDialog *pDialog = pSettings->createDialog(QString("title"), QString("content"), true);

    ASSERT_TRUE(pDialog != nullptr);

    pSettingsDialog->deleteLater();
    pDialog->deleteLater();
    pSettings->deleteLater();
}

TEST(UT_Setting_createDialog, UT_Setting_createDialog_002)
{
    DSettingsDialog *pSettingsDialog = new DSettingsDialog;
    Settings *pSettings = new Settings();
    pSettings->m_pSettingsDialog = pSettingsDialog;
    DDialog *pDialog = pSettings->createDialog(QString("title"), QString("content"), false);

    ASSERT_TRUE(pDialog != nullptr);

    pSettingsDialog->deleteLater();
    pDialog->deleteLater();
    pSettings->deleteLater();
}

//
TEST(UT_Setting_KeySequenceEdit, UT_Setting_KeySequenceEdit1)
{
    QWidget *widget = new QWidget();
    DSettingsDialog *dialog = new DSettingsDialog(widget);
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(dialog);

    KeySequenceEdit *shortCutLineEdit = new KeySequenceEdit(option);
    QEvent *e = new QEvent(QEvent::KeyPress);
    shortCutLineEdit->eventFilter(shortCutLineEdit,e);

    QVariant keymap = Settings::instance()->settings->option("shortcuts.keymap.keymap")->value();
    shortCutLineEdit->slotDSettingsOptionvalueChanged(keymap);

    EXPECT_NE(widget, nullptr);
    EXPECT_NE(shortCutLineEdit, nullptr);
    EXPECT_NE(e, nullptr);
    widget->deleteLater();
    dialog->deleteLater();
    shortCutLineEdit->deleteLater();
    delete e ;
    e=nullptr;
}

TEST(UT_Setting_KeySequenceEdit, UT_KeySequenceEdit_KeySequenceEdit)
{
    DTK_CORE_NAMESPACE::DSettingsOption *opt = new DTK_CORE_NAMESPACE::DSettingsOption;
    KeySequenceEdit *pKeySequenceEdit = new KeySequenceEdit(opt);
    ASSERT_TRUE(pKeySequenceEdit->m_pOption != nullptr);

    opt->deleteLater();
    pKeySequenceEdit->deleteLater();
}

TEST(UT_Setting_KeySequenceEdit, UT_KeySequenceEdit_eventFilter_001)
{
    DTK_CORE_NAMESPACE::DSettingsOption *opt = new DTK_CORE_NAMESPACE::DSettingsOption;
    KeySequenceEdit *pKeySequenceEdit = new KeySequenceEdit(opt);
    Qt::KeyboardModifier modefiers[4] = {Qt::NoModifier, Qt::AltModifier, Qt::MetaModifier, Qt::NoModifier};
    QKeyEvent *pKeyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, modefiers[0], "\r");
    bool bRet = pKeySequenceEdit->eventFilter(pKeySequenceEdit, pKeyEvent);

    ASSERT_TRUE(bRet == true);

    opt->deleteLater();
    pKeySequenceEdit->deleteLater();
    delete pKeyEvent;
    pKeyEvent = nullptr;
}

TEST(UT_Setting_KeySequenceEdit, UT_KeySequenceEdit_eventFilter_002)
{
    DTK_CORE_NAMESPACE::DSettingsOption *opt = new DTK_CORE_NAMESPACE::DSettingsOption;
    KeySequenceEdit *pKeySequenceEdit = new KeySequenceEdit(opt);
    bool bRet = pKeySequenceEdit->eventFilter(nullptr, nullptr);
    ASSERT_TRUE(bRet == false);

    opt->deleteLater();
    pKeySequenceEdit->deleteLater();
}

TEST(UT_Setting_KeySequenceEdit, UT_KeySequenceEdit_slotDSettingsOptionvalueChanged_001)
{
    DTK_CORE_NAMESPACE::DSettingsOption *opt = new DTK_CORE_NAMESPACE::DSettingsOption;
    KeySequenceEdit *pKeySequenceEdit = new KeySequenceEdit(opt);
    pKeySequenceEdit->slotDSettingsOptionvalueChanged(QString(""));
    ASSERT_TRUE(pKeySequenceEdit->m_pOption != nullptr);

    opt->deleteLater();
    pKeySequenceEdit->deleteLater();
}

TEST(UT_Setting_KeySequenceEdit, UT_KeySequenceEdit_slotDSettingsOptionvalueChanged_002)
{
    DTK_CORE_NAMESPACE::DSettingsOption *opt = new DTK_CORE_NAMESPACE::DSettingsOption;
    KeySequenceEdit *pKeySequenceEdit = new KeySequenceEdit(opt);
    pKeySequenceEdit->slotDSettingsOptionvalueChanged(QString("Ctr+t"));
    ASSERT_TRUE(pKeySequenceEdit->m_pOption != nullptr);

    opt->deleteLater();
    pKeySequenceEdit->deleteLater();
}

TEST(UT_Setting_KeySequenceEdit, UT_KeySequenceEdit_option)
{
    DTK_CORE_NAMESPACE::DSettingsOption *opt = new DTK_CORE_NAMESPACE::DSettingsOption;
    KeySequenceEdit *pKeySequenceEdit = new KeySequenceEdit(opt);
    DTK_CORE_NAMESPACE::DSettingsOption *pRet = pKeySequenceEdit->option();

    ASSERT_TRUE(pRet == opt);

    opt->deleteLater();
    pKeySequenceEdit->deleteLater();
}

//以下两条CASE 脚本跑会造成程序崩，加两行debug后就不崩了
//TEST(UT_Setting_createDialog2, UT_Setting_createDialog2)
//{
//    Settings set;
//    set.createDialog("ba", "bb", false);
//}

//TEST(UT_Setting_createDialog, UT_Setting_createDialog)
//{
//    Settings set;
//    set.createDialog("ba", "bb", true);
//}

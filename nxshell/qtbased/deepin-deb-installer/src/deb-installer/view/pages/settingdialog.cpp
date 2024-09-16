// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "utils/hierarchicalverify.h"

#include <DSettingsWidgetFactory>
#include <DStandardPaths>
#include <DCheckBox>
#include <DLabel>
#include <DGuiApplicationHelper>

#include <QApplication>
#include <QDir>
#include <qsettingbackend.h>
#include <QDebug>
#include <QTextStream>
#include <QSettings>

// 富文本链接模板 color: #0082FA
const char WEBSITE_LINK_TEMPLATE[] = "<a href='%1' style='text-decoration: none; font-size:%2px; color: %3;'>%4</a>";

/**
   @class ProceedLabel
   @brief 分级管控安全等级设置引导提示控件
 */
class ProceedLabel : public DLabel
{
public:
    explicit ProceedLabel();
    ~ProceedLabel() override = default;

protected:
    bool event(QEvent *e) override;
    void updateText();
};

ProceedLabel::ProceedLabel()
{
    setObjectName("OptionProceedLabel");
    setAccessibleName("OptionProceedLabel");

    setWordWrap(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setOpenExternalLinks(false);

    updateText();

    // 点击链接跳转安全中 > 应用安全
    QObject::connect(
        this, &DLabel::linkActivated, [](const QString &) { HierarchicalVerify::instance()->proceedDefenderSafetyPage(); });
}

/**
   @reimp
 */
bool ProceedLabel::event(QEvent *e)
{
    switch (e->type()) {
        case QEvent::PaletteChange:
            updateText();
            break;
        default:
            break;
    }

    return DLabel::event(e);
}

/**
   @brief 更新提示的文件的颜色，Dark模式下，显示文字可能偏暗，如有必要，可通过如下代码调整亮度凸显效果。
   @code
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        color = color.lighter(150 / color.lightness() * 100);
    }
   @endcode
 */
void ProceedLabel::updateText()
{
    QString proceedNotify =
        QObject::tr("To install unsigned apps, go to Security Center > Tools > App Security, and select the app types that can "
                    "be installed.");
    QString tmp = QObject::tr("Security Center > Tools > App Security");
    QColor color = palette().color(QPalette::Highlight);
    int pxSize = font().pixelSize();

    proceedNotify =
        proceedNotify.replace(tmp, QString(WEBSITE_LINK_TEMPLATE).arg("localtest.com").arg(pxSize).arg(color.name()).arg(tmp));
    setText(proceedNotify);
}

SettingDialog::SettingDialog(QWidget *parent)
    : DSettingsDialog(parent)
{
    init();
}

SettingDialog::~SettingDialog()
{
    delete m_setting;
}

void SettingDialog::init()
{
    if (widgetFactory()) {
        widgetFactory()->registerWidget("proceedLabel", &SettingDialog::createProceedDefenderSafetyLabel);
    }

    m_setting = DSettings::fromJsonFile(":/data/deepin-deb-installer.json");
    setFocus(Qt::PopupFocusReason);
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);  // 换了枚举值，待验证

    const QString confPath = confDir + QDir::separator() + "deepin-deb-installer.conf";
    QDir dir;
    bool isexist = dir.exists(confDir);
    if (!isexist)
        dir.mkpath(confDir);
    // 创建设置项存储后端
    auto backend = new QSettingBackend(confPath, this);
    m_setting->setBackend(backend);
    updateSettings(m_setting);

    QString key = "basic.develop_digital_verify.";
    QString bValue = m_setting->option(key)->value().toString();

    QSettings setting(confPath, QSettings::IniFormat);
    setting.setValue(key + "/" + "value", bValue);
    setting.sync();

    connect(m_setting, &DSettings::valueChanged, this, [=] {
        m_isDigital = m_setting->value("basic.develop_digital_verify.").toBool();
    });
    m_isDigital = m_setting->value("basic.develop_digital_verify.").toBool();

    // 分级管控不同状态下切换显示信息，初始化后绑定信号
    switchHierarchicalNotify(HierarchicalVerify::instance()->isValid());
    connect(HierarchicalVerify::instance(), &HierarchicalVerify::validChanged, this, &SettingDialog::switchHierarchicalNotify);
}

bool SettingDialog::isDigitalVerified()
{
    return m_setting->value("basic.develop_digital_verify.").toBool();
}

/**
   @brief 创建前往安全中心的跳转链接，弹出分级管控安全等级设置引导提示窗口
   @return 创建的跳转提示控件
 */
QWidget *SettingDialog::createProceedDefenderSafetyLabel(QObject *obj)
{
    Q_UNUSED(obj)
    ProceedLabel *label = new ProceedLabel;
    return label;
}

/**
   @brief 处理显示事件，此处用于展示前刷新分级管控接口状态
 */
void SettingDialog::showEvent(QShowEvent *e)
{
    // 刷新分级管控接口状态
    (void)HierarchicalVerify::instance()->isValid();
    DSettingsDialog::showEvent(e);
}

/**
   @brief 跟进 @a valid 当前是否处于分级管控状态切换显示界面
 */
void SettingDialog::switchHierarchicalNotify(bool valid)
{
    setGroupVisible("basic.develop_digital_verify", !valid);
    setResetVisible(!valid);
    setGroupVisible("basic.hierarachical_verify", valid);
}

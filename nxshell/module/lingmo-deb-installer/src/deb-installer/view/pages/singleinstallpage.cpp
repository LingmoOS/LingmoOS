// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleinstallpage.h"
#include "model/deblistmodel.h"
#include "view/widgets/workerprogress.h"
#include "utils/utils.h"
#include "compatible/compatible_backend.h"

#include <QApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QTextLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QFontMetrics>

#include <QApt/DebFile>
#include <QApt/Transaction>

#include <DStyleHelper>
#include <DApplicationHelper>

using QApt::DebFile;
using QApt::Transaction;

DWIDGET_USE_NAMESPACE

SingleInstallPage::SingleInstallPage(AbstractPackageListModel *model, QWidget *parent)
    : QWidget(parent)
    , m_operate(Install)
    , m_workerStarted(false)
    , m_packagesModel(model)
    , m_contentFrame(new QWidget(this))
    , m_itemInfoFrame(new QWidget(this))
    , m_progressFrame(new QWidget(this))
    , m_packageIcon(new DLabel(this))
    , m_packageName(new DebInfoLabel(this))
    , m_packageVersion(new DebInfoLabel(this))
    , m_packageDescription(new DLabel(this))
    , m_tipsLabel(new DebInfoLabel(this))
    , m_progress(new WorkerProgress(this))
    , m_installProcessView(new InstallProcessInfoView(440, 186, this))
    , m_showDependsView(new InstallProcessInfoView(440, 170, this))
    , m_infoControlButton(
          new InfoControlButton(QApplication::translate("SingleInstallPage_Install", "Show details"), tr("Collapse", "button")))
    , m_showDependsButton(new InfoControlButton(QApplication::translate("SingleInstallPage_Install", "Show dependencies"),
                                                tr("Collapse", "button")))
    , m_installButton(new DPushButton(this))
    , m_uninstallButton(new DPushButton(this))
    , m_reinstallButton(new DPushButton(this))
    , m_confirmButton(new DPushButton(this))
    , m_backButton(new DPushButton(this))
    , m_doneButton(new DPushButton(this))
    , m_contentLayout(new QVBoxLayout(m_contentFrame))
    , m_centralLayout(new QVBoxLayout())
    , m_pDSpinner(new DSpinner(this))
    , m_pLoadingLabel(new DCommandLinkButton("", this))
{
    // check if compatible mode before init
    const QModelIndex index = m_packagesModel->index(0);
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
    if (Pkg::CompatibleNotInstalled == dependsStat || Pkg::CompatibleIntalled == dependsStat) {
        m_inCompatibleMode = true;
        const QString rootfsName = index.data(AbstractPackageListModel::CompatibleRootfsRole).toString();
        m_rootfsOsName = CompBackend::instance()->osName(rootfsName);
    }

    initUI();                     // 初始化界面
    initControlAccessibleName();  // 自动化测试
}

void SingleInstallPage::initUI()
{
    QFontInfo fontinfo = this->fontInfo();  // 获取字体信息
    int fontsize = fontinfo.pixelSize();    // 获得字体大小
    initContentLayout();                    // 初始化主布局
    initPkgInfoView(fontsize);              // 初始化包信息布局
    initPkgDependsInfoView();               // 初始化依赖显示布局
    initPkgInstallProcessView(fontsize);    // 初始化包安装过程进度显示布局
    initConnections();                      // 链接信号和槽

    const QModelIndex index = m_packagesModel->index(0);
    Pkg::PackageType type = index.data(AbstractPackageListModel::PackageTypeRole).value<Pkg::PackageType>();
    QIcon icon = Utils::packageIcon(type);
    m_packageIcon->setPixmap(icon.pixmap(m_packageIcon->size()));
    m_itemInfoFrame->setVisible(true);
    m_upDown = true;  // 当前是收缩的

    // update show info, must call before refresh depends
    showPackageInfo();
    connect(m_packagesModel, &AbstractPackageListModel::dataChanged, this, [this]() { showPackageInfo(); });
}

void SingleInstallPage::initControlAccessibleName()
{
    // 获取的包名
    m_packageName->setObjectName("SinglePagePackageName");
    m_packageName->setAccessibleName("SinglePagePackageName");

    // 包的图标
    m_packageIcon->setObjectName("SinglePagePackageIcon");
    m_packageIcon->setAccessibleName("SinglePagePackageIcon");

    // 获取的包的版本
    m_packageVersion->setObjectName("SinglePagePackageVersion");
    m_packageVersion->setAccessibleName("SinglePagePackageVersion");

    // 获取的包的描述
    m_packageDescription->setObjectName("SinglePagePackageDescription");
    m_packageDescription->setAccessibleName("SinglePagePackageDescription");

    // 获取的包的提示信息
    m_tipsLabel->setObjectName("SinglePagePackageStatusTips");
    m_tipsLabel->setAccessibleName("SinglePagePackageStatusTips");

    // 依赖安装的提示信息
    m_pLoadingLabel->setObjectName("SinglePagePackageLoadingTips");
    m_pLoadingLabel->setAccessibleName("SinglePagePackageLoadingTips");
}

void SingleInstallPage::initContentLayout()
{
    m_contentLayout->setSpacing(0);                       // 设置控件边距
    m_contentLayout->setContentsMargins(20, 10, 20, 20);  // 设置四周边距
    m_contentFrame->setLayout(m_contentLayout);           // 设置布局
    m_centralLayout->addWidget(m_contentFrame);

    m_centralLayout->setSpacing(0);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);  // 设置中心布局
    this->setLayout(m_centralLayout);

// #define SHOWBGCOLOR
#ifdef SHOWBGCOLOR
    m_contentFrame->setStyleSheet("QFrame{background: cyan}");
#endif
}

void SingleInstallPage::initInstallWineLoadingLayout()
{
    QVBoxLayout *m_pLoadingLayout = new QVBoxLayout();  // 依赖安装的布局

    m_pDSpinner->setFixedSize(24, 24);  // 设置动画的大小
    m_pDSpinner->setVisible(false);     // 隐藏等待动画
    m_pDSpinner->start();
    m_pLoadingLayout->addWidget(m_pDSpinner);                       // 添加到布局中
    m_pLoadingLayout->setAlignment(m_pDSpinner, Qt::AlignHCenter);  // 居中显示

    // fix bug:33999 The spinner and The Label are too close together add a distence of 4px
    //  使用 margin 而不是 addSpacing ，在设置setVisible(false)时不再占位
    auto spinnerMargin = m_pDSpinner->contentsMargins();
    spinnerMargin.setBottom(5);
    m_pDSpinner->setContentsMargins(spinnerMargin);
    m_pLoadingLabel->setVisible(false);            // 隐藏依赖安装提示信息
    m_pLoadingLabel->setFocusPolicy(Qt::NoFocus);  // 修复会有焦点在依赖加载提示上的问题
    m_pLoadingLayout->setEnabled(true);            // fix bug:33999 Make the DCommandLinkbutton looks like a Lable O_o
    m_pLoadingLayout->addWidget(m_pLoadingLabel);  // 添加提示信息到布局中
    m_pLoadingLayout->setAlignment(m_pLoadingLabel, Qt::AlignHCenter);  // fix bug:33999 keep the label in the middle
    m_pLoadingLabel->setMinimumHeight(24);                              // 设置高度
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_pLoadingLabel, fontFamily, 12, QFont::ExtraLight);

    m_contentLayout->addLayout(m_pLoadingLayout);  // 添加到主布局中
}

void SingleInstallPage::initCompatibleSelectLayout()
{
    if (!m_inCompatibleMode) {
        return;
    }

    m_compatibleLabel = new DLabel(this);
    m_compatibleLabel->setObjectName("SinglePageCompatibleLabel");
    m_compatibleLabel->setText(tr("Select a compatibility mode:"));

    m_compatibleBox = new DComboBox(this);
    m_compatibleBox->setObjectName("SinglePageCompatibleBox");
    m_compatibleBox->setFixedWidth(226);
    auto rootfsList = CompBackend::instance()->rootfsList();
    for (const auto &rootfs : rootfsList) {
        m_compatibleBox->addItem(rootfs->osName, rootfs->name);
    }
    if (!rootfsList.isEmpty()) {
        m_compatibleBox->setCurrentIndex(0);
    }

    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_compatibleLabel, normalFontFamily, 12, QFont::Normal);
    Utils::bindFontBySizeAndWeight(m_compatibleBox, normalFontFamily, 14, QFont::Normal);

    m_compatibleLayout = new QHBoxLayout;
    m_compatibleLayout->setContentsMargins(0, 0, 0, 0);
    m_compatibleLayout->setSpacing(10);
    m_compatibleLayout->addWidget(m_compatibleLabel);
    m_compatibleLayout->addWidget(m_compatibleBox);
    m_compatibleLayout->setAlignment(Qt::AlignHCenter);

    m_contentLayout->addLayout(m_compatibleLayout);
}

void SingleInstallPage::initPkgInfoView(int fontinfosize)
{
    // 根据系统字体的大小设置显示字体的大小
    int fontinfosizetemp = 0;
    int fontinfosizetemp_version = 0;
    if (fontinfosize > 18) {  // 字体大于18说明是大字体，使用较大的字体显示
        fontinfosizetemp = 23;
        fontinfosizetemp_version = 25;
    } else {
        fontinfosizetemp = 20;  // 使用小字体显示
        fontinfosizetemp_version = 20;
    }

    m_packageIcon->setText("icon");       // 图标
    m_packageIcon->setFixedSize(64, 64);  // 设置图标的大小

    DebInfoLabel *packageName = new DebInfoLabel(this);           // 包名的描述性文字
    packageName->setCustomQPalette(QPalette::WindowText);         // 设置包名的字体颜色
    packageName->setMinimumHeight(fontinfosizetemp);              // 根据字体大小设置高度
    packageName->setText(tr("Name: "));                           // 设置具体的提示
    packageName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);  // 垂直居中，水平靠左显示
    packageName->setObjectName("PackageNameTitle");               // 添加ObjectName

    // 设置包名的样式，但是后续再设置显示的内容
    m_packageName->setCustomQPalette(QPalette::WindowText);         // 设置字体样式
    m_packageName->setMinimumHeight(fontinfosizetemp);              // 根据字体大小设置高度
    m_packageName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);  // 垂直靠上，水平靠左

    DebInfoLabel *packageVersion = new DebInfoLabel(this);           // 包版本的描述性文字
    packageVersion->setCustomQPalette(QPalette::WindowText);         // 设置字体颜色
    packageVersion->setMinimumHeight(fontinfosizetemp_version);      // 根据字体大小设置高度
    packageVersion->setText(tr("Version: "));                        // 添加具体的提示文字
    packageVersion->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);  // 垂直居中，水平靠左
    packageVersion->setObjectName("PackageVersionTitle");            // 添加ObjectName

    m_packageVersion->setCustomQPalette(QPalette::WindowText);         // 设置字体样式
    m_packageVersion->setMinimumHeight(fontinfosizetemp_version);      // 根据字体大小设置高度
    m_packageVersion->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);  // 垂直居中，水平靠左

    // 此处可优化
    QVBoxLayout *packageNameVLayout = new QVBoxLayout();  // 包名提示的布局
    packageNameVLayout->setSpacing(0);                    // 设置控件间距
    packageNameVLayout->setContentsMargins(0, 0, 0, 0);   // 设置四周边距
    packageNameVLayout->addSpacing(4);
    packageNameVLayout->addWidget(packageName);  // 添加控件

    QVBoxLayout *pkgNameValueLayout = new QVBoxLayout();  // 实际获取到包名
    pkgNameValueLayout->setSpacing(0);                    // 设置控件间距
    pkgNameValueLayout->setContentsMargins(0, 0, 0, 0);
    pkgNameValueLayout->addSpacing(4);             // 增加空间
    pkgNameValueLayout->addWidget(m_packageName);  // 添加控件

    QHBoxLayout *pkgNameLayout = new QHBoxLayout();  // 包名的控件布局
    pkgNameLayout->setSpacing(0);
    pkgNameLayout->setContentsMargins(0, 0, 0, 0);  // 设置边距
    pkgNameLayout->addSpacing(2);
    pkgNameLayout->addLayout(packageNameVLayout);  // 添加包名提示的label
    pkgNameLayout->addLayout(pkgNameValueLayout);  // 添加实际获取到的包名的label
    pkgNameLayout->addStretch();                   // 添加弹簧

    QHBoxLayout *pkgVersionLayout = new QHBoxLayout();  // 包版本的控件布局
    pkgVersionLayout->setSpacing(0);
    pkgVersionLayout->setContentsMargins(0, 0, 0, 0);  // 设置四周边距
    pkgVersionLayout->addSpacing(2);
    pkgVersionLayout->addWidget(packageVersion);    // 添加包版本提示的label
    pkgVersionLayout->addWidget(m_packageVersion);  // 添加包实际版本的label
    pkgVersionLayout->addStretch();

    QVBoxLayout *itemInfoLayout = new QVBoxLayout();  // 包名和包版本的布局
    itemInfoLayout->setSpacing(0);
    itemInfoLayout->setContentsMargins(0, 0, 0, 0);  // 设置四周边距
    itemInfoLayout->addStretch();
    itemInfoLayout->addLayout(pkgNameLayout);     // 添加包名布局
    itemInfoLayout->addSpacing(6);                // 调整UI效果，上下文本框间距6px
    itemInfoLayout->addLayout(pkgVersionLayout);  // 添加包版本布局
    itemInfoLayout->addStretch();

    QHBoxLayout *itemBlockLayout = new QHBoxLayout();  // 单包安装上半部分布局（包名，包版本和图标）
    itemBlockLayout->setSpacing(0);
    itemBlockLayout->setContentsMargins(0, 0, 0, 0);
    itemBlockLayout->addSpacing(112 - 20 - 10);  // 与标题栏保持一定的间距
    itemBlockLayout->addWidget(m_packageIcon);   // 添加图标
    itemBlockLayout->addLayout(itemInfoLayout);  // 添加包名和版本

    QWidget *itemInfoWidget = new QWidget(this);
    itemInfoWidget->setLayout(itemBlockLayout);  // 保存成一个widget

    QHBoxLayout *packageDescLayout = new QHBoxLayout();  // 包描述的 布局
    packageDescLayout->addStretch();
    packageDescLayout->addWidget(m_packageDescription);  // 添加包描述的Label
    packageDescLayout->addStretch();
    packageDescLayout->setSpacing(0);
    packageDescLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *itemLayout = new QVBoxLayout();  // 整合包信息的布局
    itemLayout->addSpacing(30);
    itemLayout->addWidget(itemInfoWidget);  // 添加包的信息
    itemLayout->addSpacing(20);
    itemLayout->addLayout(packageDescLayout);     // 添加包的描述
    itemLayout->setContentsMargins(0, 0, 0, 18);  // 设置和下方的边距
    itemLayout->setSpacing(0);

    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);

    Utils::bindFontBySizeAndWeight(packageVersion, mediumFontFamily, 14, QFont::DemiBold);  // 设置版本提示label的字体大小和样式
    Utils::bindFontBySizeAndWeight(packageName, mediumFontFamily, 14, QFont::DemiBold);  // 设置包名提示Label的字体大小和样式
    Utils::bindFontBySizeAndWeight(m_packageName, normalFontFamily, 14, QFont::ExtraLight);  // 设置实际包名label的字体大小和样式
    Utils::bindFontBySizeAndWeight(
        m_packageVersion, normalFontFamily, 14, QFont::ExtraLight);  // 设置实际版本Label的字体大小和样式

    m_itemInfoFrame->setLayout(itemLayout);
    m_itemInfoFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_itemInfoFrame->setVisible(false);

    m_contentLayout->addWidget(m_itemInfoFrame);
}

void SingleInstallPage::initTabOrder()
{
    // 调整tab切换焦点的顺序，第一个焦点是infoControlButton中的DCommandLinkButton
    QWidget::setTabOrder(m_infoControlButton->controlButton(), m_installButton);  // 当前包首次安装
    QWidget::setTabOrder(m_infoControlButton->controlButton(), m_backButton);     // 安装完成或安装失败
    QWidget::setTabOrder(m_backButton, m_doneButton);                             // 安装完成场景
    QWidget::setTabOrder(m_backButton, m_confirmButton);                          // 不能安装场景

    QWidget::setTabOrder(m_infoControlButton->controlButton(), m_uninstallButton);  // 当前场景为重新安装
    QWidget::setTabOrder(m_uninstallButton, m_reinstallButton);                     // 重新安装场景
}

void SingleInstallPage::initButtonFocusPolicy()
{
    this->setFocusPolicy(Qt::NoFocus);  // 主界面不接受焦点
    auto focus = Qt::TabFocus;          // 其余控件焦点为tabFocus
    m_installButton->setFocusPolicy(focus);
    m_uninstallButton->setFocusPolicy(focus);
    m_reinstallButton->setFocusPolicy(focus);
    m_confirmButton->setFocusPolicy(focus);
    m_backButton->setFocusPolicy(focus);
    m_doneButton->setFocusPolicy(focus);
    m_infoControlButton->controlButton()->setFocusPolicy(focus);
}

void SingleInstallPage::initButtonAutoDefault()
{
    m_installButton->setAutoDefault(true);
    m_uninstallButton->setAutoDefault(true);
    m_reinstallButton->setAutoDefault(true);
    m_confirmButton->setAutoDefault(true);
    m_backButton->setAutoDefault(true);
    m_doneButton->setAutoDefault(true);
}

void SingleInstallPage::initPkgInstallProcessView(int fontinfosize)
{
    int fontinfosizetemp = 0;  // 根据当前字体的大小设置按钮的字体大小
    if (fontinfosize > 16) {
        fontinfosizetemp = 21;
    } else {
        fontinfosizetemp = 18;
    }

    // 添加Accessible Name
    m_infoControlButton->setObjectName("InfoControlButton");
    m_infoControlButton->setAccessibleName("InfoControlButton");
    m_installProcessView->setObjectName("WorkerInformation");
    m_installProcessView->setAccessibleName("WorkerInformation");
    m_packageDescription->setObjectName("PackageDescription");
    m_packageDescription->setAccessibleName("PackageDescription");

    m_tipsLabel->setMinimumHeight(fontinfosizetemp);             // 设置提示label的高度
    m_tipsLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);  // 提示居中显示
    m_tipsLabel->setElideMode(Qt::ElideRight);

    m_progressFrame->setObjectName("progressFrame");
    m_progressFrame->setAccessibleName("progressFrame");
    m_progressFrame->setVisible(false);      // 默认隐藏进度条view
    m_infoControlButton->setVisible(false);  // infoControlButton 默认隐藏

    m_installProcessView->setVisible(false);      // 默认隐藏安装进程信息
    m_installProcessView->setAcceptDrops(false);  // 不接受拖入的数据
    m_installProcessView->setMinimumHeight(200);  // 设置高度
    m_installProcessView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 安装按钮
    m_installButton->setText(tr("Install", "button"));
    m_installButton->setVisible(false);

    // 卸载按钮
    m_uninstallButton->setText(tr("Remove", "button"));
    m_uninstallButton->setVisible(false);

    // 重新安装按钮
    m_reinstallButton->setText(tr("Reinstall"));
    m_reinstallButton->setVisible(false);

    // 确定按钮
    m_confirmButton->setText(tr("OK", "button"));
    m_confirmButton->setVisible(false);

    // 返回按钮
    m_backButton->setText(tr("Back", "button"));
    m_backButton->setVisible(false);

    // 完成按钮
    m_doneButton->setText(tr("Done", "button"));
    m_doneButton->setVisible(false);

    m_packageDescription->setWordWrap(true);  // 允许内容自动换行

    // 设置各个按钮的大小
    m_installButton->setMinimumWidth(120);
    m_uninstallButton->setMinimumWidth(120);
    m_reinstallButton->setMinimumWidth(120);
    m_confirmButton->setMinimumWidth(120);
    m_backButton->setMinimumWidth(120);
    m_doneButton->setMinimumWidth(120);

    // 启用焦点切换。
    initButtonFocusPolicy();
    // 设置按钮回车触发
    initButtonAutoDefault();

    // 设置描述信息的size 与位置
    m_packageDescription->setMinimumHeight(60);
    m_packageDescription->setMinimumWidth(270);
    m_packageDescription->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // 各个按钮的布局
    QVBoxLayout *btnsFrameLayout = new QVBoxLayout();
    btnsFrameLayout->setSpacing(0);
    btnsFrameLayout->setContentsMargins(0, 0, 0, 0);
    btnsFrameLayout->addSpacing(5);

    // 安装 卸载 重新安装 返回 完成 确认按钮的布局
    QHBoxLayout *btnsLayout = new QHBoxLayout();
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installButton, 0, Qt::AlignBottom);
    btnsLayout->addWidget(m_uninstallButton, 0, Qt::AlignBottom);
    btnsLayout->addWidget(m_reinstallButton, 0, Qt::AlignBottom);
    btnsLayout->addWidget(m_backButton, 0, Qt::AlignBottom);
    btnsLayout->addWidget(m_confirmButton, 0, Qt::AlignBottom);
    btnsLayout->addWidget(m_doneButton, 0, Qt::AlignBottom);
    btnsLayout->addStretch();
    btnsLayout->setSpacing(20);

#ifdef DTKWIDGET_CLASS_DSizeMode
    // adapt compact mode
    auto setBtnSizeMode = [btnsLayout]() {
        if (DGuiApplicationHelper::instance()->isCompactMode()) {
            btnsLayout->setContentsMargins(0, 0, 0, 4);
        } else {
            btnsLayout->setContentsMargins(0, 0, 0, 0);
        }
    };
    setBtnSizeMode();
    // setBtnSizeMode moved
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, setBtnSizeMode);

#else
    btnsLayout->setContentsMargins(0, 0, 0, 0);
#endif  // DTKWIDGET_CLASS_DSizeMode

    // 进度条 布局
    QVBoxLayout *progressLayout = new QVBoxLayout();
    progressLayout->setSpacing(0);
    // TODO：通过计算调整的高度，和UI图匹配，应当将界面修改为类 QWizardPage 方式布局
    if (m_inCompatibleMode) {
        // compatible mode will show tips label
        progressLayout->setContentsMargins(0, 10, 0, 28);
    } else {
        // Bottom margin: 48 = Content margin: 20 + Control margin: 28
        progressLayout->setContentsMargins(0, 0, 0, 28);
    }
    progressLayout->addStretch();
    progressLayout->addWidget(m_progress);
    progressLayout->setAlignment(m_progress, Qt::AlignHCenter);  // 进度条水平居中
    m_progressFrame->setLayout(progressLayout);
    m_progressFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 把所有的按钮合并成一个widget
    m_btnsFrame = new QWidget(this);
    m_btnsFrame->setObjectName("btnsFrame");
    m_btnsFrame->setAccessibleName("btnsFrame");
    m_btnsFrame->setMinimumHeight(m_installButton->maximumHeight());
    btnsFrameLayout->addLayout(btnsLayout);
    m_btnsFrame->setLayout(btnsFrameLayout);
    m_btnsFrame->setFixedHeight(45);

    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);

    // 设置各个按钮的字体大小与样式
    Utils::bindFontBySizeAndWeight(m_tipsLabel, normalFontFamily, 12, QFont::Normal);
    Utils::bindFontBySizeAndWeight(m_installButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_uninstallButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_reinstallButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_confirmButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_backButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_doneButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_packageDescription, normalFontFamily, 12, QFont::ExtraLight);

    // 将进度条布局。提示布局。按钮布局添加到主布局中
    m_contentLayout->addWidget(m_infoControlButton);
    m_contentLayout->addWidget(m_installProcessView);

    m_contentLayout->addStretch();
    m_contentLayout->addWidget(m_tipsLabel);

    // for compatible mode
    initCompatibleSelectLayout();

    m_contentLayout->addWidget(m_progressFrame);

    // 添加 wine下载等待提示布局
    initInstallWineLoadingLayout();
    m_tipsLabel->setMinimumHeight(20);
    m_tipsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_contentLayout->addWidget(m_btnsFrame);

    // bug 139875  Tab Order要在布局之后设置才能生效
    initTabOrder();
}

void SingleInstallPage::initPkgDependsInfoView()
{
    m_showDependsView->setVisible(false);
    m_showDependsButton->setVisible(false);
    m_showDependsView->setAcceptDrops(false);  // 不接受拖入的数据
    m_showDependsView->setMinimumHeight(200);  // 设置高度
    m_showDependsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_contentLayout->addWidget(m_showDependsButton);
    m_contentLayout->addWidget(m_showDependsView);
}

void SingleInstallPage::initConnections()
{
    // infoControlButton的展开与收缩
    connect(m_infoControlButton, &InfoControlButton::expand, this, &SingleInstallPage::slotShowInfomation);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &SingleInstallPage::slotHideInfomation);

    // showDependsButton的展开与收缩
    connect(m_showDependsButton, &InfoControlButton::expand, this, &SingleInstallPage::slotShowDependsInfo);
    connect(m_showDependsButton, &InfoControlButton::shrink, this, &SingleInstallPage::slotHideDependsInfo);

    // 各个按钮的触发事件
    connect(m_installButton, &DPushButton::clicked, this, &SingleInstallPage::slotInstall);
    connect(m_reinstallButton, &DPushButton::clicked, this, &SingleInstallPage::slotReinstall);
    connect(m_uninstallButton, &DPushButton::clicked, this, &SingleInstallPage::signalRequestUninstallConfirm);
    connect(m_backButton, &DPushButton::clicked, this, &SingleInstallPage::signalBacktoFileChooseWidget);
    connect(m_confirmButton, &DPushButton::clicked, this, [this]() {
        QModelIndex index = m_packagesModel->index(0);
        const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
        if (Finished != m_operate && Pkg::CompatibleNotInstalled == dependsStat) {
            // requset install package to compatible.
            m_targetRootfsOsName = m_compatibleBox->currentText();
            const QString targetRootfs = m_compatibleBox->currentData().toString();
            m_packagesModel->setData(index, targetRootfs, AbstractPackageListModel::CompatibleTargetRootfsRole);
            slotInstall();
        } else {
            qApp->quit();
        }
    });
    connect(m_doneButton, &DPushButton::clicked, qApp, &QApplication::quit);

    // model 安装进程信息的展示
    connect(m_packagesModel, &DebListModel::signalAppendOutputInfo, this, &SingleInstallPage::slotOutputAvailable);
    // transaction 进度改变 进度条进度改变
    connect(
        m_packagesModel, &DebListModel::signalCurrentPacakgeProgressChanged, this, &SingleInstallPage::slotWorkerProgressChanged);

    // 安装结束
    connect(m_packagesModel, &DebListModel::signalWorkerFinished, this, &SingleInstallPage::slotWorkerFinished);
}

int SingleInstallPage::initLabelWidth(int fontinfo)
{
    int fontlabelwidth = 0;
    switch (fontinfo) {
        case 11:
            fontlabelwidth = 260;
            break;
        case 12:
            fontlabelwidth = 255;
            break;
        case 13:
            fontlabelwidth = 250;
            break;
        case 14:
            fontlabelwidth = 250;
            break;
        case 15:
            fontlabelwidth = 240;
            break;
        case 16:
            fontlabelwidth = 240;
            break;
        case 18:
            fontlabelwidth = 230;
            break;
        case 20:
            fontlabelwidth = 220;
            break;
        default:
            fontlabelwidth = 220;
            break;
    }
    return fontlabelwidth;
}

void SingleInstallPage::slotReinstall()
{
    // 重装按钮点击后清除焦点
    m_reinstallButton->clearFocus();

    // 隐藏不需要的按钮
    m_backButton->setVisible(false);
    m_installButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);

    // 安装开始 隐藏包信息提示
    m_tipsLabel->setVisible(false);

    // 安装开始 显示安装进度
    m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Install", "Show details"));
    m_infoControlButton->setVisible(true);
    m_showDependsButton->shrinkContent();
    m_showDependsButton->setVisible(false);
    m_progress->setValue(0);
    m_progressFrame->setVisible(true);
    m_btnsFrame->setVisible(false);

    // for compatible mode
    if (m_inCompatibleMode) {
        m_compatibleLabel->setVisible(false);
        m_compatibleBox->setVisible(false);

        m_tipsLabel->setTextAndTips(
            tr("Trying to install %2 in %1 compatibility mode").arg(m_targetRootfsOsName).arg(m_pkgNameDescription));
        m_tipsLabel->setCustomDPalette(DPalette::TextLively);
        m_tipsLabel->setVisible(true);
    }

    // 重置包的工作状态
    m_operate = Reinstall;

    // 开始安装
    m_packagesModel->slotInstallPackages();
}

void SingleInstallPage::slotInstall()
{
    // 安装按钮点击后清除焦点
    m_installButton->clearFocus();

    // 隐藏按钮
    m_backButton->setVisible(false);
    m_installButton->setVisible(false);

    // 隐藏提示
    m_tipsLabel->setVisible(false);

    // 安装开始 显示安装进度
    m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Install", "Show details"));
    m_infoControlButton->setVisible(true);
    m_showDependsButton->shrinkContent();
    m_showDependsButton->setVisible(false);
    m_progress->setValue(0);
    m_progressFrame->setVisible(true);
    m_btnsFrame->setVisible(false);

    // for compatible mode
    if (m_inCompatibleMode) {
        m_compatibleLabel->setVisible(false);
        m_compatibleBox->setVisible(false);

        m_tipsLabel->setTextAndTips(
            tr("Trying to install %2 in %1 compatibility mode").arg(m_targetRootfsOsName).arg(m_pkgNameDescription));
        m_tipsLabel->setCustomDPalette(DPalette::TextLively);
        m_tipsLabel->setVisible(true);
    }

    // 重置工作状态
    m_operate = Install;

    // 开始安装
    m_packagesModel->slotInstallPackages();
}

void SingleInstallPage::slotUninstallCurrentPackage()
{
    // 卸载按钮点击后清除焦点
    m_uninstallButton->clearFocus();

    // 隐藏不需要的按钮
    m_tipsLabel->setVisible(false);
    m_backButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);

    // 卸载开始 显示进度
    m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Uninstall", "Show details"));
    m_infoControlButton->setVisible(true);
    m_progress->setValue(0);
    m_progressFrame->setVisible(true);
    m_showDependsButton->shrinkContent();
    m_showDependsButton->setVisible(false);
    m_btnsFrame->setVisible(false);

    // for compatible mode
    if (m_inCompatibleMode) {
        m_compatibleLabel->setVisible(false);
        m_compatibleBox->setVisible(false);

        m_tipsLabel->setTextAndTips(
            tr("Uninstalling %2 from %1 compatibility mode").arg(m_targetRootfsOsName).arg(m_pkgNameDescription));
        m_tipsLabel->setCustomDPalette(DPalette::TextLively);
        m_tipsLabel->setVisible(true);
    }

    // 重置工作状态
    m_operate = Uninstall;

    // 开始卸载
    m_packagesModel->slotUninstallPackage(0);
}

void SingleInstallPage::slotShowInfomation()
{
    m_upDown = false;
    m_installProcessView->setVisible(true);
    m_itemInfoFrame->setVisible(false);

    m_infoControlButton->setContentsMargins(0, 0, 0, 4);
}

void SingleInstallPage::slotHideInfomation()
{
    m_upDown = true;
    m_installProcessView->setVisible(false);
    m_itemInfoFrame->setVisible(true);

    m_infoControlButton->setContentsMargins(0, 0, 0, 0);
}

void SingleInstallPage::slotShowDependsInfo()
{
    m_showDependsView->setVisible(true);
    m_itemInfoFrame->setVisible(false);

    m_showDependsButton->setContentsMargins(0, 0, 0, 4);
}

void SingleInstallPage::slotHideDependsInfo()
{
    m_showDependsView->setVisible(false);
    m_itemInfoFrame->setVisible(true);

    m_showDependsButton->setContentsMargins(0, 0, 0, 0);
}

void SingleInstallPage::slotShowInfo()
{
    // 显示进度信息
    m_infoControlButton->setVisible(true);
    m_showDependsButton->setVisible(false);
    m_progressFrame->setVisible(true);
    m_btnsFrame->setVisible(false);

    /* Clear tips in normal mode, but not in compatible mode,
     * keep layout with visible on.
     */
    if (!m_inCompatibleMode) {
        m_tipsLabel->clear();
    }

    // 隐藏按钮
    m_installButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_confirmButton->setVisible(false);
    m_doneButton->setVisible(false);
    m_backButton->setVisible(false);
}

void SingleInstallPage::slotOutputAvailable(const QString &output)
{
    // 信息展示窗口添加进程数据
    m_installProcessView->appendText(output.trimmed());

    // 如果infoControlButton 未显示，则显示
    if (!m_infoControlButton->isVisible())
        m_infoControlButton->setVisible(true);

    // 切换状态为已经开始安装
    if (!m_workerStarted) {
        m_workerStarted = true;
        slotShowInfo();
    }
}

void SingleInstallPage::slotWorkerFinished()
{
    // 显示infoControlButton
    m_infoControlButton->setVisible(true);

    // 显示安装结果提示信息
    m_tipsLabel->setVisible(true);

    // 隐藏进度条
    m_progressFrame->setVisible(false);
    m_progress->setValue(0);

    // 显示需要显示的按钮
    m_btnsFrame->setVisible(true);
    m_uninstallButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_backButton->setVisible(true);

    if (m_inCompatibleMode) {
        m_confirmButton->setVisible(false);
    }

    // 获取当前包的安装结果
    QModelIndex index = m_packagesModel->index(0);
    const int stat = index.data(DebListModel::PackageOperateStatusRole).toInt();

    if (stat == Pkg::PackageOperationStatus::Success) {  // 操作成功
        m_doneButton->setVisible(true);
        m_doneButton->setFocus();
        if (m_operate == Install || m_operate == Reinstall) {  // 安装成功
            qDebug() << "SingleInstallPage: Installed successfully";
            m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Install", "Show details"));

            if (m_inCompatibleMode) {
                m_tipsLabel->setTextAndTips(tr("%2 was successfully installed to %1 compatibility mode")
                                                .arg(m_targetRootfsOsName)
                                                .arg(m_pkgNameDescription));
            } else {
                m_tipsLabel->setTextAndTips(tr("Installed successfully"));
            }

            m_tipsLabel->setCustomDPalette(DPalette::DarkLively);
        } else {  // 卸载成功
            qDebug() << "SingleInstallPage: Uninstalled successfully";
            m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Uninstall", "Show details"));

            if (m_inCompatibleMode) {
                m_tipsLabel->setTextAndTips(tr("%2 has been successfully uninstalled from %1 compatibility mode")
                                                .arg(m_rootfsOsName)
                                                .arg(m_pkgNameDescription));
            } else {
                m_tipsLabel->setTextAndTips(tr("Uninstalled successfully"));
            }

            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
        }
    } else if (stat == Pkg::PackageOperationStatus::Failed) {  // 安装/卸载失败
        m_confirmButton->setVisible(true);
        m_confirmButton->setFocus();
        m_tipsLabel->setCustomDPalette(DPalette::TextWarning);

        if (m_operate == Install || m_operate == Reinstall) {
            // 添加安装失败原因的提示
            QFont font;
            QFontMetrics elideFont(font);
            m_tipsLabel->setTextAndTips(elideFont.elidedText(index.data(DebListModel::PackageFailReasonRole).toString(),
                                                             Qt::ElideRight,
                                                             m_tipsLabel->width() - 100));  // 修复授权取消后无提示的问题
            m_tipsLabel->setToolTip(index.data(DebListModel::PackageFailReasonRole).toString());
        } else {
            m_tipsLabel->setTextAndTips(tr("Uninstall Failed"));  // 卸载只显示卸载失败
        }
    } else {
        // 正常情况不会进入此分支，如果进入此分支表明状态错误。
        m_confirmButton->setVisible(true);
        m_confirmButton->setFocus();
        qDebug() << "Operate Status Error. current"
                 << "index=" << index.row() << "stat=" << stat;
    }
    if (!m_upDown)
        m_infoControlButton->setShrinkTips(tr("Collapse"));

    // mark down
    m_operate = Finished;
}

void SingleInstallPage::slotWorkerProgressChanged(const int progress)
{
    if (progress < m_progress->value()) {  // 进度不后退
        return;
    }

    m_progress->setValue(progress);  // 进度增加
}

void SingleInstallPage::slotDependPackages(Pkg::DependsPair dependPackages, bool installWineDepends)
{
    // 依赖关系满足或者正在下载wine依赖，则不显示依赖关系
    m_showDependsView->clearText();
    m_showRemovePackages = false;

    // if compatible valid, disable extend info
    if (installWineDepends || m_inCompatibleMode) {
        return;
    }

    if (dependPackages.second.isEmpty()) {
        // depends ok or available, show package that will be remove after install
        const QStringList removePackages = m_packagesModel->index(0).data(DebListModel::PackageRemoveDependsRole).toStringList();
        if (!removePackages.isEmpty()) {
            QString removeInfo = tr("Install %1 will remove: ").arg(m_pkgNameDescription);
            removeInfo.append('\n');
            removeInfo.append(removePackages.join('\n'));
            removeInfo.append('\n');

            m_showDependsView->appendText(removeInfo);
            m_showDependsButton->setVisible(true);

            m_tipsLabel->setTextAndTips(removeInfo.simplified());
            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);

            m_showRemovePackages = true;
        }

        return;
    }

    m_showDependsButton->setVisible(true);

    if (dependPackages.first.size() > 0) {
        m_showDependsView->appendText(tr("Dependencies in the repository"));
        for (int i = 0; i < dependPackages.first.size(); i++)
            m_showDependsView->appendText(dependPackages.first.at(i).packageName + "   " + dependPackages.first.at(i).version);
        m_showDependsView->appendText(tr(""));
    }
    if (dependPackages.second.size() > 0) {
        m_showDependsView->appendText(tr("Missing dependencies"));
        for (int i = 0; i < dependPackages.second.size(); i++)
            m_showDependsView->appendText(dependPackages.second.at(i).packageName + "   " + dependPackages.second.at(i).version);
    }
    m_showDependsView->setTextCursor(QTextCursor::Start);
}

/**
   @brief Refresh package error depends info, do nothing if no errors occur.
        Will call at init.
 */
void SingleInstallPage::slotRefreshSinglePackageDepends()
{
    if (1 == m_packagesModel->rowCount()) {
        QModelIndex index = m_packagesModel->index(0);
        QVariant data = m_packagesModel->data(index, AbstractPackageListModel::PackageDependsDetailRole);
        auto depends = data.value<Pkg::DependsPair>();

        slotDependPackages(depends, GlobalStatus::winePreDependsInstalling());
    }
}

/**
 * @brief showPackageInfo 获取并显示deb包的信息
 */
void SingleInstallPage::showPackageInfo()
{
    const QSize boundingSize = QSize(m_packageDescription->width(), m_packageDescription->height());
    QFontInfo fontinfosize = this->fontInfo();  // 获取系统字体
    int fontlabelsize = fontinfosize.pixelSize();
    const QModelIndex index = m_packagesModel->index(0);

    if (m_packagesModel->isWorkerPrepare() && index.isValid()) {
        m_description = index.data(DebListModel::PackageLongDescriptionRole).toString();
        m_pkgNameDescription = index.data(DebListModel::PackageNameRole).toString();
        const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
        const int installStat = index.data(DebListModel::PackageVersionStatusRole).toInt();
        m_versionDescription = index.data(DebListModel::PackageVersionRole).toString();

        m_packageDescription->setText(Utils::holdTextInRect(m_packageDescription->font(), m_description, boundingSize));
        m_packageName->setText(
            m_packageName->fontMetrics().elidedText(m_pkgNameDescription, Qt::ElideRight, initLabelWidth(fontlabelsize)));
        m_packageVersion->setText(
            m_packageVersion->fontMetrics().elidedText(m_versionDescription, Qt::ElideRight, initLabelWidth(fontlabelsize)));
        // package install status
        // 否则会导致安装不同版本的包（依赖不同）时安装依赖出现问题（包括界面混乱、无法下载依赖等）
        // 根据依赖状态调整显示效果
        // 添加依赖授权确认处理
        if (dependsError(dependsStat) && dependAuthStatu != DebListModel::AuthConfirm) {  // 添加架构不匹配的处理
            m_tipsLabel->setTextAndTips(index.data(DebListModel::PackageFailReasonRole).toString());
            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);

            m_installButton->setVisible(false);
            m_reinstallButton->setVisible(false);
            m_infoControlButton->setVisible(false);
            m_confirmButton->setVisible(true);

            if (m_inCompatibleMode) {
                switch (dependsStat) {
                    case Pkg::CompatibleNotInstalled:
                        // no back button, select 'compatible rootfs' and install
                        m_compatibleLabel->setVisible(true);
                        m_compatibleBox->setVisible(true);
                        break;
                    case Pkg::CompatibleIntalled:
                        // uninstall or quit
                        m_compatibleLabel->setVisible(false);
                        m_compatibleBox->setVisible(false);
                        m_uninstallButton->setVisible(true);
                        break;
                    default:
                        break;
                }
            } else {
                m_backButton->setVisible(true);
            }

            if (resetButtonFocus) {
                resetButtonFocus = false;
                m_confirmButton->setFocus();
            }
            return;
        }

        // 根据安装状态调整显示效果
        const bool installed = installStat != Pkg::PackageInstallStatus::NotInstalled;
        if (dependAuthStatu == DebListModel::AuthConfirm) {  // 安装wine依赖时，所有的按钮都不显示
            m_installButton->setVisible(false);
            m_uninstallButton->setVisible(false);
            m_reinstallButton->setVisible(false);
            m_infoControlButton->setVisible(false);
        } else {  // 安装wine前或者安装完成后，根据安装状态显示对应的按钮
            m_installButton->setVisible(!installed);
            m_uninstallButton->setVisible(installed);
            m_reinstallButton->setVisible(installed);

            if (resetButtonFocus) {
                resetButtonFocus = false;
                if (installed) {
                    m_reinstallButton->setFocus();
                } else {
                    m_installButton->setFocus();
                }
            }
        }

        m_confirmButton->setVisible(false);
        m_doneButton->setVisible(false);

        // 根据安装状态设置提示文字
        if (installed) {
            if (installStat == Pkg::PackageInstallStatus::InstalledSameVersion) {
                m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
                m_tipsLabel->setTextAndTips(tr("Same version installed"));
                m_reinstallButton->setText(tr("Reinstall"));
            } else if (installStat == Pkg::PackageInstallStatus::InstalledLaterVersion) {
                m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
                m_tipsLabel->setTextAndTips(
                    tr("Later version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString()));
                m_reinstallButton->setText(tr("Downgrade"));
            } else {
                m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
                m_tipsLabel->setTextAndTips(
                    tr("Earlier version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString()));
                m_reinstallButton->setText(tr("Update", "button"));
            }
        }

        // refresh depends at init.
        slotRefreshSinglePackageDepends();
    }
}

void SingleInstallPage::setEnableButton(bool bEnable)
{
    // After the uninstall authorization is canceled, hide the uninstall details and display the version status
    m_tipsLabel->setVisible(true);
    m_tipsLabel->setVisible(true);

    // 设置各个按钮的可用性
    m_installButton->setEnabled(bEnable);
    m_reinstallButton->setEnabled(bEnable);
    m_uninstallButton->setEnabled(bEnable);
}

void SingleInstallPage::afterGetAutherFalse()
{
    // 取消授权/授权失败后，界面重绘，复位焦点
    resetButtonFocus = true;

    // 等待dpkg启动但是授权取消后，如果详细信息是expend状态，则shrink
    m_infoControlButton->shrink();
    m_infoControlButton->setVisible(false);
    m_progressFrame->setVisible(false);
    m_btnsFrame->setVisible(true);

    // 根据安装场景显示按钮
    if (m_operate == Install) {
        m_installButton->setVisible(true);
        m_installButton->setFocus();
    } else if (m_operate == Uninstall) {
        m_reinstallButton->setVisible(true);
        m_uninstallButton->setVisible(true);
        m_reinstallButton->setFocus();
    } else if (m_operate == Reinstall) {
        m_reinstallButton->setVisible(true);
        m_uninstallButton->setVisible(true);
        m_reinstallButton->setFocus();
    }

    if (m_showRemovePackages) {
        m_showDependsButton->setVisible(true);
    }
}

void SingleInstallPage::showEvent(QShowEvent *e)
{
    showPackageInfo();
    // 每次切换展示当前页面，复位焦点状态
    resetButtonFocus = true;
    QWidget::showEvent(e);
}

void SingleInstallPage::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    DPalette palette = DApplicationHelper::instance()->palette(m_packageDescription);
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextTips));
    m_packageDescription->setPalette(palette);
}

void SingleInstallPage::setAuthConfirm(QString dependName)
{
    // 调整按钮的显示效果
    m_installButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_confirmButton->setVisible(false);
    m_backButton->setVisible(false);

    // 显示等待动画
    m_pDSpinner->setVisible(true);
    m_pDSpinner->start();

    // 显示下载提示
    m_pLoadingLabel->setText(tr("Installing dependencies: %1").arg(dependName));
    m_pLoadingLabel->setVisible(true);

    // 隐藏包的提示
    m_tipsLabel->setVisible(false);
    m_infoControlButton->setVisible(false);
}

void SingleInstallPage::setAuthBefore()
{
    // 显示包信息提示
    m_tipsLabel->setVisible(true);

    // 隐藏进度条
    m_progressFrame->setVisible(false);
    m_btnsFrame->setVisible(true);

    // 获取依赖状态
    QModelIndex index = m_packagesModel->index(0);
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();

    // 依赖不满足或依赖授权被取消
    if (dependsError(dependsStat)) {
        m_tipsLabel->setCustomDPalette(DPalette::TextWarning);

        m_confirmButton->setVisible(true);
        m_backButton->setVisible(true);

        m_confirmButton->setFocus();
    } else {  // 依赖正常
        if (m_operate == Install) {
            m_installButton->setVisible(true);
            m_installButton->setFocus();
        } else if (m_operate == Uninstall) {
            m_reinstallButton->setVisible(true);
            m_uninstallButton->setVisible(true);
            m_reinstallButton->setFocus();
        } else if (m_operate == Reinstall) {
            m_reinstallButton->setVisible(true);
            m_uninstallButton->setVisible(true);
            m_reinstallButton->setFocus();
        }
        m_installButton->setEnabled(false);
        m_reinstallButton->setEnabled(false);
        m_uninstallButton->setEnabled(false);
    }

    // 依赖下载 进度显示
    m_pLoadingLabel->setVisible(false);
    m_pDSpinner->stop();
    m_pDSpinner->setVisible(false);
}

void SingleInstallPage::setCancelAuthOrAuthDependsErr()
{
    m_tipsLabel->setVisible(true);
    m_progressFrame->setVisible(false);
    m_btnsFrame->setVisible(true);

    // 获取依赖状态
    QModelIndex index = m_packagesModel->index(0);
    const int dependsStatus = index.data(DebListModel::PackageDependsStatusRole).toInt();
    // 根据依赖状态 调整界面显示
    if (dependsError(dependsStatus)) {
        // 依赖不满足或依赖授权取消
        m_tipsLabel->setTextAndTips(index.data(DebListModel::PackageFailReasonRole).toString());  // 修复授权取消后无提示的问题
        m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
        qWarning() << "SingleInstallPage:"
                   << "depends Break or Revoke installation authorization";
        m_confirmButton->setVisible(true);
        m_backButton->setVisible(true);
        m_confirmButton->setEnabled(true);
        m_backButton->setEnabled(true);
        m_installButton->setVisible(false);
        m_reinstallButton->setVisible(false);
        m_uninstallButton->setVisible(false);

        m_confirmButton->setFocus();
    } else {
        // 依赖安装成功
        m_tipsLabel->clear();  // 依赖安装成功后，去除依赖错误的提示信息。
        m_confirmButton->setVisible(false);
        m_backButton->setVisible(false);
        const int installStat = index.data(DebListModel::PackageVersionStatusRole).toInt();
        if (installStat == Pkg::PackageInstallStatus::NotInstalled) {  // 没有安装过其他版本
            m_installButton->setVisible(true);
            m_installButton->setEnabled(true);
            m_tipsLabel->setVisible(false);

            m_installButton->setFocus();
        } else {  // 已经安装过其他版本
            // 增加提示 依赖安装完成后的提示
            if (installStat == Pkg::PackageInstallStatus::InstalledSameVersion) {
                m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
                m_tipsLabel->setTextAndTips(tr("Same version installed"));
                m_reinstallButton->setText(tr("Reinstall"));
            } else if (installStat == Pkg::PackageInstallStatus::InstalledLaterVersion) {
                m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
                m_tipsLabel->setTextAndTips(
                    tr("Later version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString()));
                m_reinstallButton->setText(tr("Downgrade"));
            } else {
                m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
                m_tipsLabel->setTextAndTips(
                    tr("Earlier version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString()));
                m_reinstallButton->setText(tr("Update", "button"));
            }
            m_reinstallButton->setVisible(true);
            m_uninstallButton->setVisible(true);
            m_reinstallButton->setEnabled(true);
            m_uninstallButton->setEnabled(true);

            m_reinstallButton->setFocus();
        }
    }

    // 隐藏wine下载进度
    m_pLoadingLabel->setVisible(false);
    m_pDSpinner->stop();
    m_pDSpinner->setVisible(false);
}

bool SingleInstallPage::dependsError(int status) const
{
    return status != Pkg::DependsOk && status != Pkg::DependsAvailable;
}

void SingleInstallPage::DealDependResult(int authStatus, QString dependName)
{
    dependAuthStatu = authStatus;
    switch (authStatus) {
        case DebListModel::AuthConfirm:  // 授权成功
            setAuthConfirm(dependName);
            break;

        case DebListModel::AuthBefore:  // 授权之前
            setAuthBefore();
            break;
        case DebListModel::AnalysisErr:  // 解析错误
            setCancelAuthOrAuthDependsErr();
            break;
        case DebListModel::AuthDependsSuccess:  // 下载成功
            setCancelAuthOrAuthDependsErr();
            break;
        case DebListModel::CancelAuth:  // 授权取消和授权失败的提示语公用，修复授权取消后无提示的问题
        case DebListModel::AuthDependsErr:
            setCancelAuthOrAuthDependsErr();
            m_tipsLabel->setTextAndTips(tr("Failed to install %1").arg(dependName));
            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
            break;
        case DebListModel::VerifyDependsErr:
            setCancelAuthOrAuthDependsErr();
            m_tipsLabel->setTextAndTips(dependName + tr("Invalid digital signature"));
            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
            break;
        default:
            break;
    }
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packageslistdelegate.h"
#include "model/deblistmodel.h"
#include "utils/utils.h"

#include <QPixmap>
#include <QPainterPath>

#include <DSvgRenderer>
#include <DPalette>
#include <DStyleHelper>
#include <DApplicationHelper>
#include <DApplication>

DWIDGET_USE_NAMESPACE

// delegate 直接传入 model 解决多次创建model packagemanager导致崩溃的问题
PackagesListDelegate::PackagesListDelegate(AbstractPackageListModel *m_model, QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_fileListModel(m_model)  // 从新new一个对象修改为获取传入的对象
    , m_parentView(parent)
{
    qGuiApp->installEventFilter(this);  // 事件筛选

    m_itemHeight = 50 - 2 * (13 - DFontSizeManager::fontPixelSize(qGuiApp->font()));
    if (DFontSizeManager::fontPixelSize(qGuiApp->font()) > 13) {  // 当前字体大小是否小于13
        m_itemHeight += 2;
    }
}

void PackagesListDelegate::refreshDebItemStatus(
    const int operate_stat, QRect install_status_rect, QPainter *painter, bool isSelect, bool isEnable) const
{
    DPalette parentViewPattle = DebApplicationHelper::instance()->palette(m_parentView);

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette appPalette = dAppHelper->applicationPalette();
    QPen forground;  // 前景色

    QColor color;      // 画笔颜色
    QString showText;  // 要显示的文本信息（安装状态）

    DPalette::ColorGroup colorGroup;
    if (DApplication::activeWindow()) {  // 当前处于激活状态
        colorGroup = DPalette::Active;   // 设置Palette为激活状态
    } else {
        colorGroup = DPalette::Inactive;  // 设置Palette为非激活状态
    }

    // 根据操作的状态显示提示语
    switch (operate_stat) {
        case Pkg::PackageOperationStatus::Operating:  // 正在安装
            painter->setPen(QPen(parentViewPattle.color(DPalette::TextLively)));
            showText = tr("Installing");
            break;
        case Pkg::PackageOperationStatus::Success:  // 安装成功
            painter->setPen(QPen(parentViewPattle.color(DPalette::LightLively)));
            showText = tr("Installed");
            break;
        case Pkg::PackageOperationStatus::Waiting:  // 等待安装
            painter->setPen(QPen(parentViewPattle.color(DPalette::TextLively)));
            showText = tr("Waiting");
            break;
        default:  // 安装失败
            painter->setPen(QPen(parentViewPattle.color(DPalette::TextWarning)));
            showText = tr("Failed");
            break;
    }

    if (isSelect && isEnable) {  // 当前被选中 未被选中使用默认颜色
        forground.setColor(appPalette.color(colorGroup, DPalette::HighlightedText));
        painter->setPen(forground);
    }
    painter->drawText(install_status_rect, showText, Qt::AlignVCenter | Qt::AlignRight);  // 在item上添加安装提示
}

void PackagesListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {  // 判断传入的index是否有效
        DStyledItemDelegate::paint(painter, option, index);
        return;
    }

    auto themeType = DGuiApplicationHelper::instance()->themeType();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);

    const int content_x = 46;
    painter->setOpacity(1);

    QPainterPath bgPath;
    bgPath.addRect(option.rect);
    // 将当前Item的位置参数 发送给ListView,确定右键菜单的位置。
    emit sigIndexAndRect(option.rect, index.row());
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QBrush background;
    QPen forground;
    DPalette::ColorGroup colorGroup;
    if (!(option.state & DStyle::State_Enabled)) {  // 当前appListView not enable
        colorGroup = DPalette::Disabled;
    } else {
        if (!DApplication::activeWindow()) {  // 当前窗口未被激活
            colorGroup = DPalette::Inactive;
        } else {
            colorGroup = DPalette::Active;  // 当前窗口被激活
        }
    }
    background.setColor(QColor(0, 0, 0, 0));  // 未选中时直接背景透明

    // 被选中时设置颜色高亮
    forground.setColor(palette.color(colorGroup, DPalette::Text));
    if (option.state & DStyle::State_Enabled) {
        if (option.state & DStyle::State_Selected) {
            background = palette.color(colorGroup, DPalette::Highlight);
        }
    }
    painter->setPen(forground);
    painter->fillPath(bgPath, background);

    // 设置包名和版本号的字体颜色 fix bug: 59390
    forground.setColor(palette.color(colorGroup, DPalette::ToolTipText));

    // 绘制分割线
    QRect lineRect;
    lineRect.setX(content_x);
    int itemHeight = m_itemHeight;
    lineRect.setY(option.rect.y() + itemHeight - 1);
    lineRect.setWidth(option.rect.width() - content_x - 10);
    lineRect.setHeight(1);
    auto fillColor = themeType == DGuiApplicationHelper::LightType ? QColor(0, 0, 0, 12) : QColor(255, 255, 255, 12);
    painter->fillRect(lineRect, fillColor);

    QRect bg_rect = option.rect;

    Pkg::PackageType type = index.data(AbstractPackageListModel::PackageTypeRole).value<Pkg::PackageType>();
    QIcon icon = Utils::packageIcon(type);

    // draw package icon
    const int x = 6;
    int y = bg_rect.y() + (m_itemHeight - 32) / 2;

    icon.paint(painter, x, y, 32, 32);

    // draw package name
    QRect name_rect = bg_rect;
    name_rect.setX(content_x);
    name_rect.setY(bg_rect.y() + 5);

    const QString pkg_name = index.data(DebListModel::PackageNameRole).toString();
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString defaultFontFamily = Utils::loadFontFamilyByType(Utils::DefautFont);

    QFont pkg_name_font = Utils::loadFontBySizeAndWeight(mediumFontFamily, 14, QFont::Medium);
    pkg_name_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));

    name_rect.setHeight(pkg_name_font.pixelSize() + 7);

    painter->setFont(pkg_name_font);
    QFontMetrics fontMetric(pkg_name_font);

    const QString elided_pkg_name = fontMetric.elidedText(pkg_name, Qt::ElideRight, 150);

    if (option.state & DStyle::State_Enabled) {
        if (option.state & DStyle::State_Selected) {
            forground.setColor(palette.color(colorGroup, DPalette::HighlightedText));
        }
    }
    painter->setPen(forground);
    painter->drawText(name_rect, elided_pkg_name, Qt::AlignLeft | Qt::AlignVCenter);

    // draw package version
    QRect version_rect = name_rect;

    const int version_y = version_rect.top();
    version_rect.setLeft(200);
    version_rect.setTop(version_y);
    version_rect.setRight(option.rect.right() - 80);
    QFontMetrics versionFontMetric(pkg_name_font);
    const QString version = index.data(DebListModel::PackageVersionRole).toString();
    const QString version_str = versionFontMetric.elidedText(version, Qt::ElideRight, 195);
    painter->setPen(forground);
    QFont version_font = Utils::loadFontBySizeAndWeight(defaultFontFamily, 12, QFont::Light);
    version_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8));
    painter->setFont(version_font);
    painter->drawText(version_rect, version_str, Qt::AlignLeft | Qt::AlignVCenter);

    // install status
    const int operate_stat = index.data(DebListModel::PackageOperateStatusRole).toInt();  // 获取包的状态
    if (operate_stat != Pkg::PackageOperationStatus::Prepare) {
        QRect install_status_rect = option.rect;
        install_status_rect.setRight(option.rect.right() - 20);
        install_status_rect.setTop(version_y - 4);

        QFont stat_font = Utils::loadFontBySizeAndWeight(mediumFontFamily, 11, QFont::Medium);
        stat_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T9));
        painter->setFont(stat_font);
        // 刷新添加包状态的提示
        refreshDebItemStatus(operate_stat,
                             install_status_rect,
                             painter,
                             (option.state & DStyle::State_Selected),
                             (option.state & DStyle::State_Enabled));
    }

    // draw package info
    QString info_str;

    QRect info_rect = option.rect;
    info_rect.setLeft(content_x);
    info_rect.setTop(name_rect.bottom() + 2);

    // 获取包的版本
    const int install_stat = index.data(DebListModel::PackageVersionStatusRole).toInt();

    // 获取包的依赖状态
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
    DPalette pa = DebApplicationHelper::instance()->palette(m_parentView);

    // 未被选中，设置正常的颜色
    forground.setColor(palette.color(colorGroup, DPalette::ToolTipText));

    // 安装状态
    if (install_stat != Pkg::PackageInstallStatus::NotInstalled) {
        // 获取安装版本
        if (install_stat == Pkg::PackageInstallStatus::InstalledSameVersion) {  // 安装了相同版本
            info_str = tr("Same version installed");
        } else if (install_stat == Pkg::PackageInstallStatus::InstalledLaterVersion) {  // 安装了更新的版本
            info_str = tr("Later version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString());
        } else {  // 安装了较早的版本
            info_str = tr("Earlier version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString());
        }
        // fix bug: 43139
        forground.setColor(palette.color(colorGroup, DPalette::TextTips));
    } else {  // 当前没有安装过
        // 获取包的短描述（model增加长描述接口，批量安装显示的是短描述）
        info_str = index.data(DebListModel::PackageShortDescriptionRole).toString();
        // fix bug: 43139
        forground.setColor(palette.color(colorGroup, DPalette::TextTips));
    }

    if (operate_stat == Pkg::PackageOperationStatus::Failed) {
        info_str = index.data(DebListModel::PackageFailReasonRole).toString();
        forground.setColor(palette.color(colorGroup, DPalette::TextWarning));  // 安装失败或依赖错误
    }
    // not contains prohibit error
    if (dependsStat == Pkg::DependsStatus::DependsBreak || dependsStat == Pkg::DependsStatus::DependsAuthCancel ||
        dependsStat == Pkg::DependsStatus::DependsVerifyFailed || dependsStat == Pkg::DependsStatus::ArchBreak ||
        dependsStat == Pkg::CompatibleIntalled || dependsStat == Pkg::CompatibleNotInstalled) {
        info_str = index.data(DebListModel::PackageFailReasonRole).toString();
        forground.setColor(palette.color(colorGroup, DPalette::TextWarning));  // 安装失败或依赖错误
    }

    // No other error, show will remove packages
    if (dependsStat != Pkg::CompatibleIntalled && dependsStat != Pkg::CompatibleNotInstalled) {
        QStringList removePackages = index.data(DebListModel::PackageRemoveDependsRole).toStringList();
        if (!removePackages.isEmpty()) {
            forground.setColor(palette.color(colorGroup, DPalette::TextWarning));
            info_str = QObject::tr("Will remove: ") + removePackages.join(' ');
        }
    }

    // 当前选中 设置高亮
    if (option.state & DStyle::State_Enabled) {
        if (option.state & DStyle::State_Selected) {
            forground.setColor(palette.color(colorGroup, DPalette::HighlightedText));
        }
    }
    painter->setPen(forground);

    QFont info_font = Utils::loadFontBySizeAndWeight(normalFontFamily, 12, QFont::ExtraLight);
    info_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8));
    painter->setFont(info_font);
    info_str = painter->fontMetrics().elidedText(info_str, Qt::ElideRight, 306);
    painter->drawText(info_rect, info_str, Qt::AlignLeft | Qt::AlignTop);  // 将提示绘制到item上

    painter->restore();
}

QSize PackagesListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    Q_UNUSED(option);

    QSize itemSize = QSize(0, m_itemHeight);  // 设置Item的高度
    return itemSize;
}

bool PackagesListDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FontChange && watched == this) {
        QFontInfo fontinfo = m_parentView->fontInfo();
        emit fontinfo.pixelSize();
    }
    return QObject::eventFilter(watched, event);
}

void PackagesListDelegate::getItemHeight(int height)
{
    m_itemHeight = height;
}

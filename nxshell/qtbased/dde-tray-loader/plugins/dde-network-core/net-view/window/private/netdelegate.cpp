// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "netdelegate.h"

#include "neticonbutton.h"
#include "netitem.h"
#include "netmanager.h"
#include "netmodel.h"
#include "netsecretwidget.h"
#include "networkconst.h"

#include <DLabel>
#include <DSpinner>
#include <DStyleOption>
#include <DSwitchButton>

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QToolButton>
#include <QTextLine>
#include <QTextDocument>

DWIDGET_USE_NAMESPACE

#define MAX_TEXT_WIDTH 200

namespace dde {
namespace network {

NetDelegate::NetDelegate(QAbstractItemView *view)
    : QStyledItemDelegate(view)
    , m_view(view)
    , m_model(static_cast<QSortFilterProxyModel *>(m_view->model()))
{
}

NetDelegate::~NetDelegate() { }

ItemSpacing NetDelegate::getItemSpacing(const QModelIndex &index) const
{
    ItemSpacing spacing;
    spacing.left = 10;
    spacing.top = 0;
    spacing.right = 0;
    spacing.bottom = 0;
    spacing.height = -1;
    spacing.viewItemPosition = QStyleOptionViewItem::OnlyOne; // 显示样式

    switch (index.data(TYPEROLE).value<NetItemType>()) {
    case NetItemType::WirelessDeviceItem:
    case NetItemType::WiredDeviceItem:
    case NetItemType::VPNControlItem:
    case NetItemType::SystemProxyControlItem: {
        spacing.height = 40;
        spacing.top = index.row() == 0 ? 0 : 6;
        spacing.right = 10;
    } break;
    case NetItemType::WirelessMineItem: {
        spacing.height = 24;
    } break;
    case NetItemType::WiredItem:
    case NetItemType::WirelessHiddenItem:
    case NetItemType::WirelessItem: {
        spacing.height = 30;
        spacing.bottom = 1;
        // 计算ViewItemPosition,WirelessOtherItem的子项根据实际viewItemPosition显示
        QStyleOptionViewItem::ViewItemPosition viewItemPosition = QStyleOptionViewItem::Invalid;
        const QModelIndex &parentIndex = m_model->parent(index);
        int rowCount = m_model->rowCount(parentIndex);
        if (rowCount == 1) {
            viewItemPosition = QStyleOptionViewItem::OnlyOne;
        } else if (index.row() == 0) {
            viewItemPosition = QStyleOptionViewItem::Beginning;
        } else if (index.row() == rowCount - 1) {
            viewItemPosition = QStyleOptionViewItem::End;
        } else {
            viewItemPosition = QStyleOptionViewItem::Middle;
        }
        // 处理显示差异
        switch (parentIndex.data(TYPEROLE).value<NetItemType>()) {
        case NetItemType::WirelessMineItem:
        case NetItemType::WiredDeviceItem:
            spacing.height = 36;
            spacing.bottom = 10;
            break;
        case NetItemType::WirelessOtherItem:
            spacing.viewItemPosition = viewItemPosition;
            break;
        default:
            break;
        }
        // 处理最后项的spacing
        switch (viewItemPosition) {
        case QStyleOptionViewItem::Beginning:
        case QStyleOptionViewItem::Middle:
            break;
        default:
            spacing.bottom = 0;
            break;
        }
    } break;
    case NetItemType::WirelessOtherItem: {
        spacing.top = 10;
        spacing.height = 24;
        spacing.bottom = 4;
    } break;
    case NetItemType::WirelessDisabledItem:
    case NetItemType::WiredDisabledItem:
        spacing.height = 220;
        break;
    default:
        break;
    }
    return spacing;
}

void NetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DStyleOptionBackgroundGroup boption;
    boption.init(m_view);
    boption.QStyleOption::operator=(option);
    boption.state.setFlag(QStyle::State_Active);

    ItemSpacing itemSpacing = getItemSpacing(index);
    if (itemSpacing.left != 0 || itemSpacing.top != 0 || itemSpacing.right != 0 || itemSpacing.bottom != 0) {
        boption.rect.adjust(itemSpacing.left, itemSpacing.top, itemSpacing.right, -itemSpacing.bottom);
    }

    QColor bgColor, textColor;
    switch (index.data(TYPEROLE).value<NetItemType>()) {
    case NetItemType::WirelessOtherItem: {
        if (m_view->currentIndex() == index) {
            textColor = boption.dpalette.highlightedText().color();
            bgColor = boption.dpalette.highlight().color();
        } else {
            textColor = boption.dpalette.brightText().color();
            textColor.setAlphaF(0.6);
            bgColor = boption.dpalette.brightText().color();
            bgColor.setAlphaF(0.05);
        }
    } break;
    case NetItemType::WirelessDisabledItem:
    case NetItemType::WiredDisabledItem:
    case NetItemType::WirelessMineItem: {
        textColor = boption.dpalette.brightText().color();
        textColor.setAlphaF(0.6);
    } break;
    case NetItemType::WirelessItem:
    case NetItemType::WiredItem:
    case NetItemType::WirelessHiddenItem: {
        if (m_view->currentIndex() == index) {
            if (option.rect.height() > 100) {
                textColor = boption.dpalette.brightText().color();
                bgColor = boption.dpalette.brightText().color();
                bgColor.setAlphaF(0.1);
            } else {
                textColor = boption.dpalette.highlightedText().color();
                bgColor = boption.dpalette.highlight().color();
            }
        } else {
            textColor = boption.dpalette.brightText().color();
            bgColor = boption.dpalette.brightText().color();
            bgColor.setAlphaF(0.05);
        }
    } break;
    default:
        break;
    }
    if (textColor.isValid()) {
        boption.palette.setBrush(QPalette::BrightText, textColor);
        boption.palette.setBrush(QPalette::Highlight, textColor);
        if (auto widget = qobject_cast<NetWidget *>(m_view->indexWidget(index))) {
            if (auto w = widget->centralWidget()) {
                w->setPalette(boption.palette);
            }
        }
    }
    if (bgColor.isValid()) {
        boption.dpalette.setBrush(DPalette::ItemBackground, bgColor);
        boption.directions = Qt::Vertical;
        boption.position = DStyleOptionBackgroundGroup::ItemBackgroundPosition(itemSpacing.viewItemPosition);
        m_view->style()->drawPrimitive(static_cast<QStyle::PrimitiveElement>(DStyle::PE_ItemBackground), &boption, painter, option.widget);
    }
}

QWidget *NetDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    NetItem *item = static_cast<NetItem *>(m_model->mapToSource(index).internalPointer());
    NetWidget *netWidget = nullptr;
    if (item) {
        switch (item->itemType()) {
        case NetItemType::WirelessControlItem:
        case NetItemType::WiredControlItem:
        case NetItemType::WirelessDeviceItem:
        case NetItemType::WiredDeviceItem:
        case NetItemType::VPNControlItem:
        case NetItemType::SystemProxyControlItem: {
            netWidget = new NetDeviceWidget(static_cast<NetDeviceItem *>(item), parent);
        } break;
        case NetItemType::WirelessMineItem: {
            netWidget = new NetWirelessTypeControlWidget(static_cast<NetWirelessMineItem *>(item), parent);
        } break;
        case NetItemType::WirelessOtherItem: {
            netWidget = new NetWirelessTypeControlWidget(static_cast<NetWirelessOtherItem *>(item), parent);
        } break;
        case NetItemType::WirelessItem: {
            netWidget = new NetWirelessWidget(static_cast<NetWirelessItem *>(item), parent);
        } break;
        case NetItemType::WirelessHiddenItem: {
            netWidget = new NetWirelessHiddenWidget(static_cast<NetWirelessHiddenItem *>(item), parent);
        } break;
        case NetItemType::WiredItem: {
            netWidget = new NetWiredWidget(static_cast<NetWiredItem *>(item), parent);
        } break;
        case NetItemType::WirelessDisabledItem:
        case NetItemType::WiredDisabledItem: {
            netWidget = new NetDisabledWidget(item, parent);
        } break;
        case NetItemType::AirplaneModeTipsItem: {
            netWidget = new NetAirplaneModeTipsWidget(static_cast<NetAirplaneModeTipsItem *>(item), parent);
        } break;
        case NetItemType::VPNTipsItem: {
            netWidget = new NetVPNTipsWidget(static_cast<NetVPNTipsItem *>(item), parent);
        } break;
        default:
            qCWarning(DNC) << "Network item type error while craeting editor, item type: " << item->itemType() << ", item: " << item;
            return new QLabel(item->name(), parent);
        }
        if (netWidget) {
            connect(netWidget, &NetWidget::requestExec, this, &NetDelegate::requestExec);
            connect(netWidget, &NetWidget::requestShow, this, &NetDelegate::requestShow);
            connect(netWidget, &NetWidget::requestUpdateLayout, this, &NetDelegate::requestUpdateLayout);
            connect(this, &NetDelegate::request, netWidget, &NetWidget::exec);
        }
    }
    return netWidget;
}

QSize NetDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    ItemSpacing itemSpacing = getItemSpacing(index);
    return QSize(-1, itemSpacing.top + itemSpacing.height + itemSpacing.bottom);
}

void NetDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!editor)
        return;

    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    ItemSpacing itemSpacing = getItemSpacing(index);
    if (itemSpacing.left != 0 || itemSpacing.top != 0 || itemSpacing.right != 0 || itemSpacing.bottom != 0) {
        QRect geom = editor->geometry();
        geom.adjust(itemSpacing.left, itemSpacing.top, itemSpacing.right, -itemSpacing.bottom);
        editor->setGeometry(geom);
    }
}

void NetDelegate::destroyEditor(QWidget *editor, const QModelIndex &) const
{
    delete editor;
}

void NetDelegate::onRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    Q_EMIT request(cmd, id, param);
}

static int drawText(QPainter *painter, const QRectF &rect, int lineHeight, QTextLayout *layout, Qt::TextElideMode mode)
{
    int lineCount = 0;
    qreal height = 0;
    QString text = layout->text();
    QTextOption text_option = layout->textOption();

    if (painter) {
        text_option.setTextDirection(painter->layoutDirection());
    }

    layout->beginLayout();

    QTextLine line = layout->createLine();
    QPointF offset = rect.topLeft();

    while (line.isValid()) {
        ++lineCount;
        height += lineHeight;

        if (height + lineHeight > rect.height()) {
            QString end_str = text.mid(line.textStart());

            if (painter)
                end_str = painter->fontMetrics().elidedText(end_str, mode, qRound(rect.width() - 1));

            layout->endLayout();
            layout->setText(end_str);

            text_option.setWrapMode(QTextOption::NoWrap);
            layout->beginLayout();
            line = layout->createLine();
            line.setLineWidth(rect.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(rect.width());
        }

        line.setPosition(offset);

        if (painter) {
            line.draw(painter, QPointF(0, 0));
        }

        offset.setY(offset.y() + lineHeight);

        if (height + lineHeight > rect.height())
            break;

        line = layout->createLine();
    }
    layout->setTextOption(text_option);
    layout->endLayout();

    return lineCount;
}

NetWidget::NetWidget(NetItem *item, QWidget *parent)
    : QWidget(parent)
    , m_item(item)
    , m_mainLayout(new QVBoxLayout(this))
    , m_noMousePropagation(false)
{
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    if (parent)
        setForegroundRole(parent->foregroundRole());
}

NetWidget::~NetWidget() { }

void NetWidget::setCentralWidget(QWidget *widget)
{
    m_mainLayout->addWidget(widget);
}

QWidget *NetWidget::centralWidget() const
{
    if (m_mainLayout->count() <= 0)
        return nullptr;
    return m_mainLayout->itemAt(0)->widget();
}

void NetWidget::addPasswordWidget(QWidget *widget)
{
    m_mainLayout->addWidget(widget);
}

void NetWidget::setNoMousePropagation(bool noMousePropagation)
{
    m_noMousePropagation = noMousePropagation;
}

void NetWidget::removePasswordWidget()
{
    if (m_mainLayout->count() == 2) {
        setNoMousePropagation(false);
        QLayoutItem *item = m_mainLayout->takeAt(1);
        QWidget *w = item->widget();
        if (w)
            delete w;
        delete item;
    }
}

void NetWidget::exec(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    switch (cmd) {
    case NetManager::RequestPassword:
        showPassword(id, param);
        break;
    case NetManager::InputError:
        showError(id, param);
        break;
    case NetManager::InputValid:
        updateInputValid(id, param);
        break;
    case NetManager::CloseInput:
        closeInput();
        break;
    default:
        break;
    }
}

void NetWidget::showPassword(const QString &id, const QVariantMap &param)
{
    if (id != m_item->id() || m_mainLayout->count() >= 2)
        return;
    setNoMousePropagation(true);
    NetSecretWidget *secretWidget = new NetSecretWidget(this);
    secretWidget->setPalette(qApp->palette());
    secretWidget->initUI(param);
    addPasswordWidget(secretWidget);
    connect(secretWidget, &NetSecretWidget::submit, this, &NetWidget::onSubmit);
    connect(secretWidget, &NetSecretWidget::requestCheckInput, this, &NetWidget::onRequestCheckInput);
    Q_EMIT requestShow(id);
    Q_EMIT requestUpdateLayout();
}

void NetWidget::showError(const QString &id, const QVariantMap &param)
{
    if (id != m_item->id() || m_mainLayout->count() != 2)
        return;
    NetSecretWidget *secretWidget = qobject_cast<NetSecretWidget *>(m_mainLayout->itemAt(1)->widget());
    if (secretWidget)
        secretWidget->showError(param);
}

void NetWidget::onRequestCheckInput(const QVariantMap &param)
{
    sendRequest(NetManager::CheckInput, m_item->id(), param);
}

void NetWidget::updateInputValid(const QString &id, const QVariantMap &param)
{
    if (id != m_item->id() || m_mainLayout->count() != 2)
        return;
    NetSecretWidget *secretWidget = qobject_cast<NetSecretWidget *>(m_mainLayout->itemAt(1)->widget());
    if (secretWidget)
        secretWidget->updateInputValid(param);
}

void NetWidget::onSubmit(const QVariantMap &param)
{
    if (param.value("input", true).toBool()) {
        sendRequest(NetManager::Connect, m_item->id(), param);
    } else {
        sendRequest(NetManager::UserCancelRequest, m_item->id(), param);
    }
}

void NetWidget::closeInput()
{
    removePasswordWidget();
    Q_EMIT requestUpdateLayout();
}

void NetWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_noMousePropagation)
        QWidget::mousePressEvent(event);
}

void NetWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_noMousePropagation)
        QWidget::mousePressEvent(event);
}

void NetWidget::sendRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    Q_EMIT requestExec(cmd, id, param);
}

static QLabel *createNemeLabel(const NetItem *item, QWidget *parent, DFontSizeManager::SizeType fontSize)
{
    DLabel *label = new DLabel(item->name(), parent);
    label->setFixedWidth(MAX_TEXT_WIDTH);
    label->setElideMode(Qt::ElideRight);
    label->setForegroundRole(parent->foregroundRole());
    DFontSizeManager::instance()->bind(label, fontSize);
    parent->connect(item, &NetDeviceItem::nameChanged, label, &DLabel::setText);
    return label;
}

NetDeviceWidget::NetDeviceWidget(NetDeviceItem *item, QWidget *parent)
    : NetWidget(item, parent)
    , m_switchBut(new DSwitchButton(this))
{
    QWidget *widget = new QWidget(this);
    widget->setFixedHeight(36);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = createNemeLabel(item, this, DFontSizeManager::T5);
    layout->addWidget(label);
    layout->addStretch();
    switch (item->itemType()) {
    case NetItemType::WiredControlItem:
    case NetItemType::WirelessControlItem:
        DFontSizeManager::instance()->bind(label, DFontSizeManager::T4);
        break;
    case NetItemType::WirelessDeviceItem: {
        NetIconButton *refreshBut = new NetIconButton(this);
        refreshBut->setFixedSize(16, 16);
        refreshBut->setIcon(QIcon::fromTheme("refresh"));
        refreshBut->setClickable(true);
        refreshBut->setRotatable(true);
        refreshBut->setVisible(item->isEnabled());
        layout->addWidget(refreshBut);
        connect(refreshBut, &NetIconButton::clicked, this, &NetDeviceWidget::onScanClicked);
        connect(item, &NetDeviceItem::enabledChanged, refreshBut, &NetIconButton::setVisible);
    } break;
    default:
        break;
    }

    layout->addWidget(m_switchBut);
    setCentralWidget(widget);
    m_switchBut->setChecked(item->isEnabled());
    m_switchBut->setEnabled(item->enabledable());
    connect(item, &NetDeviceItem::enabledChanged, this, &NetDeviceWidget::onEnabledChanged);
    connect(item, &NetDeviceItem::enabledableChanged, m_switchBut, &DSwitchButton::setEnabled);
    connect(m_switchBut, &DSwitchButton::checkedChanged, this, &NetDeviceWidget::onCheckedChanged);
}

NetDeviceWidget::~NetDeviceWidget() { }

void NetDeviceWidget::onEnabledChanged(bool enabled)
{
    m_switchBut->blockSignals(true);
    m_switchBut->setChecked(enabled);
    m_switchBut->blockSignals(false);
}

void NetDeviceWidget::onCheckedChanged(bool checked)
{
    sendRequest(checked ? NetManager::EnabledDevice : NetManager::DisabledDevice, item()->id());
}

void NetDeviceWidget::onScanClicked()
{
    sendRequest(NetManager::RequestScan, item()->id());
}

NetWirelessTypeControlWidget::NetWirelessTypeControlWidget(NetItem *item, QWidget *parent)
    : NetWidget(item, parent)
    , m_expandButton(nullptr)
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(createNemeLabel(item, this, DFontSizeManager::T10));
    layout->addStretch();

    switch (item->itemType()) {
    case NetItemType::WirelessOtherItem: {
        m_expandButton = new NetIconButton(this);
        m_expandButton->setBackgroundRole(DPalette::Base);
        m_expandButton->setIcon(QIcon::fromTheme("network-arrow-down"));
        m_expandButton->setFixedSize(12, 12);
        m_expandButton->setClickable(true);
        connect(m_expandButton, &NetIconButton::clicked, this, &NetWirelessTypeControlWidget::onClicked);
        NetWirelessOtherItem *otherItem = NetItem::toItem<NetWirelessOtherItem>(item);
        updateExpandState(otherItem->isExpanded());
        connect(otherItem, &NetWirelessOtherItem::expandedChanged, this, &NetWirelessTypeControlWidget::updateExpandState);
        layout->addWidget(m_expandButton);
    } break;
    default:
        break;
    }

    widget->setFixedHeight(24);
    setCentralWidget(widget);
}

void NetWirelessTypeControlWidget::onClicked()
{
    NetWirelessOtherItem *otherItem = NetItem::toItem<NetWirelessOtherItem>(item());
    if (otherItem)
        sendRequest(NetManager::ToggleExpand, otherItem->id());
}

void NetWirelessTypeControlWidget::updateExpandState(bool isExpanded)
{
    if (!m_expandButton)
        return;
    const QString &icon = isExpanded ? "network-arrow-up" : "network-arrow-down";
    m_expandButton->setIcon(QIcon::fromTheme(icon));
}

NetWirelessTypeControlWidget::~NetWirelessTypeControlWidget() { }

NetWirelessWidget::NetWirelessWidget(NetWirelessItem *item, QWidget *parent)
    : NetWidget(item, parent)
    , m_isWifi6(item->flags())
    , m_iconBut(new NetIconButton(this))
    , m_connBut(new NetIconButton(this))
    , m_loading(new DSpinner(this))
{
    QWidget *widget = new QWidget(this);
    if (item->hasConnection()) {
        widget->setFixedHeight(36);
    } else {
        widget->setFixedHeight(30);
    }
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(3, 0, 5, 0);

    m_iconBut->setFixedSize(16, 16);
    layout->addWidget(m_iconBut);
    layout->addSpacing(3);

    layout->addWidget(createNemeLabel(item, this, DFontSizeManager::T6));
    layout->addStretch();

    m_connBut->setIcon(QIcon::fromTheme("select"));
    m_connBut->setHoverIcon(QIcon::fromTheme("disconnect"));
    m_connBut->setFixedSize(16, 16);
    m_connBut->setClickable(true);
    layout->addWidget(m_connBut);

    m_loading->setFixedSize(16, 16);
    layout->addWidget(m_loading);
    setCentralWidget(widget);

    updateIcon();
    onStatusChanged(item->status());

    connect(item, &NetWirelessItem::secureChanged, this, &NetWirelessWidget::updateIcon);
    connect(item, &NetWirelessItem::strengthLevelChanged, this, &NetWirelessWidget::updateIcon);
    connect(item, &NetWirelessItem::statusChanged, this, &NetWirelessWidget::onStatusChanged);
    connect(m_connBut, &NetIconButton::clicked, this, &NetWirelessWidget::onDisconnectClicked);
}

NetWirelessWidget::~NetWirelessWidget() { }

void NetWirelessWidget::updateIcon()
{
    NetWirelessItem *item = NetItem::toItem<NetWirelessItem>(this->item());
    m_iconBut->setIcon(QIcon::fromTheme(QString("network-wireless%1-symbolic").arg((item->flags() ? "-6" : "") + NetManager::StrengthLevelString(item->strengthLevel()) + (item->isSecure() ? "-secure" : ""))));
}

void NetWirelessWidget::onStatusChanged(NetConnectionStatus status)
{
    switch (status) {
    case NetConnectionStatus::Connected:
        m_connBut->setVisible(true);
        m_loading->stop();
        m_loading->setVisible(false);
        break;
    case NetConnectionStatus::Connecting:
        m_connBut->setVisible(false);
        m_loading->start();
        m_loading->setVisible(true);
        break;
    default:
        m_connBut->setVisible(false);
        m_loading->stop();
        m_loading->setVisible(false);
        break;
    }
}

void NetWirelessWidget::onDisconnectClicked()
{
    sendRequest(NetManager::Disconnect, item()->id());
}

NetWirelessHiddenWidget::NetWirelessHiddenWidget(NetWirelessHiddenItem *item, QWidget *parent)
    : NetWidget(item, parent)
{
    QWidget *widget = new QWidget(this);
    widget->setFixedHeight(30);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(31, 0, 0, 0);

    layout->addWidget(createNemeLabel(item, this, DFontSizeManager::T6));
    layout->addStretch();
    setCentralWidget(widget);
}

NetWirelessHiddenWidget::~NetWirelessHiddenWidget() { }

NetTipsWidget::NetTipsWidget(NetTipsItem *item, QWidget *parent)
    : NetWidget(item, parent)
{
    QLabel *label  = new QLabel(item->name(), this);
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T8);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignLeft);
    connect(label, &QLabel::linkActivated, this, [this, item] {
        sendRequest(NetManager::GoToControlCenter, item->linkActivatedText());
    });

    auto updateHeight = [item, label] {
        const QString &name = item->name();
        QTextDocument textDocument;
        textDocument.setHtml(name);
        const QString &plainText = textDocument.toPlainText();
        QTextLayout layout(plainText, label->font());
        QTextOption option;
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        option.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        layout.setTextOption(option);
        QFontMetrics fm(label->font());
        int line = drawText(nullptr, QRectF(QPointF(0, 0), QSizeF(294, INT_MAX)), fm.boundingRect(plainText).height(), &layout, Qt::ElideNone);
        label->setFixedHeight(line * fm.boundingRect(plainText).height());
        label->setText(item->tipsLinkEnabled() ? name : plainText);
    };

    connect(item, &NetTipsItem::nameChanged, this, updateHeight);
    updateHeight();
    setCentralWidget(label);
}

NetTipsWidget::~NetTipsWidget() { }

NetAirplaneModeTipsWidget::NetAirplaneModeTipsWidget(NetAirplaneModeTipsItem *item, QWidget *parent)
    : NetTipsWidget(item, parent)
{
}

NetAirplaneModeTipsWidget::~NetAirplaneModeTipsWidget() { }

NetVPNTipsWidget::NetVPNTipsWidget(NetVPNTipsItem *item, QWidget *parent)
    : NetTipsWidget(item, parent)
{
}

NetVPNTipsWidget::~NetVPNTipsWidget() { }

NetWiredWidget::NetWiredWidget(NetWiredItem *item, QWidget *parent)
    : NetWidget(item, parent)
    , m_iconBut(new NetIconButton(this))
    , m_connBut(new NetIconButton(this))
    , m_loading(new DSpinner(this))
{
    QWidget *widget = new QWidget(this);
    widget->setFixedHeight(36);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(3, 0, 5, 0);

    m_iconBut->setIcon(QIcon::fromTheme("network-wired-symbolic"));
    m_iconBut->setFixedSize(16, 16);
    layout->addWidget(m_iconBut);
    layout->addSpacing(3);

    layout->addWidget(createNemeLabel(item, this, DFontSizeManager::T6));
    layout->addStretch();

    m_connBut->setIcon(QIcon::fromTheme("select"));
    m_connBut->setHoverIcon(QIcon::fromTheme("disconnect"));
    m_connBut->setFixedSize(16, 16);
    m_connBut->setClickable(true);
    layout->addWidget(m_connBut);

    m_loading->setFixedSize(16, 16);
    layout->addWidget(m_loading);
    setCentralWidget(widget);

    onStatusChanged(item->status());

    connect(item, &NetWiredItem::statusChanged, this, &NetWiredWidget::onStatusChanged);
    connect(m_connBut, &NetIconButton::clicked, this, &NetWiredWidget::onDisconnectClicked);
}

NetWiredWidget::~NetWiredWidget() { }

void NetWiredWidget::onStatusChanged(NetConnectionStatus status)
{
    switch (status) {
    case NetConnectionStatus::Connected:
        m_connBut->setVisible(true);
        m_loading->stop();
        m_loading->setVisible(false);
        break;
    case NetConnectionStatus::Connecting:
        m_connBut->setVisible(false);
        m_loading->start();
        m_loading->setVisible(true);
        break;
    default:
        m_connBut->setVisible(false);
        m_loading->stop();
        m_loading->setVisible(false);
        break;
    }
}

void NetWiredWidget::onDisconnectClicked()
{
    sendRequest(NetManager::Disconnect, item()->id());
}

NetDisabledWidget::NetDisabledWidget(NetItem *item, QWidget *parent)
    : NetWidget(item, parent)
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setContentsMargins(3, 0, 5, 10);

    QToolButton *icon = new QToolButton(this);
    icon->setIconSize(QSize(96, 96));
    icon->setFixedSize(96, 96);
    icon->setAttribute(Qt::WA_TransparentForMouseEvents);
    icon->setFocusPolicy(Qt::NoFocus);
    if (item->itemType() == NetItemType::WiredDisabledItem) {
        icon->setIcon(QIcon::fromTheme("network-wired-disabled"));
    } else {
        icon->setIcon(QIcon::fromTheme("network-wireless-disabled"));
    }

    QLabel *label = createNemeLabel(item, this, DFontSizeManager::T8);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedWidth(QWIDGETSIZE_MAX);
    layout->addStretch();
    layout->addWidget(icon, 0, Qt::AlignHCenter);
    layout->addWidget(label);
    layout->addStretch();
    setCentralWidget(widget);
}

NetDisabledWidget::~NetDisabledWidget() { }

} // namespace network
} // namespace dde

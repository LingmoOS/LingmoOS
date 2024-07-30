/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpluginwidget.h"
#include "kcmoduleloader.h"
#include "kpluginproxymodel.h"
#include "kpluginwidget_p.h"

#include <kcmutils_debug.h>

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>

#include <KAboutPluginDialog>
#include <KCategorizedSortFilterProxyModel>
#include <KCategorizedView>
#include <KCategoryDrawer>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KStandardGuiItem>
#include <utility>

static constexpr int s_margin = 5;

int KPluginWidgetPrivate::dependantLayoutValue(int value, int width, int totalWidth) const
{
    if (listView->layoutDirection() == Qt::LeftToRight) {
        return value;
    }

    return totalWidth - width - value;
}

KPluginWidget::KPluginWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KPluginWidgetPrivate)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Adding content margins on a QLineEdit breaks inline actions
    auto lineEditWrapper = new QWidget(this);
    auto lineEditWrapperLayout = new QVBoxLayout(lineEditWrapper);
    lineEditWrapperLayout->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                              style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                                              style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                              style()->pixelMetric(QStyle::PM_LayoutBottomMargin));

    d->lineEdit = new QLineEdit(lineEditWrapper);
    d->lineEdit->setClearButtonEnabled(true);
    d->lineEdit->setPlaceholderText(i18n("Search…"));
    lineEditWrapperLayout->addWidget(d->lineEdit);
    d->listView = new KCategorizedView(this);
    d->listView->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::TopEdge}));
    d->categoryDrawer = new KCategoryDrawer(d->listView);
    d->listView->setVerticalScrollMode(QListView::ScrollPerPixel);
    d->listView->setAlternatingRowColors(true);
    d->listView->setCategoryDrawer(d->categoryDrawer);

    d->pluginModel = new KPluginModel(this);

    connect(d->pluginModel, &KPluginModel::defaulted, this, &KPluginWidget::defaulted);
    connect(d->pluginModel,
            &QAbstractItemModel::dataChanged,
            this,
            [this](const QModelIndex &topLeft, const QModelIndex & /*bottomRight*/, const QList<int> &roles) {
                if (roles.contains(KPluginModel::EnabledRole)) {
                    Q_EMIT pluginEnabledChanged(topLeft.data(KPluginModel::IdRole).toString(), topLeft.data(KPluginModel::EnabledRole).toBool());
                    Q_EMIT changed(d->pluginModel->isSaveNeeded());
                }
            });

    d->proxyModel = new KPluginProxyModel(this);
    d->proxyModel->setModel(d->pluginModel);
    d->listView->setModel(d->proxyModel);
    d->listView->setAlternatingRowColors(true);

    auto pluginDelegate = new PluginDelegate(d.get(), this);
    d->listView->setItemDelegate(pluginDelegate);

    d->listView->setMouseTracking(true);
    d->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(d->lineEdit, &QLineEdit::textChanged, d->proxyModel, [this](const QString &query) {
        d->proxyModel->setProperty("query", query);
        d->proxyModel->invalidate();
    });
    connect(pluginDelegate, &PluginDelegate::configCommitted, this, &KPluginWidget::pluginConfigSaved);
    connect(pluginDelegate, &PluginDelegate::changed, this, &KPluginWidget::pluginEnabledChanged);

    layout->addWidget(lineEditWrapper);
    layout->addWidget(d->listView);

    // When a KPluginWidget instance gets focus,
    // it should pass over the focus to its child searchbar.
    setFocusProxy(d->lineEdit);
}

KPluginWidget::~KPluginWidget()
{
    delete d->listView->itemDelegate();
    delete d->listView; // depends on some other things in d, make sure this dies first.
}

void KPluginWidget::addPlugins(const QList<KPluginMetaData> &plugins, const QString &categoryLabel)
{
    d->pluginModel->addPlugins(plugins, categoryLabel);
    d->proxyModel->sort(0);
}

void KPluginWidget::setConfig(const KConfigGroup &config)
{
    d->pluginModel->setConfig(config);
}

void KPluginWidget::clear()
{
    d->pluginModel->clear();
}

void KPluginWidget::save()
{
    d->pluginModel->save();
}

void KPluginWidget::load()
{
    d->pluginModel->load();
}

void KPluginWidget::defaults()
{
    d->pluginModel->defaults();
}

bool KPluginWidget::isDefault() const
{
    for (int i = 0, count = d->pluginModel->rowCount(); i < count; ++i) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        if (d->pluginModel->data(index, Qt::CheckStateRole).toBool() != d->pluginModel->data(index, KPluginModel::EnabledByDefaultRole).toBool()) {
            return false;
        }
    }

    return true;
}

bool KPluginWidget::isSaveNeeded() const
{
    return d->pluginModel->isSaveNeeded();
}

void KPluginWidget::setConfigurationArguments(const QVariantList &arguments)
{
    d->kcmArguments = arguments;
}

QVariantList KPluginWidget::configurationArguments() const
{
    return d->kcmArguments;
}

void KPluginWidget::showConfiguration(const QString &pluginId)
{
    QModelIndex idx;
    for (int i = 0, c = d->proxyModel->rowCount(); i < c; ++i) {
        const auto currentIndex = d->proxyModel->index(i, 0);
        const QString id = currentIndex.data(KPluginModel::IdRole).toString();
        if (id == pluginId) {
            idx = currentIndex;
            break;
        }
    }

    if (idx.isValid()) {
        auto delegate = static_cast<PluginDelegate *>(d->listView->itemDelegate());
        delegate->configure(idx);
    } else {
        qCWarning(KCMUTILS_LOG) << "Could not find plugin" << pluginId;
    }
}

void KPluginWidget::setDefaultsIndicatorsVisible(bool isVisible)
{
    auto delegate = static_cast<PluginDelegate *>(d->listView->itemDelegate());
    delegate->resetModel();

    d->showDefaultIndicator = isVisible;
}

void KPluginWidget::setAdditionalButtonHandler(const std::function<QPushButton *(const KPluginMetaData &)> &handler)
{
    auto delegate = static_cast<PluginDelegate *>(d->listView->itemDelegate());
    delegate->handler = handler;
}

PluginDelegate::PluginDelegate(KPluginWidgetPrivate *pluginSelector_d_ptr, QObject *parent)
    : KWidgetItemDelegate(pluginSelector_d_ptr->listView, parent)
    , checkBox(new QCheckBox)
    , pushButton(new QPushButton)
    , pluginSelector_d(pluginSelector_d_ptr)
{
    // set the icon to make sure the size can be properly calculated
    pushButton->setIcon(QIcon::fromTheme(QStringLiteral("configure-symbolic")));
}

PluginDelegate::~PluginDelegate()
{
    delete checkBox;
    delete pushButton;
}

void PluginDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    const int xOffset = checkBox->sizeHint().width();
    const bool disabled = !index.model()->data(index, KPluginModel::IsChangeableRole).toBool();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);

    const int iconSize = option.rect.height() - (s_margin * 2);
    QIcon icon = QIcon::fromTheme(index.model()->data(index, Qt::DecorationRole).toString());
    icon.paint(painter,
               QRect(pluginSelector_d->dependantLayoutValue(s_margin + option.rect.left() + xOffset, iconSize, option.rect.width()),
                     s_margin + option.rect.top(),
                     iconSize,
                     iconSize));

    QRect contentsRect(pluginSelector_d->dependantLayoutValue(s_margin * 2 + iconSize + option.rect.left() + xOffset,
                                                              option.rect.width() - (s_margin * 3) - iconSize - xOffset,
                                                              option.rect.width()),
                       s_margin + option.rect.top(),
                       option.rect.width() - (s_margin * 3) - iconSize - xOffset,
                       option.rect.height() - (s_margin * 2));

    int lessHorizontalSpace = s_margin * 2 + pushButton->sizeHint().width();
    if (index.model()->data(index, KPluginModel::ConfigRole).value<KPluginMetaData>().isValid()) {
        lessHorizontalSpace += s_margin + pushButton->sizeHint().width();
    }
    // Reserve space for extra button
    if (handler) {
        lessHorizontalSpace += s_margin + pushButton->sizeHint().width();
    }

    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    if (pluginSelector_d->listView->layoutDirection() == Qt::RightToLeft) {
        contentsRect.translate(lessHorizontalSpace, 0);
    }

    painter->save();
    if (disabled) {
        QPalette pal(option.palette);
        pal.setCurrentColorGroup(QPalette::Disabled);
        painter->setPen(pal.text().color());
    }

    painter->save();
    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(contentsRect,
                      Qt::AlignLeft | Qt::AlignTop,
                      fmTitle.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();

    painter->drawText(
        contentsRect,
        Qt::AlignLeft | Qt::AlignBottom,
        option.fontMetrics.elidedText(index.model()->data(index, KPluginModel::DescriptionRole).toString(), Qt::ElideRight, contentsRect.width()));

    painter->restore();
    painter->restore();
}

QSize PluginDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int i = 5;
    int j = 1;
    if (index.model()->data(index, KPluginModel::ConfigRole).value<KPluginMetaData>().isValid()) {
        i = 6;
        j = 2;
    }
    // Reserve space for extra button
    if (handler) {
        ++j;
    }

    const QFont font = titleFont(option.font);
    const QFontMetrics fmTitle(font);
    const QString text = index.model()->data(index, Qt::DisplayRole).toString();
    const QString comment = index.model()->data(index, KPluginModel::DescriptionRole).toString();
    const int maxTextWidth = qMax(fmTitle.boundingRect(text).width(), option.fontMetrics.boundingRect(comment).width());

    const auto iconSize = pluginSelector_d->listView->style()->pixelMetric(QStyle::PM_IconViewIconSize);
    return QSize(maxTextWidth + iconSize + s_margin * i + pushButton->sizeHint().width() * j,
                 qMax(iconSize + s_margin * 2, fmTitle.height() + option.fontMetrics.height() + s_margin * 2));
}

QList<QWidget *> PluginDelegate::createItemWidgets(const QModelIndex &index) const
{
    Q_UNUSED(index);
    QList<QWidget *> widgetList;

    auto enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, &QAbstractButton::clicked, this, &PluginDelegate::slotStateChanged);

    auto aboutPushButton = new QPushButton;
    aboutPushButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about-symbolic")));
    aboutPushButton->setToolTip(i18n("About"));
    connect(aboutPushButton, &QAbstractButton::clicked, this, &PluginDelegate::slotAboutClicked);

    auto configurePushButton = new QPushButton;
    configurePushButton->setIcon(QIcon::fromTheme(QStringLiteral("configure-symbolic")));
    configurePushButton->setToolTip(i18n("Configure"));
    connect(configurePushButton, &QAbstractButton::clicked, this, &PluginDelegate::slotConfigureClicked);

    const static QList<QEvent::Type> blockedEvents{
        QEvent::MouseButtonPress,
        QEvent::MouseButtonRelease,
        QEvent::MouseButtonDblClick,
        QEvent::KeyPress,
        QEvent::KeyRelease,
    };
    setBlockedEventTypes(enabledCheckBox, blockedEvents);

    setBlockedEventTypes(aboutPushButton, blockedEvents);

    setBlockedEventTypes(configurePushButton, blockedEvents);

    widgetList << enabledCheckBox << aboutPushButton << configurePushButton;
    if (handler) {
        QPushButton *btn = handler(pluginSelector_d->pluginModel->data(index, KPluginModel::MetaDataRole).value<KPluginMetaData>());
        if (btn) {
            widgetList << btn;
        }
    }

    return widgetList;
}

void PluginDelegate::updateItemWidgets(const QList<QWidget *> &widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const
{
    int extraButtonWidth = 0;
    QPushButton *extraButton = nullptr;
    if (widgets.count() == 4) {
        extraButton = static_cast<QPushButton *>(widgets[3]);
        extraButtonWidth = extraButton->sizeHint().width() + s_margin;
    }
    auto checkBox = static_cast<QCheckBox *>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(pluginSelector_d->dependantLayoutValue(s_margin, checkBox->sizeHint().width(), option.rect.width()),
                   option.rect.height() / 2 - checkBox->sizeHint().height() / 2);

    auto aboutPushButton = static_cast<QPushButton *>(widgets[1]);
    const QSize aboutPushButtonSizeHint = aboutPushButton->sizeHint();
    aboutPushButton->resize(aboutPushButtonSizeHint);
    aboutPushButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - s_margin - aboutPushButtonSizeHint.width() - extraButtonWidth,
                                                                 aboutPushButtonSizeHint.width(),
                                                                 option.rect.width()),
                          option.rect.height() / 2 - aboutPushButtonSizeHint.height() / 2);

    auto configurePushButton = static_cast<QPushButton *>(widgets[2]);
    const QSize configurePushButtonSizeHint = configurePushButton->sizeHint();
    configurePushButton->resize(configurePushButtonSizeHint);
    configurePushButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - s_margin * 2 - configurePushButtonSizeHint.width()
                                                                         - aboutPushButtonSizeHint.width() - extraButtonWidth,
                                                                     configurePushButtonSizeHint.width(),
                                                                     option.rect.width()),
                              option.rect.height() / 2 - configurePushButtonSizeHint.height() / 2);

    if (extraButton) {
        const QSize extraPushButtonSizeHint = extraButton->sizeHint();
        extraButton->resize(extraPushButtonSizeHint);
        extraButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - extraButtonWidth, extraPushButtonSizeHint.width(), option.rect.width()),
                          option.rect.height() / 2 - extraPushButtonSizeHint.height() / 2);
    }

    if (!index.isValid() || !index.internalPointer()) {
        checkBox->setVisible(false);
        aboutPushButton->setVisible(false);
        configurePushButton->setVisible(false);
        if (extraButton) {
            extraButton->setVisible(false);
        }
    } else {
        const bool enabledByDefault = index.model()->data(index, KPluginModel::EnabledByDefaultRole).toBool();
        const bool enabled = index.model()->data(index, KPluginModel::EnabledRole).toBool();
        checkBox->setProperty("_kde_highlight_neutral", pluginSelector_d->showDefaultIndicator && enabledByDefault != enabled);
        checkBox->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());
        checkBox->setEnabled(index.model()->data(index, KPluginModel::IsChangeableRole).toBool());
        configurePushButton->setVisible(index.model()->data(index, KPluginModel::ConfigRole).value<KPluginMetaData>().isValid());
        configurePushButton->setEnabled(index.model()->data(index, Qt::CheckStateRole).toBool());
    }
}

void PluginDelegate::slotStateChanged(bool state)
{
    if (!focusedIndex().isValid()) {
        return;
    }

    QModelIndex index = focusedIndex();

    const_cast<QAbstractItemModel *>(index.model())->setData(index, state, Qt::CheckStateRole);
}

void PluginDelegate::slotAboutClicked()
{
    const QModelIndex index = focusedIndex();

    auto pluginMetaData = index.data(KPluginModel::MetaDataRole).value<KPluginMetaData>();

    auto *aboutPlugin = new KAboutPluginDialog(pluginMetaData, itemView());
    aboutPlugin->setAttribute(Qt::WA_DeleteOnClose);
    aboutPlugin->show();
}

void PluginDelegate::slotConfigureClicked()
{
    configure(focusedIndex());
}

void PluginDelegate::configure(const QModelIndex &index)
{
    const QAbstractItemModel *model = index.model();
    const auto kcm = model->data(index, KPluginModel::ConfigRole).value<KPluginMetaData>();

    auto configDialog = new QDialog(itemView());
    configDialog->setAttribute(Qt::WA_DeleteOnClose);
    configDialog->setModal(true);
    configDialog->setWindowTitle(model->data(index, KPluginModel::NameRole).toString());

    QWidget *kcmWrapper = new QWidget;
    auto kcmInstance = KCModuleLoader::loadModule(kcm, kcmWrapper, pluginSelector_d->kcmArguments);

    auto layout = new QVBoxLayout(configDialog);
    layout->addWidget(kcmWrapper);

    auto buttonBox = new QDialogButtonBox(configDialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    connect(buttonBox, &QDialogButtonBox::accepted, configDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, configDialog, &QDialog::reject);
    connect(configDialog, &QDialog::accepted, this, [kcmInstance, this, model, index]() {
        Q_EMIT configCommitted(model->data(index, KPluginModel::IdRole).toString());
        kcmInstance->save();
    });
    connect(configDialog, &QDialog::rejected, this, [kcmInstance]() {
        kcmInstance->load();
    });

    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, [kcmInstance] {
        kcmInstance->defaults();
    });
    layout->addWidget(buttonBox);

    // Load KCM right before showing it
    kcmInstance->load();
    configDialog->show();
}

QFont PluginDelegate::titleFont(const QFont &baseFont) const
{
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}

#include "moc_kpluginwidget.cpp"
#include "moc_kpluginwidget_p.cpp"

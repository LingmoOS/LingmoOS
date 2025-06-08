/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_keyboard_widget.h"
#include "debug.h"

#include <KAboutData>
#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KWindowSystem>

#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#else
#include <QtGui/private/qtx11extras_p.h>
#endif

#include "workspace_options.h"
#include "keyboardmiscsettings.h"

#include "bindings.h"
#include "flags.h"
#include "kcm_add_layout_dialog.h"
#include "kcm_view_models.h"
#include "tastenbrett.h"
#include "x11_helper.h"
#include "xkb_rules.h"

#include "kcmmisc.h"
#include "ui_kcm_add_layout_dialog.h"

static const QString GROUP_SWITCH_GROUP_NAME(QStringLiteral("grp"));
static const QString LV3_SWITCH_GROUP_NAME(QStringLiteral("lv3"));
// static const QString RESET_XKB_OPTIONS("-option");

static const int TAB_HARDWARE = 0;
static const int TAB_LAYOUTS = 1;
static const int TAB_ADVANCED = 2;

static const int MIN_LOOPING_COUNT = 2;

KCMKeyboardWidget::KCMKeyboardWidget(Rules *rules_,
                                     KeyboardConfig *keyboardConfig_,
                                     WorkspaceOptions &workspaceOptions,
                                     KCMiscKeyboardWidget *kcmMiscWidget,
                                     const QVariantList &args,
                                     QWidget * /*parent*/)
    : rules(rules_)
    , m_workspaceOptions(workspaceOptions)
    , actionCollection(nullptr)
    , uiUpdating(false)
{
    flags = new Flags();
    keyboardConfig = keyboardConfig_;

    uiWidget = new Ui::TabWidget;
    uiWidget->setupUi(this);
    kcmMiscWidget->setParent(uiWidget->lowerHardwareWidget);
    uiWidget->lowerHardwareWidget->layout()->addWidget(kcmMiscWidget);

    if (rules != nullptr) {
        initializeKeyboardModelUI();
        initializeXkbOptionsUI();
        initializeLayoutsUI();
    } else {
        uiWidget->tabLayouts->setEnabled(false);
        uiWidget->tabAdvanced->setEnabled(false);
        uiWidget->keyboardModelComboBox->setEnabled(false);
    }

    handleParameters(args);
}

KCMKeyboardWidget::~KCMKeyboardWidget()
{
    delete uiWidget;
    delete flags;
}

void KCMKeyboardWidget::handleParameters(const QVariantList &args)
{
    // TODO: improve parameter handling
    setCurrentIndex(TAB_HARDWARE);
    for (const auto &arg : args) {
        if (arg.type() == QVariant::String) {
            const QString str = arg.toString();
            if (str == QLatin1String("--tab=layouts")) {
                setCurrentIndex(TAB_LAYOUTS);
            } else if (str == QLatin1String("--tab=advanced")) {
                setCurrentIndex(TAB_ADVANCED);
            }
        }
    }
}

void KCMKeyboardWidget::save()
{
    if (rules == nullptr)
        return;

    keyboardConfig->setKeyboardModel(keyboardModelFromUI());
    keyboardConfig->setSwitchingPolicy(switchingPolicyFromUI());

    saveXkbOptions();
}

void KCMKeyboardWidget::defaults()
{
    updateHardwareUI(keyboardConfig->defaultKeyboardModelValue());
    updateSwitchingPolicyUI(keyboardConfig->defaultSwitchingPolicyValue());
    auto *xkbOptionModel = dynamic_cast<XkbOptionsTreeModel *>(uiWidget->xkbOptionsTreeView->model());
    xkbOptionModel->setXkbOptions(keyboardConfig->defaultXkbOptionsValue());
    keyboardConfig->setDefaults();
}

bool KCMKeyboardWidget::isSaveNeeded() const
{
    return keyboardModelFromUI() != keyboardConfig->keyboardModel() ||
            switchingPolicyFromUI() != keyboardConfig->switchingPolicy() ||
            xkbOptionsFromUI() != keyboardConfig->xkbOptions() ||
            keyboardConfig->layoutsSaveNeeded();
}

bool KCMKeyboardWidget::isDefault() const
{
    return keyboardModelFromUI() == keyboardConfig->defaultKeyboardModelValue() &&
            switchingPolicyFromUI() == keyboardConfig->defaultSwitchingPolicyValue() &&
            xkbOptionsFromUI() == keyboardConfig->xkbOptions();
}

void KCMKeyboardWidget::setDefaultIndicator(bool visible)
{
    m_highlightVisible = visible;
    updateUiDefaultIndicator();
}

void KCMKeyboardWidget::updateUiDefaultIndicator()
{
    setDefaultIndicatorVisible(uiWidget->keyboardModelComboBox, m_highlightVisible && keyboardModelFromUI() != keyboardConfig->defaultKeyboardModelValue());
    const auto isDefaultswitchingPolicy = switchingPolicyFromUI() == keyboardConfig->defaultSwitchingPolicyValue();
    for (auto button : uiWidget->switchingPolicyButtonGroup->buttons()) {
        setDefaultIndicatorVisible(button, m_highlightVisible && !isDefaultswitchingPolicy && uiWidget->switchingPolicyButtonGroup->checkedButton() == button);
    }
}

void KCMKeyboardWidget::updateUI()
{
    if (rules == nullptr)
        return;

    uiWidget->layoutsTableView->setModel(uiWidget->layoutsTableView->model());
    layoutsTableModel->refresh();
    uiWidget->layoutsTableView->resizeRowsToContents();

    uiUpdating = true;
    updateHardwareUI(keyboardConfig->keyboardModel());
    updateSwitchingPolicyUI(keyboardConfig->switchingPolicy());
    XkbOptionsTreeModel *model = dynamic_cast<XkbOptionsTreeModel *>(uiWidget->xkbOptionsTreeView->model());
    model->setXkbOptions(keyboardConfig->xkbOptions());
    updateLayoutsUI();
    updateShortcutsUI();
    layoutSelectionChanged();
    uiUpdating = false;
}

void KCMKeyboardWidget::uiChanged()
{
    if (rules == nullptr) {
        return;
    }

    ((LayoutsTableModel *)uiWidget->layoutsTableView->model())->refresh();
    layoutSelectionChanged();
    // this collapses the tree so use more fine-grained updates
    //	((LayoutsTableModel*)uiWidget->xkbOptionsTreeView->model())->refresh();

    if (uiUpdating) {
        return;
    }

    updateXkbShortcutsButtons();

    updateLoopCount();
    int loop = uiWidget->layoutLoopCountSpinBox->text().isEmpty() ? KeyboardConfig::NO_LOOPING : uiWidget->layoutLoopCountSpinBox->value();
    keyboardConfig->setLayoutLoopCount(loop);

    layoutsTableModel->refresh();
    layoutSelectionChanged();
    // Refresh layout shortcuts
    switchKeyboardShortcutChanged();

    Q_EMIT changed(true);
}

void KCMKeyboardWidget::initializeKeyboardModelUI()
{
    for (const ModelInfo *modelInfo : qAsConst(rules->modelInfos)) {
        QString vendor = modelInfo->vendor;
        if (vendor.isEmpty()) {
            vendor = i18nc("unknown keyboard model vendor", "Unknown");
        }
        uiWidget->keyboardModelComboBox->addItem(i18nc("vendor | keyboard model", "%1 | %2", vendor, modelInfo->description), modelInfo->name);
    }
    uiWidget->keyboardModelComboBox->model()->sort(0);
    connect(uiWidget->keyboardModelComboBox, SIGNAL(activated(int)), this, SLOT(uiChanged()));
    connect(uiWidget->keyboardModelComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &KCMKeyboardWidget::updateUiDefaultIndicator);
}

void KCMKeyboardWidget::addLayout()
{
    AddLayoutDialog dialog(rules,
                           flags,
                           keyboardModelFromUI(),
                           xkbOptionsFromUI(),
                           false,
                           this);
    dialog.setModal(true);
    if (dialog.exec() == QDialog::Accepted) {
        keyboardConfig->layouts.append(dialog.getSelectedLayoutUnit());
        layoutsTableModel->refresh();
        uiWidget->layoutsTableView->resizeRowsToContents();
        uiChanged();
    }

    updateLoopCount();
}

void KCMKeyboardWidget::updateLoopCount()
{
    int maxLoop = qMin(X11Helper::MAX_GROUP_COUNT, keyboardConfig->layouts.count() - 1);
    uiWidget->layoutLoopCountSpinBox->setMaximum(qMax(MIN_LOOPING_COUNT, maxLoop));

    if (maxLoop < MIN_LOOPING_COUNT) {
        uiWidget->layoutLoopingCheckBox->setEnabled(false);
        uiWidget->layoutLoopingCheckBox->setChecked(false);
    } else if (maxLoop >= X11Helper::MAX_GROUP_COUNT) {
        uiWidget->layoutLoopingCheckBox->setEnabled(false);
        uiWidget->layoutLoopingCheckBox->setChecked(true);
    } else {
        uiWidget->layoutLoopingCheckBox->setEnabled(keyboardConfig->configureLayouts());
    }

    uiWidget->layoutLoopingGroupBox->setEnabled(keyboardConfig->configureLayouts() && uiWidget->layoutLoopingCheckBox->isChecked());

    if (uiWidget->layoutLoopingCheckBox->isChecked()) {
        if (uiWidget->layoutLoopCountSpinBox->text().isEmpty()) {
            uiWidget->layoutLoopCountSpinBox->setValue(maxLoop);
            keyboardConfig->setLayoutLoopCount(maxLoop);
        }
    } else {
        uiWidget->layoutLoopCountSpinBox->clear();
        keyboardConfig->setLayoutLoopCount(KeyboardConfig::NO_LOOPING);
    }
}

void KCMKeyboardWidget::initializeLayoutsUI()
{
    layoutsTableModel = new LayoutsTableModel(rules, flags, keyboardConfig, uiWidget->layoutsTableView);
    uiWidget->layoutsTableView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed
                                                | QAbstractItemView::AnyKeyPressed);
    uiWidget->layoutsTableView->setModel(layoutsTableModel);
    uiWidget->layoutsTableView->setIconSize({22, 22});

    // TODO: do we need to delete this delegate or parent will take care of it?
    VariantComboDelegate *variantDelegate = new VariantComboDelegate(keyboardConfig, rules, uiWidget->layoutsTableView);
    uiWidget->layoutsTableView->setItemDelegateForColumn(LayoutsTableModel::VARIANT_COLUMN, variantDelegate);

    LabelEditDelegate *labelDelegate = new LabelEditDelegate(keyboardConfig, uiWidget->layoutsTableView);
    uiWidget->layoutsTableView->setItemDelegateForColumn(LayoutsTableModel::DISPLAY_NAME_COLUMN, labelDelegate);

    KKeySequenceWidgetDelegate *shortcutDelegate = new KKeySequenceWidgetDelegate(keyboardConfig, uiWidget->layoutsTableView);
    uiWidget->layoutsTableView->setItemDelegateForColumn(LayoutsTableModel::SHORTCUT_COLUMN, shortcutDelegate);

    // TODO: is it ok to hardcode sizes? any better approach?
    uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::MAP_COLUMN, 70);
    uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::LAYOUT_COLUMN, 200);
    uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::VARIANT_COLUMN, 200);
    uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::DISPLAY_NAME_COLUMN, 50);
    uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::SHORTCUT_COLUMN, 130);

    connect(layoutsTableModel, &LayoutsTableModel::dataChanged, this, [this]() {
        Q_EMIT changed(true);
    });

    uiWidget->layoutLoopCountSpinBox->setMinimum(MIN_LOOPING_COUNT);

#ifdef DRAG_ENABLED
    uiWidget->layoutsTableView->setDragEnabled(true);
    uiWidget->layoutsTableView->setAcceptDrops(true);
#endif

    uiWidget->moveUpBtn->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up")));
    uiWidget->moveDownBtn->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
    uiWidget->addLayoutBtn->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    uiWidget->removeLayoutBtn->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));

    QIcon clearIcon =
        qApp->isLeftToRight() ? QIcon::fromTheme(QStringLiteral("edit-clear-locationbar-rtl")) : QIcon::fromTheme(QStringLiteral("edit-clear-locationbar-ltr"));
    uiWidget->xkbGrpClearBtn->setIcon(clearIcon);
    uiWidget->xkb3rdLevelClearBtn->setIcon(clearIcon);

    QIcon configIcon = QIcon::fromTheme(QStringLiteral("configure"));
    uiWidget->xkbGrpShortcutBtn->setIcon(configIcon);
    uiWidget->xkb3rdLevelShortcutBtn->setIcon(configIcon);

    uiWidget->kdeKeySequence->setModifierlessAllowed(false);

    uiWidget->kcfg_osdKbdLayoutChangedEnabled->setText(m_workspaceOptions.osdKbdLayoutChangedEnabledItem()->label());
    uiWidget->kcfg_osdKbdLayoutChangedEnabled->setToolTip(m_workspaceOptions.osdKbdLayoutChangedEnabledItem()->toolTip());

    connect(uiWidget->addLayoutBtn, &QAbstractButton::clicked, this, &KCMKeyboardWidget::addLayout);
    connect(uiWidget->removeLayoutBtn, &QAbstractButton::clicked, this, &KCMKeyboardWidget::removeLayout);
    connect(uiWidget->layoutsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KCMKeyboardWidget::layoutSelectionChanged);
    connect(uiWidget->layoutsTableView->model(), qOverload<const QModelIndex &, const QModelIndex &, const QVector<int> &>(&QAbstractItemModel::dataChanged),
            this, &KCMKeyboardWidget::uiChanged);

    connect(uiWidget->moveUpBtn, &QAbstractButton::clicked, this, &KCMKeyboardWidget::moveUp);
    connect(uiWidget->moveDownBtn, &QAbstractButton::clicked, this, &KCMKeyboardWidget::moveDown);

    connect(uiWidget->previewButton, &QAbstractButton::clicked, this, &KCMKeyboardWidget::previewLayout);
    connect(uiWidget->xkbGrpClearBtn, &QAbstractButton::clicked, this, &KCMKeyboardWidget::clearGroupShortcuts);
    connect(uiWidget->xkb3rdLevelClearBtn, &QAbstractButton::clicked, this, &KCMKeyboardWidget::clear3rdLevelShortcuts);

    connect(uiWidget->kdeKeySequence, &KKeySequenceWidget::keySequenceChanged, this, &KCMKeyboardWidget::alternativeShortcutChanged);
    connect(uiWidget->switchingPolicyButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(uiChanged()));
    connect(uiWidget->switchingPolicyButtonGroup, &QButtonGroup::idClicked, this, &KCMKeyboardWidget::updateUiDefaultIndicator);

    connect(uiWidget->xkbGrpShortcutBtn, &QAbstractButton::clicked, this, &KCMKeyboardWidget::scrollToGroupShortcut);
    connect(uiWidget->xkb3rdLevelShortcutBtn, &QAbstractButton::clicked, this, &KCMKeyboardWidget::scrollTo3rdLevelShortcut);

    connect(uiWidget->kcfg_configureLayouts, &QGroupBox::toggled, this, &KCMKeyboardWidget::configureLayoutsChanged);

    connect(uiWidget->layoutLoopingCheckBox, &QAbstractButton::clicked, this, &KCMKeyboardWidget::uiChanged);
    connect(uiWidget->layoutLoopCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(uiChanged()));
}

void KCMKeyboardWidget::previewLayout()
{
    QModelIndex index = uiWidget->layoutsTableView->currentIndex();

    QModelIndex idcountry = index.sibling(index.row(), 0);
    const QString country = uiWidget->layoutsTableView->model()->data(idcountry).toString();
    const QModelIndex idvariant = index.sibling(index.row(), 2);
    QString variant = uiWidget->layoutsTableView->model()->data(idvariant).toString();
    const QString model = keyboardModelFromUI();
    const QStringList options = xkbOptionsFromUI();

    const LayoutInfo *layoutInfo = rules->getLayoutInfo(country);
    if (!layoutInfo) {
        return;
    }

    for (const VariantInfo *variantInfo : layoutInfo->variantInfos) {
        if (variant == variantInfo->description) {
            variant = variantInfo->name;
            break;
        }
    }

    const QString title = Flags::getLongText(LayoutUnit(country, variant), rules);
    Tastenbrett::launch(model, country, variant, options.join(','), title);
}

void KCMKeyboardWidget::alternativeShortcutChanged(const QKeySequence &seq)
{
    Q_UNUSED(seq)

    if (rules == nullptr) {
        return;
    }

    if (actionCollection == nullptr) {
        actionCollection = new KeyboardLayoutActionCollection(this, true);
    }
    actionCollection->setToggleShortcut(uiWidget->kdeKeySequence->keySequence());
}

void KCMKeyboardWidget::switchKeyboardShortcutChanged()
{
    if (rules == nullptr) {
        return;
    }

    if (actionCollection == nullptr) {
        actionCollection = new KeyboardLayoutActionCollection(this, true);
    }
    actionCollection->resetLayoutShortcuts();
    actionCollection->setLayoutShortcuts(keyboardConfig->layouts, rules);
}

void KCMKeyboardWidget::configureLayoutsChanged()
{
    if (uiWidget->kcfg_configureLayouts->isChecked() && keyboardConfig->layouts.isEmpty()) {
        populateWithCurrentLayouts();
    } else {
        keyboardConfig->layouts.clear();
    }
    uiChanged();
}

static QPair<int, int> getSelectedRowRange(const QModelIndexList &selected)
{
    if (selected.isEmpty()) {
        return QPair<int, int>(-1, -1);
    }

    QList<int> rows;
    for (const auto &index : selected) {
        rows << index.row();
    }
    std::sort(rows.begin(), rows.end());
    return QPair<int, int>(rows[0], rows[rows.size() - 1]);
}

void KCMKeyboardWidget::layoutSelectionChanged()
{
    QModelIndexList selected = uiWidget->layoutsTableView->selectionModel()->selectedIndexes();
    uiWidget->removeLayoutBtn->setEnabled(!selected.isEmpty());
    QPair<int, int> rowsRange(getSelectedRowRange(selected));
    uiWidget->moveUpBtn->setEnabled(!selected.isEmpty() && rowsRange.first > 0);
    uiWidget->previewButton->setEnabled(uiWidget->layoutsTableView->selectionModel()->selectedRows().size() == 1);
    uiWidget->previewButton->setVisible(Tastenbrett::exists());
    uiWidget->moveDownBtn->setEnabled(!selected.isEmpty() && rowsRange.second < keyboardConfig->layouts.size() - 1);
}

void KCMKeyboardWidget::removeLayout()
{
    if (!uiWidget->layoutsTableView->selectionModel()->hasSelection())
        return;

    const QModelIndexList selected = uiWidget->layoutsTableView->selectionModel()->selectedIndexes();
    QPair<int, int> rowsRange(getSelectedRowRange(selected));
    for (const auto &idx : selected) {
        if (idx.column() == 0) {
            keyboardConfig->layouts.removeAt(rowsRange.first);
        }
    }
    layoutsTableModel->refresh();
    uiChanged();

    if (keyboardConfig->layouts.size() > 0) {
        int rowToSelect = rowsRange.first;
        if (rowToSelect >= keyboardConfig->layouts.size()) {
            rowToSelect--;
        }

        QModelIndex topLeft = layoutsTableModel->index(rowToSelect, 0, QModelIndex());
        QModelIndex bottomRight = layoutsTableModel->index(rowToSelect, layoutsTableModel->columnCount(topLeft) - 1, QModelIndex());
        QItemSelection selection(topLeft, bottomRight);
        uiWidget->layoutsTableView->selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);
        uiWidget->layoutsTableView->setFocus();
    }

    layoutSelectionChanged();

    updateLoopCount();
}

void KCMKeyboardWidget::moveUp()
{
    moveSelectedLayouts(-1);
}

void KCMKeyboardWidget::moveDown()
{
    moveSelectedLayouts(1);
}

void KCMKeyboardWidget::moveSelectedLayouts(int shift)
{
    QItemSelectionModel *selectionModel = uiWidget->layoutsTableView->selectionModel();
    if (selectionModel == nullptr || !selectionModel->hasSelection())
        return;

    const QModelIndexList selected = selectionModel->selectedRows();
    if (selected.count() < 1)
        return;

    int newFirstRow = selected[0].row() + shift;
    int newLastRow = selected[selected.size() - 1].row() + shift;

    if (newFirstRow >= 0 && newLastRow <= keyboardConfig->layouts.size() - 1) {
        QList<int> selectionRows;
        for (const QModelIndex &index : selected) {
            int newRowIndex = index.row() + shift;
            keyboardConfig->layouts.move(index.row(), newRowIndex);
            selectionRows << newRowIndex;
        }
        uiChanged();

        QItemSelection selection;
        for (const int row : qAsConst(selectionRows)) {
            QModelIndex topLeft = layoutsTableModel->index(row, 0, QModelIndex());
            QModelIndex bottomRight = layoutsTableModel->index(row, layoutsTableModel->columnCount(topLeft) - 1, QModelIndex());
            selection << QItemSelectionRange(topLeft, bottomRight);
        }
        uiWidget->layoutsTableView->selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);
        uiWidget->layoutsTableView->setFocus();
    }
}

void KCMKeyboardWidget::scrollToGroupShortcut()
{
    this->setCurrentIndex(TAB_ADVANCED);
    if (!uiWidget->kcfg_resetOldXkbOptions->isChecked()) {
        uiWidget->kcfg_resetOldXkbOptions->setChecked(true);
    }
    ((XkbOptionsTreeModel *)uiWidget->xkbOptionsTreeView->model())->gotoGroup(GROUP_SWITCH_GROUP_NAME, uiWidget->xkbOptionsTreeView);
}

void KCMKeyboardWidget::scrollTo3rdLevelShortcut()
{
    this->setCurrentIndex(TAB_ADVANCED);
    if (!uiWidget->kcfg_resetOldXkbOptions->isChecked()) {
        uiWidget->kcfg_resetOldXkbOptions->setChecked(true);
    }
    ((XkbOptionsTreeModel *)uiWidget->xkbOptionsTreeView->model())->gotoGroup(LV3_SWITCH_GROUP_NAME, uiWidget->xkbOptionsTreeView);
}

void KCMKeyboardWidget::clearGroupShortcuts()
{
    clearXkbGroup(GROUP_SWITCH_GROUP_NAME);
}

void KCMKeyboardWidget::clear3rdLevelShortcuts()
{
    clearXkbGroup(LV3_SWITCH_GROUP_NAME);
}

void KCMKeyboardWidget::clearXkbGroup(const QString &groupName)
{
    auto *xkbOptionModel = dynamic_cast<XkbOptionsTreeModel *>(uiWidget->xkbOptionsTreeView->model());
    QStringList xkbOptions = xkbOptionModel->xkbOptions();
    for (int ii = xkbOptions.count() - 1; ii >= 0; ii--) {
        if (xkbOptions.at(ii).startsWith(groupName + Rules::XKB_OPTION_GROUP_SEPARATOR)) {
            xkbOptions.removeAt(ii);
        }
    }
    xkbOptionModel->setXkbOptions(xkbOptions);

    xkbOptionModel->reset();
    uiWidget->xkbOptionsTreeView->update();
    updateXkbShortcutsButtons();
    Q_EMIT changed(true);
}

static bool xkbOptionGroupLessThan(const OptionGroupInfo *og1, const OptionGroupInfo *og2)
{
    return og1->description.toLower() < og2->description.toLower();
}
static bool xkbOptionLessThan(const OptionInfo *o1, const OptionInfo *o2)
{
    return o1->description.toLower() < o2->description.toLower();
}

void KCMKeyboardWidget::initializeXkbOptionsUI()
{
    std::sort(rules->optionGroupInfos.begin(), rules->optionGroupInfos.end(), xkbOptionGroupLessThan);
    for (OptionGroupInfo *optionGroupInfo : qAsConst(rules->optionGroupInfos)) {
        std::sort(optionGroupInfo->optionInfos.begin(), optionGroupInfo->optionInfos.end(), xkbOptionLessThan);
    }

    XkbOptionsTreeModel *model = new XkbOptionsTreeModel(rules, uiWidget->xkbOptionsTreeView);
    uiWidget->xkbOptionsTreeView->setModel(model);
    connect(model, &QAbstractItemModel::dataChanged, this, &KCMKeyboardWidget::uiChanged);

    connect(uiWidget->kcfg_resetOldXkbOptions, &QAbstractButton::toggled, this, &KCMKeyboardWidget::configureXkbOptionsChanged);
    connect(uiWidget->kcfg_resetOldXkbOptions, &QAbstractButton::toggled, uiWidget->xkbOptionsTreeView, &QWidget::setEnabled);
}

void KCMKeyboardWidget::configureXkbOptionsChanged()
{
    if (uiWidget->kcfg_resetOldXkbOptions->isChecked() && keyboardConfig->xkbOptions().isEmpty()) {
        populateWithCurrentXkbOptions();
    }
    ((XkbOptionsTreeModel *)uiWidget->xkbOptionsTreeView->model())->reset();
    uiChanged();
}

void KCMKeyboardWidget::saveXkbOptions()
{
    QStringList options;

    if(uiWidget->kcfg_resetOldXkbOptions->isChecked()) {
        options = xkbOptionsFromUI();

        // QStringLists with a single empty string are serialized as "\\0", avoid that
        // by saving them as an empty list instead. This way it can be passed as-is to
        // libxkbcommon/setxkbmap. Before KConfigXT it used QStringList::join(",").
        if (options.size() == 1 && options.constFirst().isEmpty()) {
            options.clear();
        }
    }

    keyboardConfig->setXkbOptions(options);
}

QStringList KCMKeyboardWidget::xkbOptionsFromUI() const
{
    XkbOptionsTreeModel *model = dynamic_cast<XkbOptionsTreeModel *>(uiWidget->xkbOptionsTreeView->model());
    return model->xkbOptions();
}

void KCMKeyboardWidget::updateSwitchingPolicyUI(KeyboardConfig::SwitchingPolicy policy)
{
    switch (policy) {
    case KeyboardConfig::SWITCH_POLICY_DESKTOP:
        uiWidget->switchByDesktopRadioBtn->setChecked(true);
        break;
    case KeyboardConfig::SWITCH_POLICY_APPLICATION:
        uiWidget->switchByApplicationRadioBtn->setChecked(true);
        break;
    case KeyboardConfig::SWITCH_POLICY_WINDOW:
        uiWidget->switchByWindowRadioBtn->setChecked(true);
        break;
    default:
    case KeyboardConfig::SWITCH_POLICY_GLOBAL:
        uiWidget->switchByGlobalRadioBtn->setChecked(true);
    }
}

void KCMKeyboardWidget::updateXkbShortcutButton(const QString &groupName, QPushButton *button)
{
    QStringList grpOptions;
    if (uiWidget->kcfg_resetOldXkbOptions->isChecked()) {
        QRegularExpression regexp("^" + groupName + Rules::XKB_OPTION_GROUP_SEPARATOR);
        XkbOptionsTreeModel *model = dynamic_cast<XkbOptionsTreeModel *>(uiWidget->xkbOptionsTreeView->model());
        grpOptions = model->xkbOptions().filter(regexp);
    }
    switch (grpOptions.size()) {
    case 0:
        button->setText(i18nc("no shortcuts defined", "None"));
        break;
    case 1: {
        const QString &option = grpOptions.first();
        const OptionGroupInfo *optionGroupInfo = rules->getOptionGroupInfo(groupName);
        const OptionInfo *optionInfo = optionGroupInfo->getOptionInfo(option);
        if (optionInfo == nullptr || optionInfo->description == nullptr) {
            qCDebug(KCM_KEYBOARD) << "Could not find option info for " << option;
            button->setText(grpOptions.first());
        } else {
            button->setText(optionInfo->description);
        }
    } break;
    default:
        button->setText(i18np("%1 shortcut", "%1 shortcuts", grpOptions.size()));
    }
}

void KCMKeyboardWidget::updateXkbShortcutsButtons()
{
    updateXkbShortcutButton(GROUP_SWITCH_GROUP_NAME, uiWidget->xkbGrpShortcutBtn);
    updateXkbShortcutButton(LV3_SWITCH_GROUP_NAME, uiWidget->xkb3rdLevelShortcutBtn);
}

void KCMKeyboardWidget::updateShortcutsUI()
{
    updateXkbShortcutsButtons();

    delete actionCollection;
    actionCollection = new KeyboardLayoutActionCollection(this, true);
    QAction *toggleAction = actionCollection->getToggleAction();
    const auto shortcuts = KGlobalAccel::self()->shortcut(toggleAction);
    uiWidget->kdeKeySequence->setKeySequence(shortcuts.isEmpty() ? QKeySequence() : shortcuts.first());

    actionCollection->loadLayoutShortcuts(keyboardConfig->layouts, rules);
    layoutsTableModel->refresh();
}

void KCMKeyboardWidget::updateLayoutsUI()
{
    bool loopingOn = keyboardConfig->configureLayouts() && keyboardConfig->layoutLoopCount() != KeyboardConfig::NO_LOOPING;
    uiWidget->layoutLoopingCheckBox->setChecked(loopingOn);
    uiWidget->layoutLoopingGroupBox->setEnabled(loopingOn);
    if (loopingOn) {
        // Set maximum to 99 to make sure following setValue succeeds
        // Correct maximum value will be set in updateLoopCount()
        uiWidget->layoutLoopCountSpinBox->setMaximum(99);
        uiWidget->layoutLoopCountSpinBox->setValue(keyboardConfig->layoutLoopCount());
    } else {
        uiWidget->layoutLoopCountSpinBox->clear();
    }

    updateLoopCount();
}

void KCMKeyboardWidget::updateHardwareUI(const QString &model)
{
    int idx = uiWidget->keyboardModelComboBox->findData(model);
    if (idx != -1) {
        uiWidget->keyboardModelComboBox->setCurrentIndex(idx);
    }
}

void KCMKeyboardWidget::populateWithCurrentLayouts()
{
    const QList<LayoutUnit> layouts = X11Helper::getLayoutsList();
    for (const auto &layoutUnit : layouts) {
        keyboardConfig->layouts.append(layoutUnit);
    }
}

void KCMKeyboardWidget::populateWithCurrentXkbOptions()
{
    if (!KWindowSystem::isPlatformX11()) {
        // TODO: implement for Wayland - query dbus maybe?
        return;
    }
    XkbConfig xkbConfig;
    QStringList xkbOptions;
    if (X11Helper::getGroupNames(QX11Info::display(), &xkbConfig, X11Helper::ALL)) {
        xkbOptions = xkbConfig.options;
    }
    auto *xkbOptionModel = dynamic_cast<XkbOptionsTreeModel *>(uiWidget->xkbOptionsTreeView->model());
    xkbOptionModel->setXkbOptions(xkbOptions);
    keyboardConfig->setXkbOptions(xkbOptions);
}

QString KCMKeyboardWidget::keyboardModelFromUI() const
{
    return uiWidget->keyboardModelComboBox->itemData(uiWidget->keyboardModelComboBox->currentIndex()).toString();
}

KeyboardConfig::SwitchingPolicy KCMKeyboardWidget::switchingPolicyFromUI() const
{
    if (uiWidget->switchByDesktopRadioBtn->isChecked()) {
        return KeyboardConfig::SWITCH_POLICY_DESKTOP;
    } else if (uiWidget->switchByApplicationRadioBtn->isChecked()) {
        return KeyboardConfig::SWITCH_POLICY_APPLICATION;
    } else if (uiWidget->switchByWindowRadioBtn->isChecked()) {
        return KeyboardConfig::SWITCH_POLICY_WINDOW;
    } else {
        return KeyboardConfig::SWITCH_POLICY_GLOBAL;
    }
}

void KCMKeyboardWidget::setDefaultIndicatorVisible(QWidget *widget, bool visible)
{
    widget->setProperty("_kde_highlight_neutral", visible);
    widget->update();
}

// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imsettingwindow.h"

#include "addimpage.h"
#include "configproxy.h"
#include "varianthelper.h"

#include <configwidget.h>
#include <libintl.h>
#include <model.h>
#include <widgets/keysettingsitem.h>
#include <widgets/settingsgroup.h>
#include <widgets/settingshead.h>
#include <widgets/settingsitem.h>
#include <widgets/titlelabel.h>

#include <DCommandLinkButton>
#include <DFloatingButton>
#include <DFontSizeManager>
#include <DListView>
#include <DStandardItem>
#include <DWidgetUtil>
#include <DDBusSender>
#include <DUtil>

#include <QEvent>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE

const QString SOGOU_IM_UNIQUE_NAME = "com.sogou.ime.ng.fcitx5.deepin";
const QString SOGOU_CONFIGURE_APP_ID = "com.sogou.ime.ng.fcitx5.deepin.configurer";

using namespace dcc_fcitx_configtool::widgets;

IMSettingWindow::IMSettingWindow(fcitx::kcm::DBusProvider *dbus, QWidget *parent)
    : QWidget(parent)
    , m_dbus(dbus)
    , m_config(new fcitx::kcm::IMConfig(dbus, fcitx::kcm::IMConfig::Tree, this))
    , m_configProxy(new fcitx::kcm::ConfigWidget("fcitx://config/global", m_dbus, this))
{
    initUI();
    initConnect();
}

IMSettingWindow::~IMSettingWindow() { }

void IMSettingWindow::initUI()
{
    m_configProxy->hide();

    // 滑动窗口
    m_mainLayout = new QVBoxLayout();

    QVBoxLayout *scrollAreaLayout = new QVBoxLayout(this);
    scrollAreaLayout->setContentsMargins(10, 0, 10, 0);
    scrollAreaLayout->setSpacing(0);

    QHBoxLayout *imHeaderLayout = new QHBoxLayout();

    // 创建标题
    m_editHead = new DCC_NAMESPACE::SettingsHead();
    m_editHead->setEditEnable(false);
    m_editHead->setTitle(tr("Manage Input Methods"));
    m_editHead->layout()->setContentsMargins(10, 4, 10, 0);
    m_deleteBtn = new DIconButton(DStyle::SP_DecreaseElement);
    m_deleteBtn->setEnabled(false);
    auto *addBtn = new DIconButton(DStyle::SP_IncreaseElement);
    connect(m_deleteBtn, &DIconButton::clicked, this, [=]() {
        qInfo() << "user clicked delete button";
        int row = m_IMListGroup->currentIndex().row();
        onItemDelete(row);
    });

    imHeaderLayout->addWidget(m_editHead);
    imHeaderLayout->addStretch();
    imHeaderLayout->addWidget(m_deleteBtn);
    imHeaderLayout->addWidget(addBtn);

    connect(addBtn, &DIconButton::clicked, this, [=]() {
        qInfo() << "user clicked add button";
        AddIMPage mainWindow(m_dbus, m_config, this);
        Dtk::Widget::moveToCenter(&mainWindow);
        mainWindow.exec();
    });

    // 输入法管理 编辑按钮
    m_IMListGroup = new DListView(this);
    m_IMListGroup->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_IMListGroup->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    // m_IMListGroup->setSwitchAble(true);
    m_IMListGroup->setSpacing(0);
    m_IMListGroup->setAttribute(Qt::WA_Hover, true);
    m_IMListGroup->installEventFilter(this);

    m_IMListModel = new QStandardItemModel(this);
    m_IMListGroup->setModel(m_IMListModel);

    onCurIMChanged(m_config->currentIMModel());

    connect(m_IMListGroup->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            [this](const QModelIndex &current, [[maybe_unused]] const QModelIndex &previous) {
                m_deleteBtn->setEnabled(current.isValid());
                QTimer::singleShot(0, [this]() {
                    updateActions();
                });
            });

    QHBoxLayout *shortcutLayout = new QHBoxLayout();
    auto *headTitle = new DCC_NAMESPACE::TitleLabel(tr("Shortcuts"));
    headTitle->setContentsMargins(10, 0, 0, 0);
    DFontSizeManager::instance()->bind(headTitle,
                                       DFontSizeManager::T5,
                                       QFont::DemiBold); // 设置label字体
    m_resetBtn = new DCommandLinkButton(tr("Restore Defaults"), this);
    m_resetBtn->setAccessibleName("Restore Defaults");
    shortcutLayout->addWidget(headTitle);
    shortcutLayout->addStretch();
    shortcutLayout->addWidget(m_resetBtn);

    // 快捷键 切换输入法 切换虚拟键盘 切换至默认输入法
    m_shortcutGroup =
        new DCC_NAMESPACE::SettingsGroup(nullptr, DCC_NAMESPACE::SettingsGroup::GroupBackground);
    m_enumerateForwardKeysComboBox =
        new DCC_NAMESPACE::ComboxWidget(tr("Scroll between input methods"));
    m_enumerateForwardKeysComboBox->setComboxOption(
        { "NONE", "CTRL_SHIFT", "ALT_SHIFT", "CTRL_SUPER", "ALT_SUPER" });
    m_enumerateForwardKeysComboBox->comboBox()->setAccessibleName("Switch input methods");
    for (auto child : m_enumerateForwardKeysComboBox->children()) {
        if (auto label = qobject_cast<QLabel *>(child)) {
            QFontMetrics fm(label->font());
            int pixelsWide = fm.horizontalAdvance(tr("Scroll between input methods"));
            label->setMinimumWidth(pixelsWide);
            break;
        }
    }
    m_defaultIMKey = new FcitxKeySettingsItem(tr("Switch between the current/first input method"));
    m_shortcutGroup->appendItem(m_enumerateForwardKeysComboBox);
    m_shortcutGroup->appendItem(m_defaultIMKey);

    // 控件添加至滑动窗口内
    scrollAreaLayout->addLayout(imHeaderLayout);
    scrollAreaLayout->addSpacing(10);
    scrollAreaLayout->addWidget(m_IMListGroup);
    scrollAreaLayout->addSpacing(30);

    scrollAreaLayout->addLayout(shortcutLayout);
    scrollAreaLayout->addSpacing(10);
    scrollAreaLayout->addWidget(m_shortcutGroup);
    scrollAreaLayout->addSpacing(20);

    m_advSetKey = new QPushButton(tr("Advanced Settings"));
    m_advSetKey->setAccessibleName("Advanced Settings");
    m_advSetKey->setMaximumWidth(214);

    scrollAreaLayout->addWidget(m_advSetKey, 0, Qt::AlignHCenter);
    scrollAreaLayout->addStretch();

    // 添加至主界面内
    setLayout(scrollAreaLayout);
    qInfo() << "read config:" << m_IMListModel->rowCount();
}

void IMSettingWindow::initConnect()
{
    connect(m_config, &fcitx::kcm::IMConfig::imListChanged, this, [=]() {
        qInfo() << "list changed:" << m_IMListModel->rowCount();
        onCurIMChanged(m_config->currentIMModel());
    });

    m_configProxy->requestConfig(true);

    {
        auto value =
            fcitx::kcm::readVariant(m_configProxy->value(), "Hotkey/EnumerateForwardKeys/0")
                .toString();
        if (value.contains("Alt")) {
            if (value.contains("Shift")) {
                m_enumerateForwardKeysComboBox->comboBox()->setCurrentText("ALT_SHIFT");
            } else if (value.contains("Super")) {
                m_enumerateForwardKeysComboBox->comboBox()->setCurrentText("ALT_SUPER");
            }
        } else if (value.contains("Control")) {
            if (value.contains("Shift")) {
                m_enumerateForwardKeysComboBox->comboBox()->setCurrentText("CTRL_SHIFT");
            } else if (value.contains("Super")) {
                m_enumerateForwardKeysComboBox->comboBox()->setCurrentText("CTRL_SUPER");
            }
        }
    }

    {
        // 只需要第一组快捷键
        auto value =
            fcitx::kcm::readVariant(m_configProxy->value(), "Hotkey/TriggerKeys/0").toString();
        m_defaultIMKey->setList(fcitx::Key(value.toStdString()));
    }

    connect(m_defaultIMKey, &FcitxKeySettingsItem::editedFinish, [=]() {
        auto triggerKey = m_defaultIMKey->getKeyToStr();

        auto value = m_configProxy->value().toMap();
        fcitx::kcm::writeVariant(value, "Hotkey/TriggerKeys/0", triggerKey);
        m_configProxy->setValue(value);
        m_configProxy->save();
    });

    connect(m_enumerateForwardKeysComboBox->comboBox(), &QComboBox::currentTextChanged, [=]() {
        m_enumerateForwardKeysComboBox->comboBox()->setAccessibleName(
            m_enumerateForwardKeysComboBox->comboBox()->currentText());

        auto enumerateForwardKeys = m_enumerateForwardKeysComboBox->comboBox()->currentText();

        std::tuple<QString, QString> enumerateForwardKeysTuple;
        if (enumerateForwardKeys == "NONE") {
        } else if (enumerateForwardKeys == "CTRL_SHIFT") {
            enumerateForwardKeysTuple = { "Control+Shift+Shift_L", "Control+Shift+Shift_R" };
        } else if (enumerateForwardKeys == "ALT_SHIFT") {
            enumerateForwardKeysTuple = { "Alt+Shift+Shift_R", "Alt+Shift+Shift_L" };
        } else if (enumerateForwardKeys == "CTRL_SUPER") {
            enumerateForwardKeysTuple = { "Control+Super+Control_L", "Control+Super+Control_R" };
        } else if (enumerateForwardKeys == "ALT_SUPER") {
            enumerateForwardKeysTuple = { "Alt+Super+Alt_L", "Alt+Super+Alt_R" };
        }

        auto value = m_configProxy->value().toMap();
        fcitx::kcm::writeVariant(value,
                                 "Hotkey/EnumerateForwardKeys/0",
                                 std::get<0>(enumerateForwardKeysTuple));
        fcitx::kcm::writeVariant(value,
                                 "Hotkey/EnumerateForwardKeys/1",
                                 std::get<1>(enumerateForwardKeysTuple));
        m_configProxy->setValue(value);
        m_configProxy->save();
    });

    connect(m_resetBtn, &QPushButton::clicked, [=]() {
        m_enumerateForwardKeysComboBox->comboBox()->setCurrentText("CTRL_SHIFT");
        m_defaultIMKey->setList(fcitx::Key("CTRL_SPACE"));
        std::tuple<QString, QString> enumerateForwardKeysTuple = { "Control+Shift+Shift_L",
                                                                   "Control+Shift+Shift_R" };
        auto value = m_configProxy->value().toMap();
        fcitx::kcm::writeVariant(value,
                                 "Hotkey/EnumerateForwardKeys/0",
                                 std::get<0>(enumerateForwardKeysTuple));
        fcitx::kcm::writeVariant(value,
                                 "Hotkey/EnumerateForwardKeys/1",
                                 std::get<1>(enumerateForwardKeysTuple));
        fcitx::kcm::writeVariant(value,
                                 "Hotkey/TriggerKeys/0",
                                 QString::fromStdString(fcitx::Key("CTRL_SPACE").toString()));
        m_configProxy->setValue(value);
        m_configProxy->save();
    });

    connect(m_advSetKey, &QPushButton::clicked, this, [this]() {
        m_advSetKey->setEnabled(false);
        auto *advancedSettingProcess = new QProcess(this);
        advancedSettingProcess->setProgram("fcitx5-config-qt");
        advancedSettingProcess->start();
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        auto finished = QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished);
#else
        auto finished = QOverload<int>::of(&QProcess::finished);
#endif
        auto conn = connect(advancedSettingProcess, finished, this, [this]() {
            m_advSetKey->setEnabled(true);
        });

        connect(this, &QObject::destroyed, [conn]() {
            disconnect(conn);
        });
    });
}

bool IMSettingWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_IMListGroup) {
        switch (event->type()) {
        case QEvent::HoverLeave:
            m_hoveredRow = -1;
            updateActions();
            break;
        case QEvent::HoverMove: {
            auto *he = dynamic_cast<QHoverEvent *>(event);
            int newRow = m_IMListGroup->indexAt(he->pos()).row();
            if (newRow != m_hoveredRow) {
                m_hoveredRow = newRow;
                updateActions();
            }

            break;
        }
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void IMSettingWindow::updateActions()
{
    auto selections = m_IMListGroup->selectionModel()->selectedIndexes();

    for (int i = 0; i < m_IMListModel->rowCount(); ++i) {
        auto *item = dynamic_cast<DStandardItem *>(m_IMListModel->item(i));
        auto actions = item->actionList(Qt::Edge::RightEdge);
        if (m_hoveredRow == i || selections.contains(item->index())) {
            for (auto *action : actions) {
                action->setVisible(true);
            }
        } else {
            for (auto *action : actions) {
                action->setVisible(false);
            }
        }
    }
}

// 当前输入法列表改变
void IMSettingWindow::onCurIMChanged(fcitx::kcm::FilteredIMModel *model)
{
    m_deleteBtn->setEnabled(false);
    m_IMListModel->clear();

    for (int i = 0; i < model->rowCount(); ++i) {
        QString name = model->index(i).data(Qt::DisplayRole).toString();
        DStandardItem *item = new DStandardItem(name);

        auto *upAction = new DViewItemAction(Qt::AlignVCenter, QSize(), QSize(), true);
        upAction->setIcon(QIcon::fromTheme("arrow_up"));
        upAction->setVisible(false);
        upAction->setDisabled(i == 0);

        auto *downAction = new DViewItemAction(Qt::AlignVCenter, QSize(), QSize(), true);
        downAction->setIcon(QIcon::fromTheme("arrow_down"));
        downAction->setVisible(false);
        downAction->setDisabled(i == model->rowCount() - 1);

        auto *configAction = new DViewItemAction(Qt::AlignVCenter, QSize(), QSize(), true);
        configAction->setIcon(QIcon::fromTheme("setting"));
        configAction->setVisible(false);

        item->setActionList(Qt::Edge::RightEdge, { upAction, downAction, configAction });

        connect(upAction, &DViewItemAction::triggered, this, [this, i]() {
            onItemUp(i);
        });

        connect(downAction, &DViewItemAction::triggered, this, [this, i]() {
            onItemDown(i);
        });

        connect(configAction, &DViewItemAction::triggered, this, [this, i]() {
            onItemConfig(i);
        });

        m_IMListModel->appendRow(item);
        qInfo() << "manager im changed:" << name;
    }
}

void IMSettingWindow::onItemUp(int row)
{
    m_config->move(row, row - 1);
    m_config->save();
    m_IMListGroup->setCurrentIndex(m_IMListModel->index(row - 1, 0));
}

void IMSettingWindow::onItemDown(int row)
{
    m_config->move(row, row + 1);
    m_config->save();
    m_IMListGroup->setCurrentIndex(m_IMListModel->index(row + 1, 0));
}

void IMSettingWindow::onItemConfig(int row)
{
    auto item = m_config->currentIMModel()->index(row);
    QString uniqueName = item.data(fcitx::kcm::FcitxIMUniqueNameRole).toString();

    if (uniqueName == SOGOU_IM_UNIQUE_NAME) {
        DDBusSender()
            .service("org.desktopspec.ApplicationManager1")
            .path(QStringLiteral("/org/desktopspec/ApplicationManager1/") + DUtil::escapeToObjectPath(SOGOU_CONFIGURE_APP_ID))
            .interface("org.desktopspec.ApplicationManager1.Application")
            .method("Launch")
            .arg(QString(""))
            .arg(QStringList())
            .arg(QVariantMap())
            .call();
        return;
    }

    QString title = item.data(Qt::DisplayRole).toString();
    QPointer<QDialog> dialog = fcitx::kcm::ConfigWidget::configDialog(
        this,
        m_dbus,
        QString("fcitx://config/inputmethod/%1").arg(uniqueName),
        title);
    dialog->exec();
    delete dialog;
}

void IMSettingWindow::onItemDelete(int row)
{
    m_IMListModel->removeRow(row);
    m_config->removeIM(row);
    m_config->save();

    int newSelectedRow = row >= m_IMListModel->rowCount() ? row - 1 : row;
    m_IMListGroup->setCurrentIndex(m_IMListModel->index(newSelectedRow, 0));
}

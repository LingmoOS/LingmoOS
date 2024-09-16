// SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "addimpage.h"

#include <fcitxqtcontrollerproxy.h>
#include <model.h>
#include <widgets/buttontuple.h>

#include <DDBusSender>
#include <DDialog>
#include <DFrame>
#include <DGuiApplicationHelper>
#include <DPaletteHelper>
#include <DStyle>
#include <DTableView>
#include <DTitlebar>
#include <DUtil>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace Dtk::Gui;
using namespace DCC_NAMESPACE;

class AvailItemDelegate : public DStyledItemDelegate
{
public:
    using DStyledItemDelegate::DStyledItemDelegate;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt(option);
        // 对选中项增加 checked 样式，删除选中样式，并增加鼠标悬停样式
        opt.features |= QStyleOptionViewItem::HasCheckIndicator;
        if (opt.state & QStyle::State_Selected) {
            opt.checkState = Qt::Checked;
            opt.state &= ~QStyle::State_Selected;
            opt.state |= QStyle::State_MouseOver;
        }

        DStyledItemDelegate::paint(painter, opt, index);

        int row = index.row();
        QModelIndex next = index.model()->index(row + 1, 0);
        if (Q_LIKELY(m_line) && index.data(fcitx::kcm::FcitxIMEnabledRole).toBool()
            && !next.data(fcitx::kcm::FcitxIMEnabledRole).toBool()) {
            const DPalette &dp = DPaletteHelper::instance()->palette(m_line);
            const QColor &outlineColor = dp.frameBorder().color();
            QPoint start(opt.rect.left(), opt.rect.bottom() - (margins().bottom() / 2));
            painter->fillRect(QRect(start, QSize(opt.rect.width(), m_line->lineWidth())),
                              outlineColor);
        }
    }

    inline void setLineSplitter(QFrame *line) { m_line = line; }

private:
    QPointer<QFrame> m_line;
};

AddIMPage::AddIMPage(fcitx::kcm::DBusProvider *dbus, fcitx::kcm::IMConfig *config, QWidget *parent)
    : DAbstractDialog(parent)
    , m_dbus(dbus)
    , m_config(config)
{
    setFixedSize(690, 620);

    auto *wrapperLayout = new QVBoxLayout(this);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(wrapperLayout);

    DTitlebar *titleIcon = new DTitlebar();
    titleIcon->setFrameStyle(QFrame::NoFrame); // 无边框
    titleIcon->setBackgroundTransparent(true); // 透明
    titleIcon->setMenuVisible(false);
    titleIcon->setTitle(tr("Select your language and add input methods"));
    titleIcon->setIcon(QIcon(":/img/title_img.png"));
    wrapperLayout->addWidget(titleIcon);

    auto *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(20, 20, 20, 10);
    mainLayout->setSpacing(10);
    wrapperLayout->addLayout(mainLayout);

    m_contentLayout = new QHBoxLayout();
    mainLayout->addLayout(m_contentLayout);

    auto *left = new DFrame(this);
    left->setFixedWidth(290);

    m_contentLayout->addWidget(left);

    m_leftLayout = new QVBoxLayout();
    m_leftLayout->setSpacing(10);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);
    left->setLayout(m_leftLayout);

    m_searchEdit = new Dtk::Widget::DSearchEdit();
    m_leftLayout->addWidget(m_searchEdit);

    line = new DHorizontalLine(this);
    m_leftLayout->addWidget(line);

    m_availIMList = new DListView(this);
    auto availIMDelegate = new AvailItemDelegate(m_availIMList);
    availIMDelegate->setLineSplitter(line);
    m_availIMList->setItemDelegate(availIMDelegate);
    m_availIMList->setModel(m_config->availIMModel());
    m_availIMList->setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);
    m_availIMList->setFocusPolicy(Qt::NoFocus);
    m_availIMList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_leftLayout->addWidget(m_availIMList);

    m_rightLayout = new QVBoxLayout();
    m_rightLayout->setSpacing(10);
    m_contentLayout->addLayout(m_rightLayout);

    DFrame *rightListFrame = new DFrame(this);
    rightListFrame->setFixedWidth(350);
    QVBoxLayout *rightListFrameLayout = new QVBoxLayout();
    rightListFrame->setLayout(rightListFrameLayout);
    m_rightLayout->addWidget(rightListFrame);

    m_childIMList = new DListView(this);
    m_childIMList->setModel(m_config->availIMModel());
    m_childIMList->setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);
    m_childIMList->setFocusPolicy(Qt::NoFocus);
    m_childIMList->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    m_childIMList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightListFrameLayout->addWidget(m_childIMList);

    m_laSelector = new LayoutWidget(this);
    m_laSelector->setFixedSize(350, 140);
    m_rightLayout->addWidget(m_laSelector);

    auto *findMoreLayout = new QHBoxLayout(this);
    mainLayout->addLayout(findMoreLayout);

    m_findMoreLabel = new DCommandLinkButton(tr("Find more in App Store"), this);
    m_findMoreLabel->setAccessibleName("Find more in App Store");
    findMoreLayout->addWidget(m_findMoreLabel);
    findMoreLayout->addStretch(1);

    mainLayout->addSpacing(9);

    m_buttonTuple = new ButtonTuple(ButtonTuple::Save, this);
    mainLayout->addWidget(m_buttonTuple);

    QPushButton *cancel = m_buttonTuple->leftButton();
    cancel->setText(tr("Cancel"));
    cancel->setObjectName("Cancel");
    QPushButton *ok = m_buttonTuple->rightButton();
    ok->setText(tr("Add"));
    ok->setEnabled(false);

    connect(m_config, &fcitx::kcm::IMConfig::changed, this, &AddIMPage::changed);

    connect(m_searchEdit,
            &Dtk::Widget::DSearchEdit::textChanged,
            m_config->availIMModel(),
            &fcitx::kcm::IMProxyModel::setFilterText);

    connect(m_availIMList->model(), &QAbstractItemModel::layoutChanged, this, [this]() {
        m_availIMList->setCurrentIndex(m_availIMList->model()->index(0, 0));
    });
    connect(m_availIMList->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            &AddIMPage::availIMCurrentChanged,
            Qt::QueuedConnection);
    connect(m_childIMList->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &AddIMPage::childIMSelectionChanged);

    connect(m_findMoreLabel, &DCommandLinkButton::clicked, this, &AddIMPage::clickedFindMoreButton);

    connect(cancel, &QPushButton::clicked, this, &AddIMPage::clickedCloseButton);
    connect(ok, &QPushButton::clicked, this, &AddIMPage::clickedAddButton);

    m_availIMList->setCurrentIndex(m_availIMList->model()->index(0, 0));
}

AddIMPage::~AddIMPage()
{
    m_config->availIMModel()->setFilterText("");
}

void AddIMPage::save()
{
    QItemSelectionModel *selections = m_childIMList->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    m_config->addIMs(selected);
    m_config->save();
}

void AddIMPage::load()
{
    m_config->load();
}

void AddIMPage::defaults() { }

static std::tuple<QString, QString> getLayoutString(const QString &uniqName)
{
    QString layout = "";
    QString variant = "";
    if (uniqName.startsWith("keyboard-")) {
        // layout, name likes keyboard-xx-xxx, such as
        // keyboard-cn-mon_trad_manchu
        auto layoutPos = uniqName.indexOf("-") + 1;
        auto secondDashPos = uniqName.indexOf("-", layoutPos);
        if (secondDashPos > 0) {
            int variantPos = secondDashPos + 1;
            layout = uniqName.mid(layoutPos, secondDashPos - layoutPos);
            variant = uniqName.mid(variantPos);
        } else {
            layout = uniqName.mid(layoutPos);
        }
    }
    return { layout, variant };
}

void AddIMPage::availIMCurrentChanged(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto firstChild = index.model()->index(0, 0, index);
    if (!firstChild.isValid()) {
        return;
    }

    // workaround: index 与 child 的 parent 不同
    m_childIMList->setRootIndex(firstChild.parent());

    // 默认选择第一个未被启用的输入法，当不存在未被启用的输入法时，选择空
    // index（清空选择）
    QModelIndex defaultChild;
    for (int i = 0; i < index.model()->rowCount(index); i++) {
        auto item = index.model()->index(i, 0, index);
        bool imEnabled = item.data(fcitx::kcm::FcitxIMEnabledRole).toBool();
        if (!imEnabled) {
            defaultChild = item;
            break;
        }
    }
    m_childIMList->setCurrentIndex(defaultChild);
}

void AddIMPage::childIMSelectionChanged()
{
    auto selected = m_childIMList->selectionModel()->selectedIndexes();

    m_buttonTuple->rightButton()->setEnabled(!selected.isEmpty());

    if (selected.isEmpty()) {
        m_laSelector->showNoLayout();
        return;
    }

    if (selected.count() > 1) {
        m_laSelector->showMulti();
        return;
    }

    QModelIndex i = selected.at(0);
    QString uniqueName = i.data(fcitx::kcm::FcitxIMUniqueNameRole).toString();

    QString layout;
    QString variant;
    std::tie(layout, variant) = getLayoutString(uniqueName);
    m_laSelector->setKeyboardLayout(layout, variant);
}

void AddIMPage::clickedFindMoreButton()
{
    DDBusSender()
        .service("org.desktopspec.ApplicationManager1")
        .path(QStringLiteral("/org/desktopspec/ApplicationManager1/") + DUtil::escapeToObjectPath("deepin-app-store"))
        .interface("org.desktopspec.ApplicationManager1.Application")
        .method("Launch")
        .arg(QString(""))
        .arg(QStringList())
        .arg(QVariantMap())
        .call();
}

void AddIMPage::clickedCloseButton()
{
    close();
    deleteLater();
}

void AddIMPage::clickedAddButton()
{
    save();

    close();
    deleteLater();
}

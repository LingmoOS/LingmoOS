// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchwidget.h"
#include "aminterface.h"
#include <DDBusSender>

#include <QHBoxLayout>
#include <QEvent>
#include <QAction>
#include <QDebug>
#include <QKeyEvent>

#define ICON_SIZE   24
#define BTN_SIZE    40

#define SEARCHEIT_WIDTH 350
#define SEARCHEIT_HEIGHT 40

using namespace DLauncher;

/**
 * @brief SearchWidget::SearchWidget 顶部搜索控件
 * @param parent
 */
SearchWidget::SearchWidget(QWidget *parent)
    : QFrame(parent)
    , m_enableUpdateMode(true)
{
    setAccessibleName("From_Search");
    m_leftSpacing = new QFrame(this);
    m_rightSpacing = new QFrame(this);

    m_calcUtil = CalculateUtil::instance();

    m_leftSpacing->setFixedWidth(0);
    m_rightSpacing->setFixedWidth(0);
    m_leftSpacing->setAccessibleName("LeftSpacing");
    m_rightSpacing->setAccessibleName("RightSpacing");

    m_toggleModeBtn = new DIconButton(this);
    m_toggleModeBtn->setIcon(DDciIcon::fromTheme("exit_fullscreen"));
    m_toggleModeBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_toggleModeBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    m_toggleModeBtn->setAutoExclusive(true);
    m_toggleModeBtn->setBackgroundRole(DPalette::Button);
    m_toggleModeBtn->setAccessibleName("Btn-ToggleMode");

    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->setAccessibleName("search-edit");

    // 添加launcher搜索框图标
    QAction *leftaction = m_searchEdit->findChild<QAction *>("_d_search_leftAction");
    if (leftaction)
        leftaction->setIcon(QIcon(":/icons/skin/icons/search_36px.svg"));

    QAction *clearAction = m_searchEdit->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"));
    if (clearAction)
        clearAction->setIcon(QIcon(":/icons/skin/icons/clear_36px.svg"));

    m_searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_searchEdit->lineEdit()->setFixedSize(SEARCHEIT_WIDTH, SEARCHEIT_HEIGHT);
    this->setMouseTracking(true);
    DStyle::setFocusRectVisible(m_searchEdit->lineEdit(), true);
    setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addSpacing(30);
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();
    mainLayout->addWidget(m_toggleModeBtn);
    mainLayout->addWidget(m_rightSpacing);
    mainLayout->addSpacing(30);

    setLayout(mainLayout);

    connect(m_searchEdit, &DSearchEdit::textChanged, this, &SearchWidget::onTextChanged);
    connect(m_toggleModeBtn, &DIconButton::clicked, this, &SearchWidget::onModeClicked);
}

DSearchEdit *SearchWidget::edit()
{
    return m_searchEdit;
}

DIconButton *SearchWidget::toggleModeBtn()
{
    return m_toggleModeBtn;
}

void SearchWidget::clearSearchContent()
{
    m_searchEdit->clear();
    m_searchEdit->clearEdit();
}

void SearchWidget::onTextChanged(const QString &str)
{
    Q_UNUSED(str);

    m_searchEdit->lineEdit()->setFocus();
    auto searchStr = m_searchEdit->text();

    emit searchTextChanged(searchStr.replace(" ", ""), m_enableUpdateMode);
}

void SearchWidget::onModeClicked()
{
    emit toggleMode();
    emit m_calcUtil->loadWindowIcon();
    m_calcUtil->setFullScreen(false);
    if (AMInter::isAMReborn()) {
        AMInter::instance()->setFullScreen(false);
    } else {
        DDBusSender()
            .service(DBUS_DAEMON_SERVICE_NAME)
            .interface(DBUS_DAEMON_SERVICE_NAME)
            .path(DBUS_DAEMON_PATH_NAME)
            .property("Fullscreen")
            .set(false);
    }
}

void SearchWidget::onToggleCategoryChanged()
{
    m_enableUpdateMode = false;
    m_searchEdit->lineEdit()->clearFocus();
    clearSearchContent();
    m_enableUpdateMode = true;

    emit toggleMode();
}

void SearchWidget::setLeftSpacing(int spacing)
{
    m_leftSpacing->setFixedWidth(spacing);
}

void SearchWidget::setRightSpacing(int spacing)
{
    m_rightSpacing->setFixedWidth(spacing);
}

void SearchWidget::showToggle()
{
    m_toggleModeBtn->show();
}

void SearchWidget::hideToggle()
{
    m_toggleModeBtn->hide();
}

/**
 * @brief SearchWidget::updateSize 根据屏幕比例(相较于1920*1080分辨率的屏幕)设置控件大小
 * @param scaleX 屏幕宽度比
 * @param scaleY 屏幕高度比
 */
void SearchWidget::updateSize(double scaleX, double scaleY)
{
    m_searchEdit->lineEdit()->setFixedSize(int(SEARCHEIT_WIDTH * scaleX), int(SEARCHEIT_HEIGHT * scaleY));
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;

    m_toggleModeBtn->setIconSize(QSize(int(ICON_SIZE * scale), int(ICON_SIZE * scale)));
    m_toggleModeBtn->setFixedSize(int(BTN_SIZE * scale), int(BTN_SIZE * scale));
}

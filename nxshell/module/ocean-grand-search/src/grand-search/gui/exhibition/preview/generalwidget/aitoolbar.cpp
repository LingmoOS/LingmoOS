// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aitoolbar.h"
#include "linkbutton.h"
#include "../../exhibitionwidget.h"

#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DCommandLinkButton>
#include <DDialog>
#include <DNotifySender>
#include <DApplication>

#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QDBusInterface>
#include <QDBusReply>
#include <QVariantMap>
#include <QtDBus>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace GrandSearch;

bool AiToolBar::checkUosAiInstalled() {
    QDBusInterface iface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    QDBusReply<QStringList> reply = iface.call("ListActivatableNames");
    if (reply.isValid())
        return reply.value().contains("com.lingmo.copilot");

    return false;
}

void AiToolBar::showWarningDialog(QString name) {
    DDialog *warningDlg = new DDialog();
    warningDlg->setWindowFlags((warningDlg->windowFlags() | Qt::WindowType::WindowStaysOnTopHint));
    warningDlg->setFixedWidth(380);
    warningDlg->setIcon(QIcon(":icons/ocean-grand-search-setting.svg"));
    warningDlg->setMessage(QString(tr("Unable to use %1, please go to the App Store to update the UOS AI version first.").arg(name)));
    warningDlg->addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    warningDlg->addButton(tr("Open App Stroe"), true, DDialog::ButtonRecommend);
    connect(warningDlg, &DDialog::accepted, warningDlg, [&] {
        QDBusInterface iface("com.home.appstore.client", "/com/home/appstore/client", "com.home.appstore.client");
        iface.call("openBusinessUri", QString("app_detail_info/uos-ai"));
    });
    connect(warningDlg, &DDialog::finished, warningDlg, [&] {
        emit warningDlg->closed();
    });
    warningDlg->moveToCenter();
    warningDlg->show();
}

AiToolBar::AiToolBar(QWidget *parent) : DWidget(parent) {
    m_inner = new AiToolBarInner(this);
    this->initUi();
}

AiToolBar::~AiToolBar() {
    if (m_inner) {
        delete m_inner;
    }
}

void AiToolBar::initUi() {
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_mainLayout = new QHBoxLayout(this);
    m_left  = 10;
    m_right = 10;
    m_mainLayout->setContentsMargins(m_left, m_top, m_right, 0);
    m_mainLayout->addWidget(m_inner);
    this->setLayout(m_mainLayout);
}

void AiToolBar::setFilePath(QString filePath) {
    m_inner->setFilePath(filePath);
}

AiToolBarInner::AiToolBarInner(QWidget *parent) : DWidget(parent) {
    this->initUi();
    this->initConnect();
}

void AiToolBarInner::initUi() {
    this->setFixedSize(360, 36);

    m_iconBt = new TransButton(this);
    m_iconBt->setIcon(QIcon(":/icons/uos-ai-assistant.svg"));
    m_iconBt->setIconSize(QSize(16, 16));
    m_iconBt->setIconNoPressColor();

    m_omitBt = new OmitButton(this);
    m_omitBt->setIconSize(QSize(20, 20));

    m_summaryBt     = new LinkButton(tr("Summary"), this);
    m_translationBt = new LinkButton(tr("Translation"), this);
    m_extensionBt   = new LinkButton(tr("Extension"), this);
    m_knowledgeBt   = new LinkButton(tr("Add to knowledge base"), this);
    DFontSizeManager::instance()->bind(m_summaryBt, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(m_translationBt, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(m_extensionBt, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(m_knowledgeBt, DFontSizeManager::T8, QFont::Normal);

    m_btList.append(m_summaryBt);
    m_btList.append(m_translationBt);
    m_btList.append(m_extensionBt);
    m_btList.append(m_knowledgeBt);

    m_summaryBt->setToolTip(tr("Summarize document content with UOS AI"));
    m_translationBt->setToolTip(tr("Translate document content with UOS AI"));
    m_extensionBt->setToolTip(tr("Extend document content with UOS AI"));
    m_knowledgeBt->setToolTip(tr("Add document to UOS AI knowledge base"));

    m_summarySpace     = new DWidget(this);
    m_translationSpace = new DWidget(this);
    m_extensionSpace   = new DWidget(this);
    m_knowledgeSpace   = new DWidget(this);
    m_translationSpace->setFixedWidth(1);
    m_extensionSpace->setFixedWidth(1);
    m_knowledgeSpace->setFixedWidth(1);

    m_spaceList.append(m_summarySpace);
    m_spaceList.append(m_translationSpace);
    m_spaceList.append(m_extensionSpace);
    m_spaceList.append(m_knowledgeSpace);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 0, 8, 0);
    mainLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addWidget(m_iconBt);
    mainLayout->addWidget(m_summarySpace);
    m_summarySpace->setVisible(false);
    mainLayout->addWidget(m_summaryBt);
    mainLayout->addWidget(m_translationSpace);
    mainLayout->addWidget(m_translationBt);
    mainLayout->addWidget(m_extensionSpace);
    mainLayout->addWidget(m_extensionBt);
    mainLayout->addWidget(m_knowledgeSpace);
    mainLayout->addWidget(m_knowledgeBt);
    mainLayout->addStretch();
    mainLayout->addWidget(m_omitBt);
    this->setLayout(mainLayout);

    m_menu = new DMenu(this);
    m_summaryAction     = new QAction(tr("Summary"));
    m_translationAction = new QAction(tr("Translation"));
    m_extensionAction   = new QAction(tr("Extension"));
    m_knowledgeAction   = new QAction(tr("Add to knowledge base"));
    m_menu->addAction(m_summaryAction);
    m_menu->addAction(m_translationAction);
    m_menu->addAction(m_extensionAction);
    m_menu->addAction(m_knowledgeAction);

    m_actionList.append(m_summaryAction);
    m_actionList.append(m_translationAction);
    m_actionList.append(m_extensionAction);
    m_actionList.append(m_knowledgeAction);
}

void AiToolBarInner::initConnect() {
    connect(m_iconBt, &TransButton::clicked, this, &AiToolBarInner::onBtClicked);
    connect(m_omitBt, &OmitButton::clicked, this, &AiToolBarInner::onBtClicked);
    connect(m_summaryBt, &LinkButton::clicked, this, &AiToolBarInner::onBtClicked);
    connect(m_translationBt, &LinkButton::clicked, this, &AiToolBarInner::onBtClicked);
    connect(m_extensionBt, &LinkButton::clicked, this, &AiToolBarInner::onBtClicked);
    connect(m_knowledgeBt, &LinkButton::clicked, this, &AiToolBarInner::onBtClicked);
}

void AiToolBarInner::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, int(255 * 0.05)));

    int radius = 8;
    QRect roundRect(0, 0, radius * 2, radius * 2);
    QPainterPath path;
    int w = this->width();
    int h = this->height();

    // 左上圆角
    path.moveTo(0, radius);
    roundRect.moveTo(0, 0);
    path.arcTo(roundRect, 180, -90);

    // 右上圆角
    path.lineTo(w - radius, 0);
    roundRect.moveTo(w - roundRect.width(), 0);
    path.arcTo(roundRect, 90, -90);

    // 右下圆角
    path.lineTo(w, h - radius);
    roundRect.moveTo(w - roundRect.width(), h - roundRect.width());
    path.arcTo(roundRect, 0, -90);

    // 左下圆角
    path.lineTo(radius, h);
    roundRect.moveTo(0, h - roundRect.height());
    path.arcTo(roundRect, 270, -90);

    path.closeSubpath();
    p.drawPath(path);

    return QWidget::paintEvent(event);
}

void AiToolBarInner::showEvent(QShowEvent *event) {
    DWidget::showEvent(event);
    this->adjustBts();
}

void AiToolBarInner::onBtClicked() {
    if (sender() == m_iconBt) {
        this->onOpenUosAi();
        return;
    }

    if (sender() == m_omitBt) {
        m_menu->move(QCursor::pos());
        m_menu->show();
        return;
    }

    if (sender() == m_summaryBt) {
        this->onSummary();
        return;
    }

    if (sender() == m_translationBt) {
        this->onTranslation();
        return;
    }

    if (sender() == m_extensionBt) {
        this->onExtension();
        return;
    }

    if (sender() == m_knowledgeBt) {
        this->onKnowledge();
        return;
    }
}

void AiToolBarInner::onMenuTriggered(QAction *action) {
    if (action == m_summaryAction) {
        this->onSummary();
        return;
    }

    if (action == m_translationAction) {
        this->onTranslation();
        return;
    }

    if (action == m_extensionAction) {
        this->onExtension();
        return;
    }

    if (action == m_knowledgeAction) {
        this->onKnowledge();
        return;
    }
}

void AiToolBarInner::onOpenUosAi() {
    QDBusInterface notification("com.lingmo.copilot", "/com/lingmo/copilot", "com.lingmo.copilot", QDBusConnection::sessionBus());
    notification.call(QDBus::Block, "launchChatPage");
}

void AiToolBarInner::onSummary() {
    qDBusRegisterMetaType<QMap<QString, QString>>();
    QMap<QString, QString> params;
    params.insert("file", m_filePath);
    params.insert("defaultPrompt", "帮我总结这篇文档，直接给我清晰的结果，不用多余内容");
    QDBusInterface notification("com.lingmo.copilot", "/org/lingmo/copilot/chat", "org.lingmo.copilot.chat", QDBusConnection::sessionBus());
    QString error = notification.call(QDBus::Block, "inputPrompt", "", QVariant::fromValue(params)).errorMessage();
    if (!error.isEmpty()) {
        qWarning() << QString("inputPrompt ERROR(%1)").arg(error);
        this->showWarningDialog(m_summaryAction->text());
    } else {
        this->closeMainWindow();
    }
}

void AiToolBarInner::onTranslation() {
    qDBusRegisterMetaType<QMap<QString, QString>>();
    QMap<QString, QString> params;
    params.insert("file", m_filePath);
    params.insert("defaultPrompt", "帮我翻译这篇文档，直接给我清晰的结果，不用多余内容");
    QDBusInterface notification("com.lingmo.copilot", "/org/lingmo/copilot/chat", "org.lingmo.copilot.chat", QDBusConnection::sessionBus());
    QString error = notification.call(QDBus::Block, "inputPrompt", "", QVariant::fromValue(params)).errorMessage();
    if (!error.isEmpty()) {
        qWarning() << QString("inputPrompt ERROR(%1)").arg(error);
        this->showWarningDialog(m_translationAction->text());
    } else {
        this->closeMainWindow();
    }
}

void AiToolBarInner::onExtension() {
    qDBusRegisterMetaType<QMap<QString, QString>>();
    QMap<QString, QString> params;
    params.insert("file", m_filePath);
    params.insert("defaultPrompt", "帮我扩写这篇文档，直接给我清晰的结果，不用多余内容");
    QDBusInterface notification("com.lingmo.copilot", "/org/lingmo/copilot/chat", "org.lingmo.copilot.chat", QDBusConnection::sessionBus());
    QString error = notification.call(QDBus::Block, "inputPrompt", "", QVariant::fromValue(params)).errorMessage();
    if (!error.isEmpty()) {
        qWarning() << QString("inputPrompt ERROR(%1)").arg(error);
        this->showWarningDialog(m_extensionAction->text());
    } else {
        this->closeMainWindow();
    }
}

void AiToolBarInner::onKnowledge() {
    QDBusInterface notification("com.lingmo.copilot", "/org/lingmo/copilot/chat", "org.lingmo.copilot.chat", QDBusConnection::sessionBus());
    QString error = notification.call(QDBus::Block, "sendToKnowledgeBase", QStringList(m_filePath)).errorMessage();
    if (!error.isEmpty()) {
        qWarning() << QString("sendToKnowledgeBase ERROR(%1)").arg(error);
        this->showWarningDialog(m_knowledgeAction->text());
    } else {
        this->closeMainWindow();
    }
}

void AiToolBarInner::adjustBts() {
    const int MAX_WIDTH  = this->width() - 15 - 10 - m_iconBt->sizeHint().width() - 4;
    const int MAX_WIDTH1 = this->width() - 15 - 10 - m_iconBt->sizeHint().width() - 4 - m_omitBt->sizeHint().width();

    m_knowledgeBt->setText(m_knowledgeAction->text());
    int sumWidth = 0;
    for (int i = 0; i < m_btList.size(); i++) {
        m_btList[i]->setVisible(true);
        sumWidth += m_btList[i]->sizeHint().width() + 4;
    }
    for (int i = 1; i < m_spaceList.size(); i++) {
        m_spaceList[i]->setVisible(true);
        sumWidth += 5;
    }
    for (int i = 0; i < m_actionList.size(); i++) {
        m_actionList[i]->setVisible(false);
    }
    m_omitBt->setVisible(false);

    if (sumWidth <= MAX_WIDTH) {
        return;
    }

    m_omitBt->setVisible(true);
    // 1、确定哪些显示在工具栏，哪些放入菜单
    int actionCounts = 0;
    for (int i = m_btList.size() - 1; i >= 0; i++) {
        sumWidth -= m_btList[i]->sizeHint().width() + 4;
        sumWidth -= 5;
        m_btList[i]->setVisible(false);
        m_spaceList[i]->setVisible(false);
        m_actionList[i]->setVisible(true);
        actionCounts++;
        if (sumWidth <= MAX_WIDTH1) {
            break;
        }
    }

    // 2、如果菜单中只有一项，看能不能显示2个字或更多在工具栏上而不使用菜单（…）
    if (actionCounts == 1) {
        QFontMetrics fm(m_knowledgeBt->font());
        QString displayStr = m_knowledgeBt->text().left(2) + "…";
        if (fm.width(displayStr) + 2 + 5 < MAX_WIDTH - sumWidth) {
            for (int i = 3; i < m_knowledgeBt->text().length(); i++) {
                displayStr = m_knowledgeBt->text().left(i) + "…";
                if (!(fm.width(displayStr) + 2 + 5 < MAX_WIDTH - sumWidth)) {
                    displayStr = m_knowledgeBt->text().left(i - 1) + "…";
                    break;
                }
            }
            m_knowledgeBt->setText(displayStr);
            m_knowledgeBt->setVisible(true);
            m_knowledgeSpace->setVisible(true);
            m_knowledgeAction->setVisible(false);
            m_omitBt->setVisible(false);
        }
    }
}

void AiToolBarInner::showWarningDialog(QString name) {
    QString tmpPath = QString("/tmp/%1_notify.png").arg(DApplication::instance()->applicationName());
    QIcon tmpIcon(":icons/ocean-grand-search-setting.svg");
    QPixmap tmpPixmap = tmpIcon.pixmap(QSize(32, 32));
    bool isSaved = tmpPixmap.save(tmpPath);
    qDebug() << "isSaved:" << isSaved << "path:" << tmpPath;

    DUtil::DNotifySender notify(tr("OCEAN Grand Search"));
    notify.appName(DApplication::instance()->applicationName());
    notify.appIcon(tmpPath);
    notify.appBody(QString(tr("Unable to use %1, please go to the App Store to update the UOS AI version first.").arg(name)));
    notify.actions(QStringList() << "_open" << tr("Open App Stroe"));
    QVariantMap hints;
    hints["x-lingmo-action-_open"] = QString("dbus-send,--print-reply,--dest=com.home.appstore.client,/com/home/appstore/client,com.home.appstore.client.openBusinessUri,string:app_detail_info/uos-ai"); // 通过命令定位到应用商店uos-ai页
    notify.hints(hints);
    notify.timeOut(3000);
    notify.call();
}

void AiToolBarInner::closeMainWindow() {
    if (this->parent()->parent() && this->parent()->parent()->parent()) {
        ExhibitionWidget *ptr = dynamic_cast<ExhibitionWidget *>(this->parent()->parent()->parent());
        if (ptr) {
            emit ptr->sigCloseWindow();
        }
    }
}

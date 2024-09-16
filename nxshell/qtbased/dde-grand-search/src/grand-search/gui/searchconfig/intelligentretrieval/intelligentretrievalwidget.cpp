// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "intelligentretrievalwidget.h"
#include "detailcheckbox.h"
#include "autoindexstatus.h"
#include "gui/searchconfig/switchwidget/switchwidget.h"
#include "business/config/searchconfig.h"
#include "global/searchconfigdefine.h"
#include "global/builtinsearch.h"

#include <DFontSizeManager>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

#define INER_APP_NAME "dde-grand-search"
static QDBusMessage createAnalayzeMsg(const QString &method) {
    QDBusMessage msg = QDBusMessage::createMethodCall("org.deepin.ai.daemon.AnalyzeServer", "/org/deepin/ai/daemon/AnalyzeServer",
                                                      "org.deepin.ai.daemon.AnalyzeServer", method);
    return msg;
}

static QDBusMessage createVectorMsg(const QString &method) {
    QDBusMessage msg = QDBusMessage::createMethodCall("org.deepin.ai.daemon.VectorIndex", "/org/deepin/ai/daemon/VectorIndex",
                                                      "org.deepin.ai.daemon.VectorIndex", method);
    return msg;
}

IntelligentRetrievalWidget::IntelligentRetrievalWidget(QWidget *parent)
    : DWidget(parent)
{

    m_mainLayout = new QVBoxLayout();
    setLayout(m_mainLayout);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(1);

    m_groupLabel = new QLabel(tr("Smart Search"), this);
    m_groupLabel->adjustSize();

    m_mainLayout->addWidget(m_groupLabel);
    m_mainLayout->addSpacing(9); // 1 + 9

    {
        RoundedBackground *bkg = new RoundedBackground(this);
        bkg->setFixedSize(SWITCHWIDGETWIDTH, SWITCHWIDGETHEIGHT);
        bkg->setTopRound(true);
        QVBoxLayout *vl = new QVBoxLayout(bkg);
        vl->setContentsMargins(10, 0, 10, 0);
        bkg->setLayout(vl);

        QLabel *subLabel = new QLabel(tr("Enable AI Smart Search"), bkg);
        vl->addWidget(subLabel);
        m_mainLayout->addWidget(bkg);
    }

    m_semantic = new DetailCheckBox(tr("Intelligent semantic search"), this);
    m_semantic->setMinimumSize(CHECKBOXITEMWIDTH, DETAILCHECKBOXITEMHEIGHT);
    m_semantic->setDetail(tr("When turned on, you can try to search for local documents using natural language descriptions, such as \"last week's documents\"."));
    m_mainLayout->addWidget(m_semantic);

    // 超链接
    m_vectorDetail = new QLabel;
    m_vectorDetail->setContextMenuPolicy(Qt::NoContextMenu);
    connect(m_vectorDetail, &QLabel::linkActivated, this, &IntelligentRetrievalWidget::openAppStore);

    m_vector = new DetailCheckBox(tr("Intelligent full text search"), m_vectorDetail, this);
    m_vector->setMinimumSize(CHECKBOXITEMWIDTH, DETAILCHECKBOXITEMHEIGHT);
    m_vector->setBottomRound(true);
    m_mainLayout->addWidget(m_vector);

    {
        RoundedBackground *bkg = new RoundedBackground(this);
        bkg->setMinimumSize(CHECKBOXITEMWIDTH, SWITCHWIDGETHEIGHT);
        bkg->setTopRound(true);
        bkg->setBottomRound(true);
        QVBoxLayout *vl = new QVBoxLayout(bkg);
        vl->setContentsMargins(0, 2, 0, 5);

        m_enableIndex = new SwitchWidget(tr("Automatic index update"), this);
        m_enableIndex->setEnableBackground(false);
        m_enableIndex->setFixedSize(SWITCHWIDGETWIDTH, DETAILSWITCHWIDGETHEIGHT / 2);
        m_enableIndex->setIconEnable(false);
        vl->addWidget(m_enableIndex);

        m_indexStatus = new AutoIndexStatus(bkg);
        m_indexStatus->setMinimumSize(SWITCHWIDGETWIDTH, DETAILSWITCHWIDGETHEIGHT / 2);
        vl->addWidget(m_indexStatus);

        m_mainLayout->addSpacing(8);
        m_mainLayout->addWidget(bkg);
    }

    updateState();

    connect(&m_timer, &QTimer::timeout, this, &IntelligentRetrievalWidget::updateState);
    m_timer.start(5000);
    connect(m_semantic->checkBox(), &QCheckBox::stateChanged, this, &IntelligentRetrievalWidget::checkBoxChanged);
    connect(m_vector->checkBox(), &QCheckBox::stateChanged, this, &IntelligentRetrievalWidget::checkBoxChanged);
    connect(m_enableIndex, &SwitchWidget::checkedChanged, this, &IntelligentRetrievalWidget::checkBoxChanged);
}

void IntelligentRetrievalWidget::updateState()
{
    m_ignoreSigal = true;

    auto cfg = SearchConfig::instance();
    if (isAnalayzeSupported()) {
        bool checked = cfg->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, false).toBool();
        m_semantic->checkBox()->setChecked(checked);
        m_semantic->checkBox()->setEnabled(true);
    } else {
        m_semantic->checkBox()->setChecked(false);
        m_semantic->checkBox()->setEnabled(false);
    }

    if (isVectorSupported()) {
        bool checked = cfg->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, true).toBool();
        m_vector->checkBox()->setEnabled(true);
        m_vector->checkBox()->setChecked(checked);
        m_vector->setDetail(tr("When enabled, you can search the text of articles using disjointed and incomplete keywords."));
        if (checked) {
            m_enableIndex->setEnabled(true);
            QVariantHash status;
            bool autoidx = getIndexStatus(status);
            m_enableIndex->setChecked(autoidx);

            // update index status
            if (autoidx) {
                m_indexStatus->show();
                updateStatusContent(status);
            } else {
                m_indexStatus->hide();
            }
        } else {
            m_enableIndex->setChecked(false);
            m_enableIndex->setEnabled(false);
            m_indexStatus->hide();
        }
    } else {
        m_vector->checkBox()->setChecked(false);
        m_vector->checkBox()->setEnabled(false);
        m_vector->setDetail(tr("Please install %0 in <a href=\"%0\">the app store</a> before turning on this configuration.")
                            .arg("向量化模型"));

        m_enableIndex->setChecked(false);
        m_enableIndex->setEnabled(false);
        m_indexStatus->hide();
    }
    m_ignoreSigal = false;
}

void IntelligentRetrievalWidget::openAppStore(const QString &app)
{
    qDebug() << "open app store" << app;
    QDBusMessage msg = QDBusMessage::createMethodCall("com.home.appstore.client", "/com/home/appstore/client",
                                                      "com.home.appstore.client", "openBusinessUri");
    QVariantList list;
    list.append(QString("searchApp?keyword=%0").arg(app));
    msg.setArguments(list);

    QDBusPendingReply<void> ret = QDBusConnection::sessionBus().asyncCall(msg, 100);
    ret.waitForFinished();
    if (ret.error().type() != QDBusError::NoError)
        qWarning() << "error: " << msg.service() << QDBusError::errorString(ret.error().type());

    return;
}

void IntelligentRetrievalWidget::updateStatusContent(const QVariantHash &status)
{
    bool ok = false;
    int st = status.value("completion").toInt(&ok);
    if (!ok) {
        qWarning() << "get invalid index statuas" << st;
        st = -1;
    }

    if (st == 0) {
        m_indexStatus->updateContent(AutoIndexStatus::Updating,
                    tr("Intelligent search indexing is being updated, which may take up more resources, please keep the power access."));
    } else if (st == 1) {
        auto dt = QDateTime::fromSecsSinceEpoch(status.value("updatetime").toLongLong());
        m_indexStatus->updateContent(AutoIndexStatus::Success,
                    tr("Smart Search indexing update is complete. Last update time: %0").arg(dt.toString("yyyy-MM-dd hh:mm:ss")));
    } else {
        m_indexStatus->updateContent(AutoIndexStatus::Fail,
                    tr("Indexing error!"));
    }
}

bool IntelligentRetrievalWidget::isAnalayzeSupported()
{
    auto msg = createAnalayzeMsg("Enable");
    QDBusPendingReply<bool> ret = QDBusConnection::sessionBus().asyncCall(msg, 500);
    ret.waitForFinished();
    return ret.isValid() && ret;
}

bool IntelligentRetrievalWidget::isVectorSupported()
{
    auto msg = createVectorMsg("Enable");
    QDBusPendingReply<bool> ret = QDBusConnection::sessionBus().asyncCall(msg, 500);
    ret.waitForFinished();
    return ret.isValid() && ret;
}

void IntelligentRetrievalWidget::checkBoxChanged()
{
    if (m_ignoreSigal)
        return;

    auto sd = sender();
    auto cfg = SearchConfig::instance();
    if (sd == m_semantic->checkBox()) {
        bool on = m_semantic->checkBox()->isChecked();
        cfg->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, on);

        QDBusMessage msg = createAnalayzeMsg("SetSemanticOn");
        QVariantList argvs;
        argvs.append(on);
        msg.setArguments(argvs);
        QDBusConnection::sessionBus().asyncCall(msg, 500);
    } else if (sd == m_vector->checkBox()){
        bool on = m_vector->checkBox()->isChecked();
        cfg->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, on);
        // 关闭搜索时联动关闭索引
        if (!on)
            setAutoIndex(false);
    } else if (sd == m_enableIndex) {
        setAutoIndex(m_enableIndex->checked());
    }

    updateState();
}

void IntelligentRetrievalWidget::setAutoIndex(bool on)
{
    auto msg = createVectorMsg("setAutoIndex");
    QVariantList list;
    list.append(QString(INER_APP_NAME));
    list.append(on);
    msg.setArguments(list);

    QDBusPendingReply<void> ret = QDBusConnection::sessionBus().asyncCall(msg, 500);
    ret.waitForFinished();
    if (ret.error().type() != QDBusError::NoError) {
        qWarning() << "error: " << msg.service() << QDBusError::errorString(ret.error().type());
    }
    return ;
}

bool IntelligentRetrievalWidget::getIndexStatus(QVariantHash &statuts)
{
    auto msg = createVectorMsg("getAutoIndexStatus");
    QVariantList list;
    list.append(QString(INER_APP_NAME));
    msg.setArguments(list);

    QDBusPendingReply<QString> ret = QDBusConnection::sessionBus().asyncCall(msg, 500);
    ret.waitForFinished();

    if (ret.error().type() != QDBusError::NoError) {
        qWarning() << "error: " << msg.service() << QDBusError::errorString(ret.error().type()) << ret;
        return false;
    }

    QString json = ret;
    statuts = QJsonDocument::fromJson(json.toUtf8()).object().toVariantHash();
    return statuts.value("enable", false).toBool();
}

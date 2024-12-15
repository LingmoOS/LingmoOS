// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchenginewidget.h"
#include "business/config/searchconfig.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "global/searchhelper.h"
#include "comboboxwidget/comboboxwidget.h"
#include "tipslabel.h"

#include <DWidget>
#include <DComboBox>
#include <DStyleOption>
#include <DLineEdit>

#include <QByteArray>

#define Margin  10

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace GrandSearch;

static const QHash<int, QString> searchEngineEnglish{{0, GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE}, {1, GRANDSEARCH_WEB_SEARCHENGINE_YAHOO},
                          {2, GRANDSEARCH_WEB_SEARCHENGINE_BING}, {3, GRANDSEARCH_WEB_SEARCHENGINE_BAIDU}, 
                          {4, GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM}};
static const QHash<int, QString> searchEngineChinese{{0, GRANDSEARCH_WEB_SEARCHENGINE_BAIDU}, {1, GRANDSEARCH_WEB_SEARCHENGINE_SOGOU},
                                 {2, GRANDSEARCH_WEB_SEARCHENGINE_360}, {3, GRANDSEARCH_WEB_SEARCHENGINE_360AI}, {4, GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE},
                                 {5, GRANDSEARCH_WEB_SEARCHENGINE_YAHOO}, {6, GRANDSEARCH_WEB_SEARCHENGINE_BING}, 
                                 {7, GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM}};

SearchEngineWidget::SearchEngineWidget(QWidget *parent)
    :DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Default search engine"), this);
    m_groupLabel->adjustSize();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(4);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_groupLabel);

    QString content = tr("Search for keywords by the default search engine.");

    m_contentLabel = new TipsLabel(content, this);
    m_contentLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_contentLabel);

    m_comboboxWidget = new ComboboxWidget(this);
    m_comboboxWidget->setFixedSize(COMBOBOXWIDGETWIDTH, COMBOBOXWIDGETHEIGHT);
    m_comboboxWidget->setEnableBackground(true);
    QString userChoice = SearchConfig::instance()->getConfig(GRANDSEARCH_WEB_GROUP, GRANDSEARCH_WEB_SEARCHENGINE, "").toString();
    auto comboBox = m_comboboxWidget->getComboBox();

    QString display = tr("Default search engine is");
    int maxLabelWidth = m_comboboxWidget->width() - Margin - Margin - comboBox->width();
    QFont nameFont = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QFontMetrics contentFontMetrics(nameFont);
    QString elidedContent = contentFontMetrics.elidedText(display, Qt::ElideRight, maxLabelWidth);
    m_comboboxWidget->setTitle(elidedContent);

    QStringList list;
    {
        const QString searchEngineGoogle = tr("Google");
        const QString searchEngineBaidu = tr("Baidu");
        const QString searchEngineBing = tr("Bing");
        const QString searchEngineYahoo = tr("Yahoo");
        const QString searchEngine360 = tr("360");
        const QString searchEngine360AI = tr("360 AI Search");
        const QString searchEngineSogou = tr("Sogou");
        const QString searchEngineCustom = tr("Custom");
        if (searchHelper->isSimplifiedChinese()) {
            list << searchEngineBaidu << searchEngineSogou << searchEngine360 << searchEngine360AI
            << searchEngineGoogle << searchEngineYahoo << searchEngineBing << searchEngineCustom;
        } else {
            list << searchEngineGoogle<< searchEngineYahoo << searchEngineBing << searchEngineBaidu << searchEngineCustom;
        }
    }

    comboBox->addItems(list);
    if (userChoice.isEmpty()) {
        comboBox->setCurrentIndex(0);
    } else {
        int index = getIndex(userChoice);
        comboBox->setCurrentIndex(index);
    }
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 10));
    m_mainLayout->addWidget(m_comboboxWidget);
    m_comboboxWidget->setProperty(GRANDSEARCH_SEARCH_GROUP, GRANDSEARCH_WEB_SEARCHENGINE);

    m_lineEdit =  new Dtk::Widget::DLineEdit(this);
    m_lineEdit->setPlaceholderText(tr("You need to use \"%0\" to replace the keyword in the URL"));
    if (userChoice == GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM) {
        QString searchEngineCustom = SearchConfig::instance()->getConfig(GRANDSEARCH_WEB_GROUP, GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM_ADDR, "").toString();
        m_lineEdit->setText(QString::fromStdString(QByteArray::fromBase64(searchEngineCustom.toUtf8()).toStdString()));
        m_lineEdit->setVisible(true);
    } else {
        m_lineEdit->setVisible(false);
    }
    m_mainLayout->addWidget(m_lineEdit);

    connect(m_comboboxWidget, &ComboboxWidget::checkedChanged, this, &SearchEngineWidget::checkedChangedIndex);
    connect(m_lineEdit, &DLineEdit::textChanged, this, &SearchEngineWidget::setCustomSearchEngineAddress);
}

SearchEngineWidget::~SearchEngineWidget()
{

}

void SearchEngineWidget::checkedChangedIndex(int index)
{
    GrandSearch::ComboboxWidget *comboboxWidget = qobject_cast<GrandSearch::ComboboxWidget *>(sender());

    if (comboboxWidget) {
        QString text;
        bool language = SearchHelper::instance()->isSimplifiedChinese();
        if (language) {
            text = searchEngineChinese.value(index);
        } else {
            text = searchEngineEnglish.value(index);
        }
        SearchConfig::instance()->setConfig(GRANDSEARCH_WEB_GROUP, GRANDSEARCH_WEB_SEARCHENGINE, text);

        if (text == GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM) {
            QString searchEngineCustom = SearchConfig::instance()->getConfig(GRANDSEARCH_WEB_GROUP, GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM_ADDR, "").toString();
            m_lineEdit->setText(QString::fromStdString(QByteArray::fromBase64(searchEngineCustom.toUtf8()).toStdString()));
            m_lineEdit->setVisible(true);
        } else {
            m_lineEdit->setVisible(false);
        }
    }
}

int SearchEngineWidget::getIndex(const QString text) const
{
    int index = 0;
    bool language = SearchHelper::instance()->isSimplifiedChinese();
    if (language) {
        index = searchEngineChinese.key(text);
    } else {
        index = searchEngineEnglish.key(text);
    }
    return index;
}

void SearchEngineWidget::setCustomSearchEngineAddress(const QString &text)
{
    if (text.isEmpty()) {
        m_lineEdit->setAlert(false);
        return;
    }

    if (!text.contains("%0")) {
        m_lineEdit->setAlert(true);
        m_lineEdit->showAlertMessage(tr("Invalid URL"), m_lineEdit, 2000);
    } else {
        m_lineEdit->setAlert(false);
        SearchConfig::instance()->setConfig(GRANDSEARCH_WEB_GROUP, GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM_ADDR, QString::fromStdString(text.toUtf8().toBase64().toStdString()));
    }
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutview.h"
#include "shortcutitem.h"

#include <DFontSizeManager>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonObject>
#include <QByteArray>
#include <QVBoxLayout>
#include <QtMath>
#include <QDebug>
#include <QApplication>

#define MAX_COL_COUNT 3
DWIDGET_USE_NAMESPACE

ShortcutView::ShortcutView(QWidget *parent)
    : QWidget(parent)
{
    setFocus(Qt::MouseFocusReason);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(50);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_mainLayout);
}

void ShortcutView::setData(const QString &data)
{
    QByteArray byteArry;
    byteArry.append(data);

    QJsonParseError jsError;
    QJsonDocument doc = QJsonDocument::fromJson(byteArry, &jsError);
    QJsonObject obj;
    QJsonArray shortcuts;
    if (jsError.error == QJsonParseError::NoError) {
        if (doc.isObject()) {
            obj = doc.object();
            if (obj.contains("shortcut")) {
                QJsonValue value;
                value = obj.value("shortcut");
                if (value.isArray()) {
                    shortcuts = value.toArray();
                }
            }
        }
    } else {
        qDebug() << "Json data parsing error:" << jsError.error << jsError.errorString();
        return;
    }

    foreach (QJsonValue value, shortcuts) {
        QJsonObject obj = value.toObject();
        Shortcut sc;
        sc.name = obj["groupName"].toString();
        sc.value = "type";
        sc.weight = 1.5;
        m_totalWeight += 1.5;

        m_shortcutList.append(sc);
        QJsonArray items = obj["groupItems"].toArray();
        foreach (QJsonValue itemvalue, items) {
            QJsonObject item = itemvalue.toObject();
            Shortcut sc1;
            sc1.name = item["name"].toString();
            sc1.value = item["value"].toString();
            sc1.weight = 1;
            m_shortcutList.append(sc1);
            m_totalWeight++;
        }
    }

    calcColumnData();
    initUI();
}
void ShortcutView::initUI()
{
    int spacing { itemSpacing() };
    for (const auto &col : m_colDatas) {
        QVBoxLayout *colLayout = new QVBoxLayout(this);
        colLayout->setSpacing(spacing);
        colLayout->setContentsMargins(0, 0, 0, 0);
        for (const auto &sc : col) {
            ShortcutItem *item = nullptr;
            if (sc.value == "type") {
                item = new ShortcutItem(true, this);
                item->setText(sc.name, "");
                item->setEnableBackground(true);
                colLayout->addSpacing(4);
            } else {
                item = new ShortcutItem(false, this);
                item->setText(sc.name, sc.value);
            }

            colLayout->addWidget(item);
        }

        colLayout->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        m_mainLayout->addLayout(colLayout);
    }
}

void ShortcutView::calcColumnData()
{
    if (m_shortcutList.size() <= 12) {
        m_colDatas << m_shortcutList;
    } else {
        int col = qCeil(m_totalWeight / 12);
        if (col > MAX_COL_COUNT) {
            col = MAX_COL_COUNT;
        }

        int res = qCeil(m_totalWeight / col);
        if (res > 20) {
            col = MAX_COL_COUNT + 1;
            res = qCeil(m_totalWeight / col);
        }

        double subSum = 0;
        int offset = 0;
        int colCount = 1;
        for (int i = 0; i < m_shortcutList.size(); ++i) {
            subSum += m_shortcutList[i].weight;
            if (res <= subSum) {
                m_colDatas << m_shortcutList.mid(offset, i - offset + 1);
                offset = i + 1;
                subSum = 0;

                if (++colCount == col) {
                    m_colDatas << m_shortcutList.mid(offset, m_shortcutList.size() - i);
                    break;
                }
            }
        }
    }
}

int ShortcutView::itemSpacing()
{
    QLocale systemLocale = QLocale::system();
    bool isNormal { systemLocale.language() == QLocale::Chinese };
    if (isNormal && qApp->devicePixelRatio() > 1.2)
        isNormal = false;

    int fontSize { DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6) };
    static const QMap<int, int> spacingMap {
        { 14, isNormal ? 16 : 8 },
        { 15, isNormal ? 14 : 7 },
        { 16, isNormal ? 12 : 6 },
        { 17, isNormal ? 10 : 5 },
        { 18, isNormal ? 8 : 4 },
        { 19, isNormal ? 6 : 3 },
        { 20, isNormal ? 4 : 2 }
    };
    return spacingMap.value(fontSize, 16);
}

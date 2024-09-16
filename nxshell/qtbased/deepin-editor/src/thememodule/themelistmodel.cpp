// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "themelistmodel.h"
#include "../common/utils.h"
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDir>

ThemeListModel::ThemeListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    initThemes();
}

ThemeListModel::~ThemeListModel()
{
}

void ThemeListModel::setFrameColor(const QString &selectedColor, const QString &normalColor)
{
    m_frameSelectedColor = selectedColor;
    m_frameNormalColor = normalColor;
}

void ThemeListModel::setSelection(const QString &path)
{
    for (auto pair : m_themes) {
        if (path == pair.second) {
            const int row = m_themes.indexOf(pair);
            const QModelIndex &idx = QAbstractListModel::index(row, 0);
            emit requestCurrentIndex(idx);
            break;
        }
    }
}

int ThemeListModel::rowCount(const QModelIndex &parent) const
{
    return m_themes.size();
}

QVariant ThemeListModel::data(const QModelIndex &index, int role) const
{
    const int r = index.row();

    const QString &name = m_themes.at(r).first;
    const QString &path = m_themes.at(r).second;

    switch (role) {
    case ThemeName:
        return name;
    case ThemePath:
        return path;
    case FrameNormalColor:
        return m_frameNormalColor;
    case FrameSelectedColor:
        return m_frameSelectedColor;
    }

    return QVariant();
}

void ThemeListModel::initThemes()
{
    QFileInfoList infoList = QDir(QString("%1share/deepin-editor/themes").arg(LINGLONG_PREFIX)).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    for (QFileInfo info : infoList) {
        QVariantMap jsonMap = Utils::getThemeMapFromPath(info.filePath());
        QPair<QString, QString> pair;
        pair.first = jsonMap["metadata"].toMap()["name"].toString();
        pair.second = info.filePath();

        m_themes << pair;
    }

    std::sort(m_themes.begin(), m_themes.end(),
              [=] (QPair<QString, QString> &a, QPair<QString, QString> &b) {
                  QVariantMap firstMap = Utils::getThemeMapFromPath(a.second);
                  QVariantMap secondMap = Utils::getThemeMapFromPath(b.second);

                  const QString &firstColor = firstMap["editor-colors"].toMap()["background-color"].toString();
                  const QString &secondColor = secondMap["editor-colors"].toMap()["background-color"].toString();

                  return QColor(firstColor).lightness() < QColor(secondColor).lightness();
              });
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchhelper.h"
#include "global/builtinsearch.h"

#include <QSet>
#include <QVariantList>
#include <QLocale>

using namespace GrandSearch;

SearchHelper::SearchHelper()
{
    initSuffixTable();
    initGroupSuffixTable();
    initGroupSearcherTable();
}

void SearchHelper::initSuffixTable()
{
    m_docSuffixTable = QString(DOCUMENT_SUFFIX).split(',');
    m_picSuffixTable = QString(PICTURE_SUFFIX).split(',');
    m_audioSuffixTable = QString(AUDIO_SUFFIX).split(',');
    m_videoSuffixTable = QString(VIDEO_SUFFIX).split(',');
    m_fileSuffixTable = QString(FILE_SUFFIX).split(',');
}

void SearchHelper::initGroupSuffixTable()
{
    m_groupSuffixHash = {{DOCUMENT_GROUP, m_docSuffixTable},
                         {PICTURE_GROUP, m_picSuffixTable},
                         {AUDIO_GROUP, m_audioSuffixTable},
                         {VIDEO_GROUP, m_videoSuffixTable},
                         {FILE_GROUP, m_fileSuffixTable}};
}

void SearchHelper::initGroupSearcherTable()
{
    m_groupSearcherHash.insert(DOCUMENT_GROUP, {GRANDSEARCH_CLASS_FILE_DEEPIN, GRANDSEARCH_CLASS_FILE_FSEARCH});
    m_groupSearcherHash.insert(PICTURE_GROUP, {GRANDSEARCH_CLASS_FILE_DEEPIN, GRANDSEARCH_CLASS_FILE_FSEARCH});
    m_groupSearcherHash.insert(AUDIO_GROUP, {GRANDSEARCH_CLASS_FILE_DEEPIN, GRANDSEARCH_CLASS_FILE_FSEARCH});
    m_groupSearcherHash.insert(VIDEO_GROUP, {GRANDSEARCH_CLASS_FILE_DEEPIN, GRANDSEARCH_CLASS_FILE_FSEARCH});
    m_groupSearcherHash.insert(FILE_GROUP, {GRANDSEARCH_CLASS_FILE_DEEPIN, GRANDSEARCH_CLASS_FILE_FSEARCH});
    m_groupSearcherHash.insert(FOLDER_GROUP, {GRANDSEARCH_CLASS_FILE_DEEPIN, GRANDSEARCH_CLASS_FILE_FSEARCH});
    m_groupSearcherHash.insert(APPLICATION_GROUP, {GRANDSEARCH_CLASS_APP_DESKTOP});
}

SearchHelper *SearchHelper::instance()
{
    static SearchHelper instance;
    return &instance;
}

bool SearchHelper::parseKeyword(const QString &keyword, QStringList &groupList, QStringList &suffixList, QStringList &keywordList)
{
    if (!keyword.contains(':'))
        return false;

    const auto &allItems = keyword.split(':');
    for (const auto &item : allItems) {
        if (item.isEmpty())
            continue;

        if (isGroupName(item)) {
            groupList.append(item);
        } else if (isSuffix(item)) {
            suffixList.append(item);
        } else {
            keywordList.append(item);
        }
    }

    return true;
}

bool SearchHelper::isSuffix(const QString &suffix)
{
    if (suffix.isEmpty())
        return false;

    return (m_docSuffixTable.contains(suffix, Qt::CaseInsensitive) ||
            m_picSuffixTable.contains(suffix, Qt::CaseInsensitive) ||
            m_audioSuffixTable.contains(suffix, Qt::CaseInsensitive) ||
            m_videoSuffixTable.contains(suffix, Qt::CaseInsensitive) ||
            m_fileSuffixTable.contains(suffix, Qt::CaseInsensitive));
}

bool SearchHelper::isGroupName(const QString &name)
{
    if (name.isEmpty())
        return false;

    DEF_ALL_GROUPS;
    return allGroups.contains(name, Qt::CaseInsensitive);
}

QStringList SearchHelper::getSearcherByGroupName(const QString &name)
{
    const auto &tmpName = name.toLower();
    if (!m_groupSearcherHash.contains(tmpName))
        return {};

    return m_groupSearcherHash[tmpName];
}

QStringList SearchHelper::getSuffixByGroupName(const QString &name)
{
    const auto &tmpName = name.toLower();
    if (!m_groupSuffixHash.contains(tmpName))
        return {};

    return m_groupSuffixHash[tmpName];
}

QString SearchHelper::tropeInputSymbol(const QString &pattern)
{
    const int len = pattern.length();
    QString rx;
    rx.reserve(len + len / 16);
    int i = 0;
    const QChar *wc = pattern.unicode();

    while (i < len) {
        const QChar c = wc[i++];
        switch (c.unicode()) {
        case '*':
        case '?':
        case '\\':
        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
        case '[':
        case ']':
            rx += QLatin1Char('\\');
            rx += c;
            break;
        default:
            rx += c;
            break;
        }
    }

    return rx;
}

/**
 * @brief SearchHelper::isSimplifiedChinese
 * @return 简体中文返回true，非简体中文返回false
 */
bool SearchHelper::isSimplifiedChinese() const
{
    static const QList<QLocale> supportLanguage{{QLocale::Tibetan}, {QLocale::Uighur}, {QLocale::Mongolian}, {QLocale::Chinese}};
    QLocale locale;
    if(supportLanguage.contains(locale.language())) {
        static const QStringList traditionalChinese{{"zh_HK"}, {"zh_MO"}, {"zh_TW"}, "zh_SG"};
         if (traditionalChinese.contains(locale.name().simplified())) {
             return false;
         } else {
             return true;
         }
    } else {
        return false;
    }
}

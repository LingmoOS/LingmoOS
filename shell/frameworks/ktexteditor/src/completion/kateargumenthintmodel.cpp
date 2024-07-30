/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateargumenthintmodel.h"
#include "katecompletionmodel.h"
#include "katepartdebug.h"
#include <ktexteditor/codecompletionmodel.h>

#include <QTextFormat>

using namespace KTextEditor;

void KateArgumentHintModel::clear()
{
    m_rows.clear();
}

QModelIndex KateArgumentHintModel::mapToSource(const QModelIndex &index) const
{
    if (size_t(index.row()) >= m_rows.size()) {
        return QModelIndex();
    }

    if (m_rows[index.row()] < 0 || m_rows[index.row()] >= (int)group()->filtered.size()) {
        return QModelIndex();
    }

    KateCompletionModel::ModelRow source = group()->filtered[m_rows[index.row()]].sourceRow();
    if (!source.first) {
        qCDebug(LOG_KTE) << "KateArgumentHintModel::data: Row does not exist in source";
        return QModelIndex();
    }

    QModelIndex sourceIndex = source.second.sibling(source.second.row(), index.column());

    return sourceIndex;
}

void KateArgumentHintModel::parentModelReset()
{
    clear();
    buildRows();
}

void KateArgumentHintModel::buildRows()
{
    beginResetModel();

    m_rows.clear();
    std::map<int, std::vector<int>> m_depths; // Map each hint-depth to a list of functions of that depth
    for (int a = 0; a < (int)group()->filtered.size(); a++) {
        KateCompletionModel::ModelRow source = group()->filtered[a].sourceRow();
        QModelIndex sourceIndex = source.second.sibling(source.second.row(), 0);
        QVariant v = sourceIndex.data(CodeCompletionModel::ArgumentHintDepth);
        if (v.userType() == QMetaType::Int) {
            std::vector<int> &lst(m_depths[v.toInt()]);
            lst.push_back(a);
        }
    }

    for (const auto &[key, value] : m_depths) {
        for (int row : value) {
            m_rows.insert(m_rows.begin(), row); // Insert filtered in reversed order
        }
    }

    endResetModel();

    Q_EMIT contentStateChanged(!m_rows.empty());
}

KateArgumentHintModel::KateArgumentHintModel(KateCompletionModel *parent)
    : QAbstractListModel(parent)
    , m_parent(parent)
{
    connect(parent, &KateCompletionModel::modelReset, this, &KateArgumentHintModel::parentModelReset);
    connect(parent, &KateCompletionModel::argumentHintsChanged, this, &KateArgumentHintModel::parentModelReset);
}

QVariant KateArgumentHintModel::data(const QModelIndex &index, int role) const
{
    if (size_t(index.row()) >= m_rows.size()) {
        // qCDebug(LOG_KTE) << "KateArgumentHintModel::data: index out of bound: " << index.row() << " total filtered: " << m_rows.count();
        return QVariant();
    }

    if (m_rows[index.row()] < 0 || m_rows[index.row()] >= (int)group()->filtered.size()) {
        qCDebug(LOG_KTE) << "KateArgumentHintModel::data: index out of bound: " << m_rows[index.row()] << " total filtered: " << (int)group()->filtered.size();
        return QVariant();
    }

    KateCompletionModel::ModelRow source = group()->filtered[m_rows[index.row()]].sourceRow();
    if (!source.first) {
        qCDebug(LOG_KTE) << "KateArgumentHintModel::data: Row does not exist in source";
        return QVariant();
    }

    QModelIndex sourceIndex = source.second.sibling(source.second.row(), index.column());

    if (!sourceIndex.isValid()) {
        qCDebug(LOG_KTE) << "KateArgumentHintModel::data: Source-index is not valid";
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        // Construct the text
        QString totalText;
        for (int a = CodeCompletionModel::Prefix; a <= CodeCompletionModel::Postfix; a++) {
            if (a != CodeCompletionModel::Scope) { // Skip the scope
                totalText += source.second.sibling(source.second.row(), a).data(Qt::DisplayRole).toString() + QLatin1Char(' ');
            }
        }

        return QVariant(totalText);
    }
    case CodeCompletionModel::HighlightingMethod: {
        // Return that we are doing custom-highlighting of one of the sub-strings does it
        for (int a = CodeCompletionModel::Prefix; a <= CodeCompletionModel::Postfix; a++) {
            QVariant method = source.second.sibling(source.second.row(), a).data(CodeCompletionModel::HighlightingMethod);
            if (method.userType() == QMetaType::Int && method.toInt() == CodeCompletionModel::CustomHighlighting) {
                return QVariant(CodeCompletionModel::CustomHighlighting);
            }
        }

        return QVariant();
    }
    case CodeCompletionModel::CustomHighlight: {
        QStringList strings;

        // Collect strings
        for (int a = CodeCompletionModel::Prefix; a <= CodeCompletionModel::Postfix; a++) {
            strings << source.second.sibling(source.second.row(), a).data(Qt::DisplayRole).toString();
        }

        QList<QVariantList> highlights;

        // Collect custom-highlightings
        for (int a = CodeCompletionModel::Prefix; a <= CodeCompletionModel::Postfix; a++) {
            highlights << source.second.sibling(source.second.row(), a).data(CodeCompletionModel::CustomHighlight).toList();
        }

        return mergeCustomHighlighting(strings, highlights, 1);
    }
    }

    return {};
}

int KateArgumentHintModel::rowCount(const QModelIndex &) const
{
    return m_rows.size();
}

KateCompletionModel::Group *KateArgumentHintModel::group() const
{
    return m_parent->m_argumentHints;
}

void KateArgumentHintModel::emitDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    Q_EMIT dataChanged(start, end);
}

#include "moc_kateargumenthintmodel.cpp"

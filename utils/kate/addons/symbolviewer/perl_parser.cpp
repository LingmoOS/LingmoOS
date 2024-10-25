/***************************************************************************
                          perl_parser.cpp  -  description
                             -------------------
    begin                : Apr 2 2003
    author               : 2003 Massimo Callegari
    email                : massimocallegari@yahoo.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#include "plugin_katesymbolviewer.h"

void KatePluginSymbolViewerView::parsePerlSymbols(void)
{
    if (!m_mainWindow->activeView()) {
        return;
    }

    m_macro->setText(i18n("Show Uses"));
    m_struct->setText(i18n("Show Pragmas"));
    m_func->setText(i18n("Show Subroutines"));
    bool is_comment = false;
    QTreeWidgetItem *node = nullptr;
    QTreeWidgetItem *mcrNode = nullptr, *sctNode = nullptr, *clsNode = nullptr;
    QTreeWidgetItem *lastMcrNode = nullptr, *lastSctNode = nullptr, *lastClsNode = nullptr;

    KTextEditor::Document *kv = m_mainWindow->activeView()->document();

    // kdDebug(13000)<<"Lines counted :"<<kv->numLines()<<endl;
    if (m_treeOn->isChecked()) {
        mcrNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Uses")));
        sctNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Pragmas")));
        clsNode = new QTreeWidgetItem(m_symbols, QStringList(i18n("Subroutines")));
        mcrNode->setIcon(0, m_icon_block);
        sctNode->setIcon(0, m_icon_context);
        clsNode->setIcon(0, m_icon_class);

        if (m_expandOn->isChecked()) {
            m_symbols->expandItem(mcrNode);
            m_symbols->expandItem(sctNode);
            m_symbols->expandItem(clsNode);
        }
        lastMcrNode = mcrNode;
        lastSctNode = sctNode;
        lastClsNode = clsNode;
        m_symbols->setRootIsDecorated(1);
    } else {
        m_symbols->setRootIsDecorated(0);
    }

    for (int i = 0; i < kv->lines(); i++) {
        QString cl = kv->line(i);
        // qDebug()<< "Line " << i << " : "<< cl;

        if (cl.isEmpty() || cl.at(0) == QLatin1Char('#')) {
            continue;
        }
        if (cl.indexOf(QRegularExpression(QLatin1String("^=[a-zA-Z]"))) >= 0) {
            is_comment = true;
        }
        if (cl.indexOf(QRegularExpression(QLatin1String("^=cut$"))) >= 0) {
            is_comment = false;
            continue;
        }
        if (is_comment) {
            continue;
        }

        cl = cl.trimmed();
        // qDebug()<<"Trimmed line " << i << " : "<< cl;

        if (cl.indexOf(QRegularExpression(QLatin1String("^use +[A-Z]"))) == 0 && m_macro->isChecked()) {
            QString stripped = cl.remove(QRegularExpression(QLatin1String("^use +")));
            // stripped=stripped.replace( QRegularExpression(QLatin1String(";$")), "" ); // Doesn't work ??
            stripped = stripped.left(stripped.indexOf(QLatin1Char(';')));
            if (m_treeOn->isChecked()) {
                node = new QTreeWidgetItem(mcrNode, lastMcrNode);
                lastMcrNode = node;
            } else {
                node = new QTreeWidgetItem(m_symbols);
            }

            node->setText(0, stripped);
            node->setIcon(0, m_icon_block);
            node->setText(1, QString::number(i, 10));
        }
#if 1
        if (cl.indexOf(QRegularExpression(QLatin1String("^use +[a-z]"))) == 0 && m_struct->isChecked()) {
            QString stripped = cl.remove(QRegularExpression(QLatin1String("^use +")));
            stripped.remove(QRegularExpression(QLatin1String(";$")));
            if (m_treeOn->isChecked()) {
                node = new QTreeWidgetItem(sctNode, lastSctNode);
                lastMcrNode = node;
            } else {
                node = new QTreeWidgetItem(m_symbols);
            }

            node->setText(0, stripped);
            node->setIcon(0, m_icon_context);
            node->setText(1, QString::number(i, 10));
        }
#endif
#if 1
        if (cl.indexOf(QRegularExpression(QLatin1String("^sub +"))) == 0 && m_func->isChecked()) {
            QString stripped = cl.remove(QRegularExpression(QLatin1String("^sub +")));
            stripped.remove(QRegularExpression(QLatin1String("[{;] *$")));
            if (m_treeOn->isChecked()) {
                node = new QTreeWidgetItem(clsNode, lastClsNode);
                lastClsNode = node;
            } else {
                node = new QTreeWidgetItem(m_symbols);
            }
            node->setText(0, stripped);

            if (!stripped.isEmpty() && stripped.at(0) == QLatin1Char('_')) {
                node->setIcon(0, m_icon_function);
            } else {
                node->setIcon(0, m_icon_class);
            }

            node->setText(1, QString::number(i, 10));
        }
#endif
    }
}

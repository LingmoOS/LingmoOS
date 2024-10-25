/***************************************************************************
    xml_parser.cpp  -  Produce a rudimentary list of tags/elements
    present in XML or HTML files. In the tree view of the
    symbolviewer plugin the list is grouped by the element type.
                            -------------------
    begin                : May 3 2019
    author               : 20019 Andreas Hohenegger based on
                                xslt_parser.cpp by jiri Tyr
    email                : hohenegger@gmail.com
***************************************************************************/
/***************************************************************************
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 ***************************************************************************/

#include "plugin_katesymbolviewer.h"

void KatePluginSymbolViewerView::parseXMLSymbols(void)
{
    if (!m_mainWindow->activeView()) {
        return;
    }

    m_struct->setText(i18n("Show Tags"));

    QTreeWidgetItem *node = nullptr;
    QTreeWidgetItem *topNode = nullptr;

    KTextEditor::Document *kv = m_mainWindow->activeView()->document();

    m_symbols->setRootIsDecorated(0);

    bool is_comment = false;
    for (int i = 0; i < kv->lines(); i++) {
        QString cl = kv->line(i);
        cl = cl.trimmed();

        if (cl.indexOf(QLatin1String("<!--")) >= 0) {
            is_comment = true;
        }
        if (cl.indexOf(QLatin1String("-->")) >= 0) {
            is_comment = false;
            continue;
        }

        if (is_comment) {
            continue;
        }

        if (cl.indexOf(QRegularExpression(QLatin1String("^<[a-zA-Z_]+[a-zA-Z0-9_\\.\\-]*"))) == 0 && m_struct->isChecked()) {
            /* Get the tag type */
            QString type;
            QRegularExpressionMatch match;
            QRegularExpression re(QLatin1String("^<([a-zA-Z_]+[a-zA-Z0-9_\\.\\-]*)"));
            if (cl.contains(re, &match)) {
                type = match.captured(1);
            } else {
                continue;
            }

            QString stripped = cl.remove(QRegularExpression(QLatin1String("^<[a-zA-Z_]+[a-zA-Z0-9_\\.\\-]* *")));
            stripped.remove(QRegularExpression(QLatin1String(" */*>.*")));

            if (m_treeOn->isChecked()) {
                /* See if group already exists */
                QList<QTreeWidgetItem *> reslist = m_symbols->findItems(type, Qt::MatchExactly);
                if (reslist.isEmpty()) {
                    topNode = new QTreeWidgetItem(m_symbols, QStringList(type));
                    topNode->setIcon(0, m_icon_class);
                    if (m_expandOn->isChecked()) {
                        m_symbols->expandItem(topNode);
                    }
                } else {
                    topNode = reslist[0];
                }
                node = new QTreeWidgetItem(topNode);
                topNode->addChild(node);
            } else {
                node = new QTreeWidgetItem(m_symbols);
            }
            node->setIcon(0, m_icon_variable);
            node->setText(0, stripped);
            node->setText(1, QString::number(i, 10));
        }
    }
}

// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//#include "logexportwidget.h"
//#include <QDebug>
//#include <QFile>
//#include <QTextCodec>
//#include <QTextStream>

//#include "document.h"
//#include "table.h"
//#include "xlsxdocument.h"

//#include <DApplication>
//#include <QTextDocument>
//#include <QTextDocumentWriter>

//DWIDGET_USE_NAMESPACE

//LogExportWidget::LogExportWidget(QWidget *parent)
//    : DWidget(parent)
//{
//}

//bool LogExportWidget::exportToTxt(QString fileName, QStandardItemModel *pModel, LOG_FLAG flag)
//{
//    QFile fi(fileName);
//    if (!fi.open(QIODevice::WriteOnly))
//        return false;

//    QTextStream out(&fi);
//    if (flag == APP) {
//        for (int row = 0; row < pModel->rowCount(); ++row) {
//            out << pModel->horizontalHeaderItem(0)->text() << ": "
//                << pModel->item(row, 0)->data(Qt::UserRole + 6).toString() << " ";
//            for (int col = 1; col < pModel->columnCount(); ++col) {
//                out << pModel->horizontalHeaderItem(col)->text() << ": "
//                    << pModel->item(row, col)->text() << " ";
//            }
//            out << "\n";
//        }
//    } else {
//        for (int row = 0; row < pModel->rowCount(); row++) {
//            for (int col = 0; col < pModel->columnCount(); col++) {
//                out << pModel->horizontalHeaderItem(col)->text() << ": "
//                    << pModel->item(row, col)->text() << " ";
//            }
//            out << "\n";
//        }
//    }
//    fi.close();
//    return true;
//}

//bool LogExportWidget::exportToTxt(QString fileName, QList<LOG_MSG_JOURNAL> jList)
//{
//    QFile fi(fileName);
//    if (!fi.open(QIODevice::WriteOnly)) {
//        return false;
//    }
//    QTextStream out(&fi);
//    out.setCodec(QTextCodec::codecForName("utf-8"));
//    for (int i = 0; i < jList.count(); i++) {
//        LOG_MSG_JOURNAL jMsg = jList.at(i);
//        //        out << QString(DApplication::translate("Table", "Date and Time:")) <<
//        //        jMsg.dateTime << " "; out << QString(DApplication::translate("Table", "User:")) <<
//        //        jMsg.hostName << " "; out << QString(DApplication::translate("Table", "Daemon:"))
//        //        << jMsg.daemonName << " "; out << QString(DApplication::translate("Table",
//        //        "PID:")) << jMsg.daemonId << " "; out << QString(DApplication::translate("Table",
//        //        "Level:")) << jMsg.level << " "; out << QString(DApplication::translate("Table",
//        //        "Info:")) << jMsg.msg << " "; //delete bug
//        out << QString(DApplication::translate("Table", "Level:")) << jMsg.level << " ";
//        out << QString(DApplication::translate("Table", "Process:")) << jMsg.daemonName << " ";
//        out << QString(DApplication::translate("Table", "Date and Time:")) << jMsg.dateTime << " ";
//        if (jMsg.msg.isEmpty()) {
//            out << QString(DApplication::translate("Table", "Info:"))
//                << QString(DApplication::translate("Table", "Null")) << " ";  // modify for bug
//        } else {
//            out << QString(DApplication::translate("Table", "Info:")) << jMsg.msg << " ";
//        }
//        out << QString(DApplication::translate("Table", "User:")) << jMsg.hostName << " ";
//        out << QString(DApplication::translate("Table", "PID:")) << jMsg.daemonId << " ";
//        out << "\n";
//    }
//    fi.close();
//    return true;
//}

//bool LogExportWidget::exportToTxt(QString fileName, QList<LOG_MSG_JOURNAL> jList, QStringList labels)
//{
//    QFile fi(fileName);
//    if (!fi.open(QIODevice::WriteOnly)) {
//        return false;
//    }
//    QTextStream out(&fi);
//    out.setCodec(QTextCodec::codecForName("utf-8"));
//    for (int i = 0; i < jList.count(); i++) {
//        int col = 0;
//        LOG_MSG_JOURNAL jMsg = jList.at(i);
//        out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
//        out << labels.value(col++, "") << ":" << jMsg.hostName << " ";
//        out << labels.value(col++, "") << ":" << jMsg.daemonName << " ";
//        out << labels.value(col++, "") << ":" << jMsg.msg << " ";
//        out << "\n";
//    }
//    fi.close();
//    return true;
//}

//bool LogExportWidget::exportToTxt(QString fileName, QList<LOG_MSG_APPLICATOIN> jList, QStringList labels)
//{
//    QFile fi(fileName);
//    if (!fi.open(QIODevice::WriteOnly)) {
//        return false;
//    }
//    qDebug() << "labels" << labels;
//    QTextStream out(&fi);

//    out.setCodec(QTextCodec::codecForName("utf-8"));
//    for (int i = 0; i < jList.count(); i++) {
//        int col = 0;
//        LOG_MSG_APPLICATOIN jMsg = jList.at(i);
//        out << labels.value(col++, "") << ":" << jMsg.level << " ";
//        out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
//        out << labels.value(col++, "") << ":" << jMsg.src << " ";
//        out << labels.value(col++, "") << ":" << jMsg.msg << " ";
//        out << "\n";
//    }
//    fi.close();
//    return true;
//}

//bool LogExportWidget::exportToTxt(QString fileName, QList<LOG_MSG_DPKG> jList, QStringList labels)
//{
//    QFile fi(fileName);
//    if (!fi.open(QIODevice::WriteOnly)) {
//        return false;
//    }

//    QTextStream out(&fi);
//    out.setCodec(QTextCodec::codecForName("utf-8"));
//    for (int i = 0; i < jList.count(); i++) {
//        int col = 0;
//        LOG_MSG_DPKG jMsg = jList.at(i);
//        out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
//        out << labels.value(col++, "") << ":" << jMsg.msg << " ";
//        out << labels.value(col++, "") << ":" << jMsg.action << " ";
//        out << "\n";
//    }
//    fi.close();
//    return true;
//}

//bool LogExportWidget::exportToTxt(QString fileName, QList<LOG_MSG_BOOT> jList, QStringList labels)
//{
//    QFile fi(fileName);
//    if (!fi.open(QIODevice::WriteOnly)) {
//        return false;
//    }
//    QTextStream out(&fi);
//    out.setCodec(QTextCodec::codecForName("utf-8"));
//    for (int i = 0; i < jList.count(); i++) {
//        LOG_MSG_BOOT jMsg = jList.at(i);
//        int col = 0;
//        out << labels.value(col++, "") << ":" << jMsg.status << " ";
//        out << labels.value(col++, "") << ":" << jMsg.msg << " ";
//        out << "\n";
//    }
//    fi.close();
//    return true;
//}

//bool LogExportWidget::exportToTxt(QString fileName, QList<LOG_MSG_XORG> jList, QStringList labels)
//{
//    QFile fi(fileName);
//    if (!fi.open(QIODevice::WriteOnly)) {
//        return false;
//    }
//    QTextStream out(&fi);
//    out.setCodec(QTextCodec::codecForName("utf-8"));
//    for (int i = 0; i < jList.count(); i++) {
//        LOG_MSG_XORG jMsg = jList.at(i);
//        int col = 0;
//        out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
//        out << labels.value(col++, "") << ":" << jMsg.msg << " ";
//        out << "\n";
//    }
//    fi.close();
//    return true;
//}

//bool LogExportWidget::exportToTxt(QString fileName, QList<LOG_MSG_NORMAL> jList, QStringList labels)
//{
//    QFile fi(fileName);
//    if (!fi.open(QIODevice::WriteOnly)) {
//        return false;
//    }
//    QTextStream out(&fi);
//    out.setCodec(QTextCodec::codecForName("utf-8"));
//    for (int i = 0; i < jList.count(); i++) {
//        LOG_MSG_NORMAL jMsg = jList.at(i);
//        int col = 0;
//        out << labels.value(col++, "") << ":" << jMsg.eventType << " ";
//        out << labels.value(col++, "") << ":" << jMsg.userName << " ";
//        out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
//        out << labels.value(col++, "") << ":" << jMsg.msg << " ";
//        out << "\n";
//    }
//    fi.close();
//    return true;
//}

//bool LogExportWidget::exportToDoc(QString fileName, QStandardItemModel *pModel, LOG_FLAG flag)
//{
//#if 1
//    Docx::Document doc(DOCTEMPLATE);
//    Docx::Table *tab = doc.addTable(pModel->rowCount() + 1, pModel->columnCount());
//    tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);

//    for (int col = 0; col < pModel->columnCount(); ++col) {
//        auto item = pModel->horizontalHeaderItem(col);
//        auto cel = tab->cell(0, col);
//        if (item) {
//            cel->addText(pModel->horizontalHeaderItem(col)->text());
//        }
//    }
//    if (flag == APP) {
//        for (int row = 0; row < pModel->rowCount(); ++row) {
//            auto cel = tab->cell(row + 1, 0);
//            cel->addText(pModel->item(row, 0)->data(Qt::UserRole + 6).toString());
//            for (int col = 1; col < pModel->columnCount(); ++col) {
//                auto cel = tab->cell(row + 1, col);
//                cel->addText(pModel->item(row, col)->text());
//            }
//        }
//    } else {
//        for (int row = 0; row < pModel->rowCount(); ++row) {
//            for (int col = 0; col < pModel->columnCount(); ++col) {
//                auto cel = tab->cell(row + 1, col);
//                cel->addText(pModel->item(row, col)->text());
//            }
//        }
//    }
//    doc.save(fileName);
//#else
//    QTextDocumentWriter writer(fileName);
//    writer.setFormat("odf");
//    QTextDocument *doc = new QTextDocument;
//    QString html;
//    html.append("<!DOCTYPE html>\n");
//    html.append("<html>\n");
//    html.append("<body>\n");
//    html.append("<table border=\"1\">\n");
//    // write title
//    html.append("<tr>");
//    for (int i = 0; i < pModel->columnCount(); ++i) {
//        QString labelInfo = QString("<td>%1</td>").arg(pModel->horizontalHeaderItem(i)->text());
//        html.append(labelInfo.toUtf8().data());
//    }
//    html.append("</tr>");
//    // write contents
//    for (int row = 0; row < pModel->rowCount(); ++row) {
//        html.append("<tr>");
//        for (int col = 0; col < pModel->columnCount(); ++col) {
//            QString info = QString("<td>%1</td>").arg(pModel->item(row, col)->text());
//            html.append(info.toUtf8().data());
//        }
//        html.append("</tr>");
//    }
//    html.append("</table>\n");
//    html.append("</body>\n");
//    html.append("</html>\n");
//    doc->setHtml(html);
//    writer.write(doc);
//#endif
//    return true;
//}

//bool LogExportWidget::exportToDoc(QString fileName, QList<LOG_MSG_JOURNAL> jList,
//                                  QStringList labels, LOG_FLAG iFlag)
//{
//#if 1
//    Docx::Document doc(DOCTEMPLATE);
//    Docx::Table *tab = doc.addTable(jList.count() + 1, 6);
//    tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);

//    for (int col = 0; col < labels.count(); ++col) {
//        auto cel = tab->cell(0, col);
//        cel->addText(labels.at(col));
//    }

//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_JOURNAL message = jList.at(row);
//        int col = 0;
//        if (iFlag == JOURNAL || iFlag == BOOT_KLU) {
//            tab->cell(row + 1, col++)->addText(message.level);
//            tab->cell(row + 1, col++)->addText(message.daemonName);
//            tab->cell(row + 1, col++)->addText(message.dateTime);
//            tab->cell(row + 1, col++)->addText(message.msg);
//            tab->cell(row + 1, col++)->addText(message.hostName);
//            tab->cell(row + 1, col++)->addText(message.daemonId);
//        } else if (iFlag == KERN) {
//            tab->cell(row + 1, col++)->addText(message.dateTime);
//            tab->cell(row + 1, col++)->addText(message.hostName);
//            tab->cell(row + 1, col++)->addText(message.daemonName);
//            tab->cell(row + 1, col++)->addText(message.msg);
//        }
//    }
//    doc.save(fileName);
//#else
//    QTextDocumentWriter writer(fileName);
//    writer.setFormat("odf");
//    QTextDocument *doc = new QTextDocument;
//    QString html;
//    html.append("<!DOCTYPE html>\n");
//    html.append("<html>\n");
//    html.append("<body>\n");
//    html.append("<table border=\"1\">\n");
//    QString title = QString(
//                        "<tr><td>时间</td><td>主机名</td><td>守护进程</td><td>进程ID</td><td>级别</td><td>消息</"
//                        "td></tr>");
//    html.append(title.toUtf8().data());
//    for (int i = 0; i < jList.count(); i++) {
//        LOG_MSG_JOURNAL jMsg = jList.at(i);
//        QString info =
//            QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
//            .arg(jMsg.dateTime)
//            .arg(jMsg.hostName)
//            .arg(jMsg.daemonName)
//            .arg(jMsg.daemonId)
//            .arg(jMsg.level)
//            .arg(jMsg.msg);
//        html.append(info.toUtf8().data());
//    }

//    html.append("</table>\n");
//    html.append("</body>\n");
//    html.append("</html>\n");
//    doc->setHtml(html);
//    writer.write(doc);
//#endif
//    return true;
//}

//bool LogExportWidget::exportToDoc(QString fileName, QList<LOG_MSG_APPLICATOIN> jList, QStringList labels)
//{
//    Docx::Document doc(DOCTEMPLATE);
//    Docx::Table *tab = doc.addTable(jList.count() + 1, 6);
//    tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);

//    for (int col = 0; col < labels.count(); ++col) {
//        auto cel = tab->cell(0, col);
//        cel->addText(labels.at(col));
//    }

//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_APPLICATOIN message = jList.at(row);
//        int col = 0;
//        tab->cell(row + 1, col++)->addText(message.level);
//        tab->cell(row + 1, col++)->addText(message.dateTime);
//        tab->cell(row + 1, col++)->addText(message.src);
//        tab->cell(row + 1, col++)->addText(message.msg);
//    }
//    doc.save(fileName);
//    return true;
//}

//bool LogExportWidget::exportToDoc(QString fileName, QList<LOG_MSG_DPKG> jList, QStringList labels)
//{
//    Docx::Document doc(DOCTEMPLATE);
//    Docx::Table *tab = doc.addTable(jList.count() + 1, 6);
//    tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);

//    for (int col = 0; col < labels.count(); ++col) {
//        auto cel = tab->cell(0, col);
//        cel->addText(labels.at(col));
//    }

//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_DPKG message = jList.at(row);
//        int col = 0;
//        tab->cell(row + 1, col++)->addText(message.dateTime);
//        tab->cell(row + 1, col++)->addText(message.msg);
//        tab->cell(row + 1, col++)->addText(message.action);

//    }
//    doc.save(fileName);
//    return true;
//}

//bool LogExportWidget::exportToDoc(QString fileName, QList<LOG_MSG_BOOT> jList, QStringList labels)
//{
//    Docx::Document doc(DOCTEMPLATE);
//    Docx::Table *tab = doc.addTable(jList.count() + 1, 6);
//    tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);

//    for (int col = 0; col < labels.count(); ++col) {
//        auto cel = tab->cell(0, col);
//        cel->addText(labels.at(col));
//    }

//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_BOOT message = jList.at(row);
//        int col = 0;
//        tab->cell(row + 1, col++)->addText(message.status);
//        tab->cell(row + 1, col++)->addText(message.msg);
//    }
//    doc.save(fileName);
//    return true;
//}

//bool LogExportWidget::exportToDoc(QString fileName, QList<LOG_MSG_XORG> jList, QStringList labels)
//{
//    Docx::Document doc(DOCTEMPLATE);
//    Docx::Table *tab = doc.addTable(jList.count() + 1, 6);
//    tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);

//    for (int col = 0; col < labels.count(); ++col) {
//        auto cel = tab->cell(0, col);
//        cel->addText(labels.at(col));
//    }

//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_XORG message = jList.at(row);
//        int col = 0;
//        tab->cell(row + 1, col++)->addText(message.dateTime);
//        tab->cell(row + 1, col++)->addText(message.msg);
//    }
//    doc.save(fileName);
//    return true;
//}

//bool LogExportWidget::exportToDoc(QString fileName, QList<LOG_MSG_NORMAL> jList, QStringList labels)
//{
//    Docx::Document doc(DOCTEMPLATE);
//    Docx::Table *tab = doc.addTable(jList.count() + 1, 6);
//    tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);
//    for (int col = 0; col < labels.count(); ++col) {
//        auto cel = tab->cell(0, col);
//        cel->addText(labels.at(col));
//    }
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_NORMAL message = jList.at(row);
//        int col = 0;
//        tab->cell(row + 1, col++)->addText(message.eventType);
//        tab->cell(row + 1, col++)->addText(message.userName);
//        tab->cell(row + 1, col++)->addText(message.dateTime);
//        tab->cell(row + 1, col++)->addText(message.msg);
//    }
//    doc.save(fileName);
//    return true;
//}

//bool LogExportWidget::exportToHtml(QString fileName, QStandardItemModel *pModel, LOG_FLAG flag)
//{
//    QFile html(fileName);
//    if (!html.open(QIODevice::WriteOnly))
//        return false;
//    html.write("<!DOCTYPE html>\n");
//    html.write("<html>\n");
//    html.write("<body>\n");
//    html.write("<table border=\"1\">\n");
//    // write title
//    html.write("<tr>");
//    for (int i = 0; i < pModel->columnCount(); ++i) {
//        QString labelInfo = QString("<td>%1</td>").arg(pModel->horizontalHeaderItem(i)->text());
//        html.write(labelInfo.toUtf8().data());
//    }
//    html.write("</tr>");
//    // write contents
//    if (flag == APP) {
//        for (int row = 0; row < pModel->rowCount(); ++row) {
//            html.write("<tr>");

//            QString info =
//                QString("<td>%1</td>").arg(pModel->item(row, 0)->data(Qt::UserRole + 6).toString());
//            html.write(info.toUtf8().data());

//            for (int col = 1; col < pModel->columnCount(); ++col) {
//                QString info = QString("<td>%1</td>").arg(pModel->item(row, col)->text());
//                html.write(info.toUtf8().data());
//            }
//            html.write("</tr>");
//        }
//    } else {
//        for (int row = 0; row < pModel->rowCount(); ++row) {
//            html.write("<tr>");
//            for (int col = 0; col < pModel->columnCount(); ++col) {
//                QString info = QString("<td>%1</td>").arg(pModel->item(row, col)->text());
//                html.write(info.toUtf8().data());
//            }
//            html.write("</tr>");
//        }
//    }
//    html.write("</table>\n");
//    html.write("</body>\n");
//    html.write("</html>\n");
//    html.close();
//    return true;
//}

//bool LogExportWidget::exportToHtml(QString fileName, QList<LOG_MSG_JOURNAL> jList)
//{
//    QFile html(fileName);
//    if (!html.open(QIODevice::WriteOnly)) {
//        return false;
//    }

//    html.write("<!DOCTYPE html>\n");
//    html.write("<html>\n");
//    html.write("<body>\n");
//    html.write("<table border=\"1\">\n");
//    //    QString title = QString(
//    //        "<tr><td>时间</td><td>主机名</td><td>守护进程</td><td>进程ID</td><td>级别</td><td>消息</"
//    //        "td></tr>");
//    //    QString title = QString("<tr><td>") +
//    //                    QString(DApplication::translate("Table", "Date and Time")) +
//    //                    QString("</td><td>") + QString(DApplication::translate("Table", "User")) +
//    //                    QString("</td><td>") + QString(DApplication::translate("Table", "Daemon"))
//    //                    + QString("</td><td>") + QString(DApplication::translate("Table", "PID"))
//    //                    + QString("</td><td>") + QString(DApplication::translate("Table",
//    //                    "Level")) + QString("</td><td>") +
//    //                    QString(DApplication::translate("Table", "Info")) + QString("</td></tr>");
//    //                    //delete  for bug
//    QString title = QString("<tr><td>") + QString(DApplication::translate("Table", "Level")) +
//                    QString("</td><td>") + QString(DApplication::translate("Table", "Process")) +
//                    QString("</td><td>") +
//                    QString(DApplication::translate("Table", "Date and Time")) +
//                    QString("</td><td>") + QString(DApplication::translate("Table", "Info")) +
//                    QString("</td><td>") + QString(DApplication::translate("Table", "User")) +
//                    QString("</td><td>") + QString(DApplication::translate("Table", "PID")) +
//                    QString("</td></tr>");
//    html.write(title.toUtf8().data());
//    for (int i = 0; i < jList.count(); i++) {
//        LOG_MSG_JOURNAL jMsg = jList.at(i);
//        //        QString info =
//        //            QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
//        //                .arg(jMsg.dateTime)
//        //                .arg(jMsg.hostName)
//        //                .arg(jMsg.daemonName)
//        //                .arg(jMsg.daemonId)
//        //                .arg(jMsg.level)
//        //                .arg(jMsg.msg); //delete for bug
//        QString info =
//            QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
//            .arg(jMsg.level)
//            .arg(jMsg.daemonName)
//            .arg(jMsg.dateTime)
//            .arg(jMsg.msg)
//            .arg(jMsg.hostName)
//            .arg(jMsg.daemonId);
//        html.write(info.toUtf8().data());
//    }

//    html.write("</table>\n");
//    html.write("</body>\n");
//    html.write("</html>\n");
//    html.close();
//    return true;
//}

//bool LogExportWidget::exportToHtml(QString fileName, QList<LOG_MSG_JOURNAL> jList, QStringList labels)
//{
//    QFile html(fileName);
//    if (!html.open(QIODevice::WriteOnly))
//        return false;
//    html.write("<!DOCTYPE html>\n");
//    html.write("<html>\n");
//    html.write("<body>\n");
//    html.write("<table border=\"1\">\n");
//    // write title
//    html.write("<tr>");
//    for (int i = 0; i < labels.count(); ++i) {
//        QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
//        html.write(labelInfo.toUtf8().data());
//    }
//    html.write("</tr>");
//    // write contentselse
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_JOURNAL jMsg = jList.at(row);
//        QString info =
//            QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
//            .arg(jMsg.dateTime)
//            .arg(jMsg.hostName)
//            .arg(jMsg.daemonName)
//            .arg(jMsg.msg);
//        html.write(info.toUtf8().data());
//    }
//    html.write("</table>\n");
//    html.write("</body>\n");
//    html.write("</html>\n");
//    html.close();
//    return true;
//}

//bool LogExportWidget::exportToHtml(QString fileName, QList<LOG_MSG_APPLICATOIN> jList, QStringList labels)
//{
//    QFile html(fileName);
//    if (!html.open(QIODevice::WriteOnly))
//        return false;
//    html.write("<!DOCTYPE html>\n");
//    html.write("<html>\n");
//    html.write("<body>\n");
//    html.write("<table border=\"1\">\n");
//    // write title
//    html.write("<tr>");
//    for (int i = 0; i < labels.count(); ++i) {
//        QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
//        html.write(labelInfo.toUtf8().data());
//    }
//    html.write("</tr>");
//    // write contentselse
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_APPLICATOIN jMsg = jList.at(row);
//        QString info =
//            QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
//            .arg(jMsg.level)
//            .arg(jMsg.dateTime)
//            .arg(jMsg.src)
//            .arg(jMsg.msg);
//        html.write(info.toUtf8().data());
//    }
//    html.write("</table>\n");
//    html.write("</body>\n");
//    html.write("</html>\n");
//    html.close();
//    return true;
//}

//bool LogExportWidget::exportToHtml(QString fileName, QList<LOG_MSG_DPKG> jList, QStringList labels)
//{
//    QFile html(fileName);
//    if (!html.open(QIODevice::WriteOnly))
//        return false;
//    html.write("<!DOCTYPE html>\n");
//    html.write("<html>\n");
//    html.write("<body>\n");
//    html.write("<table border=\"1\">\n");
//    // write title
//    html.write("<tr>");
//    for (int i = 0; i < labels.count(); ++i) {
//        QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
//        html.write(labelInfo.toUtf8().data());
//    }
//    html.write("</tr>");
//    // write contentselse
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_DPKG jMsg = jList.at(row);
//        QString info =
//            QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
//            .arg(jMsg.dateTime)
//            .arg(jMsg.msg)
//            .arg(jMsg.action);
//        html.write(info.toUtf8().data());
//    }
//    html.write("</table>\n");
//    html.write("</body>\n");
//    html.write("</html>\n");
//    html.close();
//    return true;
//}

//bool LogExportWidget::exportToHtml(QString fileName, QList<LOG_MSG_BOOT> jList, QStringList labels)
//{
//    QFile html(fileName);
//    if (!html.open(QIODevice::WriteOnly))
//        return false;
//    html.write("<!DOCTYPE html>\n");
//    html.write("<html>\n");
//    html.write("<body>\n");
//    html.write("<table border=\"1\">\n");
//    // write title
//    html.write("<tr>");
//    for (int i = 0; i < labels.count(); ++i) {
//        QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
//        html.write(labelInfo.toUtf8().data());
//    }
//    html.write("</tr>");
//    // write contentselse
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_BOOT jMsg = jList.at(row);
//        QString info =
//            QString("<tr><td>%1</td><td>%2</td></tr>")
//            .arg(jMsg.status)
//            .arg(jMsg.msg);
//        html.write(info.toUtf8().data());
//    }
//    html.write("</table>\n");
//    html.write("</body>\n");
//    html.write("</html>\n");
//    html.close();
//    return true;
//}

//bool LogExportWidget::exportToHtml(QString fileName, QList<LOG_MSG_XORG> jList, QStringList labels)
//{
//    QFile html(fileName);
//    if (!html.open(QIODevice::WriteOnly))
//        return false;
//    html.write("<!DOCTYPE html>\n");
//    html.write("<html>\n");
//    html.write("<body>\n");
//    html.write("<table border=\"1\">\n");
//    // write title
//    html.write("<tr>");
//    for (int i = 0; i < labels.count(); ++i) {
//        QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
//        html.write(labelInfo.toUtf8().data());
//    }
//    html.write("</tr>");
//    // write contentselse
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_XORG jMsg = jList.at(row);
//        QString info =
//            QString("<tr><td>%1</td><td>%2</td></tr>")
//            .arg(jMsg.dateTime)
//            .arg(jMsg.msg);
//        html.write(info.toUtf8().data());
//    }
//    html.write("</table>\n");
//    html.write("</body>\n");
//    html.write("</html>\n");
//    html.close();
//    return true;
//}

//bool LogExportWidget::exportToHtml(QString fileName, QList<LOG_MSG_NORMAL> jList, QStringList labels)
//{
//    QFile html(fileName);
//    if (!html.open(QIODevice::WriteOnly))
//        return false;
//    html.write("<!DOCTYPE html>\n");
//    html.write("<html>\n");
//    html.write("<body>\n");
//    html.write("<table border=\"1\">\n");
//    // write title
//    html.write("<tr>");
//    for (int i = 0; i < labels.count(); ++i) {
//        QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
//        html.write(labelInfo.toUtf8().data());
//    }
//    html.write("</tr>");
//    // write contentselse
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_NORMAL jMsg = jList.at(row);
//        QString info =
//            QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
//            .arg(jMsg.eventType)
//            .arg(jMsg.userName)
//            .arg(jMsg.dateTime)
//            .arg(jMsg.msg);
//        html.write(info.toUtf8().data());
//    }
//    html.write("</table>\n");
//    html.write("</body>\n");
//    html.write("</html>\n");
//    html.close();
//    return true;
//}

//bool LogExportWidget::exportToXls(QString fileName, QStandardItemModel *pModel, LOG_FLAG flag)
//{
//    auto currentXlsRow = 1;
//    QXlsx::Document xlsx;

//    for (int col = 0; col < pModel->columnCount(); ++col) {
//        auto item = pModel->horizontalHeaderItem(col);
//        if (item) {
//            QXlsx::Format boldFont;
//            boldFont.setFontBold(true);
//            xlsx.write(currentXlsRow, col + 1, item->text(), boldFont);
//        }
//    }
//    ++currentXlsRow;

//    if (flag == APP) {
//        for (int row = 0; row < pModel->rowCount(); ++row) {
//            xlsx.write(currentXlsRow, 0 + 1,
//                       pModel->item(row, 0)->data(Qt::UserRole + 6).toString());
//            for (int col = 1; col < pModel->columnCount(); ++col) {
//                xlsx.write(currentXlsRow, col + 1, pModel->item(row, col)->text());
//            }
//            ++currentXlsRow;
//        }
//    } else {
//        for (int row = 0; row < pModel->rowCount(); ++row) {
//            for (int col = 0; col < pModel->columnCount(); ++col) {
//                xlsx.write(currentXlsRow, col + 1, pModel->item(row, col)->text());
//            }
//            ++currentXlsRow;
//        }
//    }

//    ++currentXlsRow;

//    xlsx.saveAs(fileName);

//    return true;
//}

//bool LogExportWidget::exportToXls(QString fileName, QList<LOG_MSG_JOURNAL> jList,
//                                  QStringList labels, LOG_FLAG iFlag)
//{
//    auto currentXlsRow = 1;
//    QXlsx::Document xlsx;
//    for (int col = 0; col < labels.count(); ++col) {
//        QXlsx::Format boldFont;
//        boldFont.setFontBold(true);
//        xlsx.write(currentXlsRow, col + 1, labels.at(col), boldFont);
//    }
//    ++currentXlsRow;
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_JOURNAL message = jList.at(row);
//        int col = 1;

//        if (iFlag == JOURNAL || iFlag == BOOT_KLU) {
//            xlsx.write(currentXlsRow, col++, message.level);
//            xlsx.write(currentXlsRow, col++, message.daemonName);
//            xlsx.write(currentXlsRow, col++, message.dateTime);
//            xlsx.write(currentXlsRow, col++, message.msg);
//            xlsx.write(currentXlsRow, col++, message.hostName);
//            xlsx.write(currentXlsRow, col++, message.daemonId);
//        } else if (iFlag == KERN) {
//            xlsx.write(currentXlsRow, col++, message.dateTime);
//            xlsx.write(currentXlsRow, col++, message.hostName);
//            xlsx.write(currentXlsRow, col++, message.daemonName);
//            xlsx.write(currentXlsRow, col++, message.msg);
//        }

//        ++currentXlsRow;
//    }
//    ++currentXlsRow;
//    xlsx.saveAs(fileName);
//    return true;
//}

//bool LogExportWidget::exportToXls(QString fileName, QList<LOG_MSG_APPLICATOIN> jList, QStringList labels)
//{
//    auto currentXlsRow = 1;
//    QXlsx::Document xlsx;
//    for (int col = 0; col < labels.count(); ++col) {
//        QXlsx::Format boldFont;
//        boldFont.setFontBold(true);
//        xlsx.write(currentXlsRow, col + 1, labels.at(col), boldFont);
//    }
//    ++currentXlsRow;
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_APPLICATOIN message = jList.at(row);
//        int col = 1;
//        xlsx.write(currentXlsRow, col++, message.level);
//        xlsx.write(currentXlsRow, col++, message.dateTime);
//        xlsx.write(currentXlsRow, col++, message.src);
//        xlsx.write(currentXlsRow, col++, message.msg);
//        ++currentXlsRow;
//    }
//    ++currentXlsRow;
//    xlsx.saveAs(fileName);
//    return true;
//}

//bool LogExportWidget::exportToXls(QString fileName, QList<LOG_MSG_DPKG> jList, QStringList labels)
//{
//    auto currentXlsRow = 1;
//    QXlsx::Document xlsx;
//    for (int col = 0; col < labels.count(); ++col) {
//        QXlsx::Format boldFont;
//        boldFont.setFontBold(true);
//        xlsx.write(currentXlsRow, col + 1, labels.at(col), boldFont);
//    }
//    ++currentXlsRow;
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_DPKG message = jList.at(row);
//        int col = 1;
//        xlsx.write(currentXlsRow, col++, message.dateTime);
//        xlsx.write(currentXlsRow, col++, message.msg);
//        xlsx.write(currentXlsRow, col++, message.action);
//        ++currentXlsRow;
//    }
//    ++currentXlsRow;
//    xlsx.saveAs(fileName);
//    return true;
//}

//bool LogExportWidget::exportToXls(QString fileName, QList<LOG_MSG_BOOT> jList, QStringList labels)
//{
//    auto currentXlsRow = 1;
//    QXlsx::Document xlsx;
//    for (int col = 0; col < labels.count(); ++col) {
//        QXlsx::Format boldFont;
//        boldFont.setFontBold(true);
//        xlsx.write(currentXlsRow, col + 1, labels.at(col), boldFont);
//    }
//    ++currentXlsRow;
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_BOOT message = jList.at(row);
//        int col = 1;
//        xlsx.write(currentXlsRow, col++, message.status);
//        xlsx.write(currentXlsRow, col++, message.msg);
//        ++currentXlsRow;
//    }
//    ++currentXlsRow;
//    xlsx.saveAs(fileName);
//    return true;
//}

//bool LogExportWidget::exportToXls(QString fileName, QList<LOG_MSG_XORG> jList, QStringList labels)
//{
//    auto currentXlsRow = 1;
//    QXlsx::Document xlsx;
//    for (int col = 0; col < labels.count(); ++col) {
//        QXlsx::Format boldFont;
//        boldFont.setFontBold(true);
//        xlsx.write(currentXlsRow, col + 1, labels.at(col), boldFont);
//    }
//    ++currentXlsRow;
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_XORG message = jList.at(row);
//        int col = 1;
//        xlsx.write(currentXlsRow, col++, message.dateTime);
//        xlsx.write(currentXlsRow, col++, message.msg);
//        ++currentXlsRow;
//    }
//    ++currentXlsRow;
//    xlsx.saveAs(fileName);
//    return true;
//}

//bool LogExportWidget::exportToXls(QString fileName, QList<LOG_MSG_NORMAL> jList, QStringList labels)
//{
//    auto currentXlsRow = 1;
//    QXlsx::Document xlsx;
//    for (int col = 0; col < labels.count(); ++col) {
//        QXlsx::Format boldFont;
//        boldFont.setFontBold(true);
//        xlsx.write(currentXlsRow, col + 1, labels.at(col), boldFont);
//    }
//    ++currentXlsRow;
//    for (int row = 0; row < jList.count(); ++row) {
//        LOG_MSG_NORMAL message = jList.at(row);
//        int col = 1;
//        xlsx.write(currentXlsRow, col++, message.eventType);
//        xlsx.write(currentXlsRow, col++, message.userName);
//        xlsx.write(currentXlsRow, col++, message.dateTime);
//        xlsx.write(currentXlsRow, col++, message.msg);
//        ++currentXlsRow;
//    }
//    ++currentXlsRow;
//    xlsx.saveAs(fileName);
//    return true;
//}

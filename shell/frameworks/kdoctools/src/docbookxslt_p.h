#ifndef _KDOCTOOLS_XSLT_P_H_
#define _KDOCTOOLS_XSLT_P_H_

QString splitOut(const QString &parsed, int index);
QByteArray fromUnicode(const QString &data);
void replaceCharsetHeader(QString &output);
QStringList locateFilesInDtdResource(const QString &file, const QStandardPaths::LocateOptions option = QStandardPaths::LocateFile);
QStringList getKDocToolsCatalogs();

#endif

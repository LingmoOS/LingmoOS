// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HTML2PDFCONVERTER_H
#define HTML2PDFCONVERTER_H

#include <QWebEnginePage>

/**
 * @brief The HtmltoPdfConverter class
 * html文件转换为pdf格式文件功能
 */
class HtmltoPdfConverter : public QObject
{
    Q_OBJECT
public:
    explicit HtmltoPdfConverter(const QString &inputPath, const QString &outputPath);
    int run();

private slots:
    void loadFinished(bool ok);
    void pdfPrintingFinished(const QString &filePath, bool success);

private:
    QString m_inputPath; //输入文件html
    QString m_outputPath; //输出文件pdf
    QScopedPointer<QWebEnginePage> m_page;
};

#endif // HTML2PDFCONVERTER_H

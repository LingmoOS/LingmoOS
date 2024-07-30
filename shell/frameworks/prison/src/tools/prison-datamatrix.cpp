/*
    SPDX-FileCopyrightText: 2010-2016 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#include <Prison/Barcode>

#include <QCoreApplication>
#include <QImageWriter>
#include <QStringList>

#include <iostream>
#include <qtextstream.h>
#include <stdlib.h>

void error(const QString &error, const QString &errormessage)
{
    QTextStream str(stdout);
    str << error << ": " << errormessage << '\n';
    str.flush();
    exit(0);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString size;
    QString outputfile;
    QString outputformat;
    QStringList arguments = app.arguments();

    arguments.takeFirst(); // Program (argv[0]).
    while (!arguments.isEmpty()) {
        QString argument = arguments.takeFirst();
        if (argument == QLatin1String("--")) {
            break; // rest is data
        } else if (argument == QLatin1String("--size") || argument == QLatin1String("-s")) {
            size = arguments.takeFirst();
        } else if (argument == QLatin1String("--outputfile") || argument == QLatin1String("--output-file") || argument == QLatin1String("-o")) {
            outputfile = arguments.takeFirst();
        } else if (argument == QLatin1String("--output-format") || argument == QLatin1String("--output-format") || argument == QLatin1String("-f")) {
            outputformat = arguments.takeFirst();
        } else if (argument.startsWith(QLatin1String("-"))) {
            error(QStringLiteral("unknown argument"), argument);
        } else {
            break;
        }
    }

    if (outputformat.isEmpty()) {
        outputformat = QStringLiteral("png");
    }

    if (!QImageWriter::supportedImageFormats().contains(outputformat.toLocal8Bit())) {
        error(QStringLiteral("unsupported output format"), outputformat);
    }

    if (outputfile.isEmpty()) {
        error(QStringLiteral("outputfile is missing"), QString());
    }

    bool ok = false;
    int intsize = size.toInt(&ok);
    if (!ok) {
        error(QStringLiteral("size not a int"), size);
    }
    if (intsize < 10) {
        error(QStringLiteral("needs a larger output size"), size);
    }

    QString data = arguments.join(QLatin1Char(' '));
    if (data.size() == 0) {
        QTextStream in(stdin);
        data = in.readAll();
        if (data.size() == 0) {
            error(QStringLiteral("No data, neither on commandline nor on stdin"), QString());
        }
    }

    auto barcode = Prison::Barcode::create(Prison::DataMatrix);
    if (!barcode) {
        error(QStringLiteral("unsupported barcode type"), QString());
    }

    barcode->setData(data);
    QImage result = barcode->toImage(QSizeF(intsize, intsize));
    QImageWriter w(outputfile, outputformat.toLocal8Bit());
    if (!w.write(result)) {
        error(QStringLiteral("writing failed"), w.errorString());
    }
    return EXIT_SUCCESS;
}

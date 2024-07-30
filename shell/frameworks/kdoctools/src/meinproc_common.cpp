
#include "meinproc_common.h"

#include "docbookxslt.h"
#include "docbookxslt_p.h"

#include <QDir>
#include <QFileInfo>

#include <cstdlib>

#ifdef Q_CC_MSVC
static inline FILE *popen(const char *command, const char *mode)
{
    return _popen(command, mode);
}

static inline int pclose(FILE *file)
{
    return _pclose(file);
}
#endif

CheckFileResult checkFile(const QString &checkFilename)
{
    const QFileInfo checkFile(checkFilename);
    if (!checkFile.exists()) {
        return CheckFileDoesNotExist;
    }
    if (!checkFile.isFile()) {
        return CheckFileIsNotFile;
    }
    if (!checkFile.isReadable()) {
        return CheckFileIsNotReadable;
    }
    return CheckFileSuccess;
}

CheckResult check(const QString &checkFilename, const QString &exe, const QByteArray &catalogs)
{
    const QString pwd_buffer = QDir::currentPath();
    const QFileInfo file(checkFilename);

    qputenv("XML_CATALOG_FILES", catalogs);
    if (QFileInfo(exe).isExecutable()) {
        QDir::setCurrent(file.absolutePath());
        QString cmd = exe;
        cmd += QStringLiteral(" --valid --noout ");
        cmd += file.fileName();
        cmd += QStringLiteral(" 2>&1");
        FILE *xmllint = popen(QFile::encodeName(cmd).constData(), "r");
        char buf[512];
        bool noout = true;
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf) - 1, xmllint))) {
            noout = false;
            buf[n] = '\0';
            fputs(buf, stderr);
        }
        pclose(xmllint);
        QDir::setCurrent(pwd_buffer);
        if (!noout) {
            return CheckNoOut;
        }
    } else {
        return CheckNoXmllint;
    }
    return CheckSuccess;
}

void doOutput(QString output, bool usingStdOut, bool usingOutput, const QString &outputOption, bool replaceCharset)
{
    if (output.indexOf(QStringLiteral("<FILENAME ")) == -1 || usingStdOut || usingOutput) {
        QFile file;
        if (usingStdOut) {
            file.open(stdout, QIODevice::WriteOnly);
        } else {
            if (usingOutput) {
                file.setFileName(outputOption);
            } else {
                file.setFileName(QStringLiteral("index.html"));
            }
            file.open(QIODevice::WriteOnly);
        }
        if (replaceCharset) {
            replaceCharsetHeader(output);
        }
#ifdef Q_OS_WIN
        QByteArray data = output.toUtf8();
#else
        QByteArray data = output.toLocal8Bit();
#endif
        file.write(data.data(), data.length());
        file.close();
    } else {
        int index = 0;
        while (true) {
            index = output.indexOf(QStringLiteral("<FILENAME "), index);
            if (index == -1) {
                break;
            }
            int filename_index = index + qstrlen("<FILENAME filename=\"");

            const QString filename = output.mid(filename_index, output.indexOf(QLatin1Char('\"'), filename_index) - filename_index);

            QString filedata = splitOut(output, index);
            QFile file(filename);
            file.open(QIODevice::WriteOnly);
            if (replaceCharset) {
                replaceCharsetHeader(filedata);
            }
            const QByteArray data = fromUnicode(filedata);
            file.write(data.data(), data.length());
            file.close();

            index += 8;
        }
    }
}

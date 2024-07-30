/*
    QImageIO Routines to read/write EPS images.
    SPDX-FileCopyrightText: 1998 Dirk Schoenberger <dirk.schoenberger@freenet.de>
    SPDX-FileCopyrightText: 2013 Alex Merry <alex.merry@kdemail.net>

    Includes code by Sven Wiegand <SWiegand@tfh-berlin.de> from KSnapshot

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "eps_p.h"

#include <QCoreApplication>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPrinter>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>

// logging category for this framework, default: log stuff >= warning
Q_LOGGING_CATEGORY(EPSPLUGIN, "kf.imageformats.plugins.eps", QtWarningMsg)

//#define EPS_PERFORMANCE_DEBUG 1

#define BBOX_BUFLEN 200
#define BBOX "%%BoundingBox:"
#define BBOX_LEN strlen(BBOX)

static bool seekToCodeStart(QIODevice *io, qint64 &ps_offset, qint64 &ps_size)
{
    char buf[4]; // We at most need to read 4 bytes at a time
    ps_offset = 0L;
    ps_size = 0L;

    if (io->read(buf, 2) != 2) { // Read first two bytes
        qCDebug(EPSPLUGIN) << "EPS file has less than 2 bytes.";
        return false;
    }

    if (buf[0] == '%' && buf[1] == '!') { // Check %! magic
        qCDebug(EPSPLUGIN) << "normal EPS file";
    } else if (buf[0] == char(0xc5) && buf[1] == char(0xd0)) { // Check start of MS-DOS EPS magic
        // May be a MS-DOS EPS file
        if (io->read(buf + 2, 2) != 2) { // Read further bytes of MS-DOS EPS magic
            qCDebug(EPSPLUGIN) << "potential MS-DOS EPS file has less than 4 bytes.";
            return false;
        }
        if (buf[2] == char(0xd3) && buf[3] == char(0xc6)) { // Check last bytes of MS-DOS EPS magic
            if (io->read(buf, 4) != 4) { // Get offset of PostScript code in the MS-DOS EPS file.
                qCDebug(EPSPLUGIN) << "cannot read offset of MS-DOS EPS file";
                return false;
            }
            ps_offset // Offset is in little endian
                = qint64(((unsigned char)buf[0]) + ((unsigned char)buf[1] << 8) + ((unsigned char)buf[2] << 16) + ((unsigned char)buf[3] << 24));
            if (io->read(buf, 4) != 4) { // Get size of PostScript code in the MS-DOS EPS file.
                qCDebug(EPSPLUGIN) << "cannot read size of MS-DOS EPS file";
                return false;
            }
            ps_size // Size is in little endian
                = qint64(((unsigned char)buf[0]) + ((unsigned char)buf[1] << 8) + ((unsigned char)buf[2] << 16) + ((unsigned char)buf[3] << 24));
            qCDebug(EPSPLUGIN) << "Offset: " << ps_offset << " Size: " << ps_size;
            if (!io->seek(ps_offset)) { // Get offset of PostScript code in the MS-DOS EPS file.
                qCDebug(EPSPLUGIN) << "cannot seek in MS-DOS EPS file";
                return false;
            }
            if (io->read(buf, 2) != 2) { // Read first two bytes of what should be the Postscript code
                qCDebug(EPSPLUGIN) << "PostScript code has less than 2 bytes.";
                return false;
            }
            if (buf[0] == '%' && buf[1] == '!') { // Check %! magic
                qCDebug(EPSPLUGIN) << "MS-DOS EPS file";
            } else {
                qCDebug(EPSPLUGIN) << "supposed Postscript code of a MS-DOS EPS file doe not start with %!.";
                return false;
            }
        } else {
            qCDebug(EPSPLUGIN) << "wrong magic for potential MS-DOS EPS file!";
            return false;
        }
    } else {
        qCDebug(EPSPLUGIN) << "not an EPS file!";
        return false;
    }
    return true;
}

static bool bbox(QIODevice *io, int *x1, int *y1, int *x2, int *y2)
{
    char buf[BBOX_BUFLEN + 1];

    bool ret = false;

    while (io->readLine(buf, BBOX_BUFLEN) > 0) {
        if (strncmp(buf, BBOX, BBOX_LEN) == 0) {
            // Some EPS files have non-integer values for the bbox
            // We don't support that currently, but at least we parse it
            float _x1;
            float _y1;
            float _x2;
            float _y2;
            if (sscanf(buf, "%*s %f %f %f %f", &_x1, &_y1, &_x2, &_y2) == 4) {
                qCDebug(EPSPLUGIN) << "BBOX: " << _x1 << " " << _y1 << " " << _x2 << " " << _y2;
                *x1 = int(_x1);
                *y1 = int(_y1);
                *x2 = int(_x2);
                *y2 = int(_y2);
                ret = true;
                break;
            }
        }
    }

    return ret;
}

EPSHandler::EPSHandler()
{
}

bool EPSHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("eps");
        return true;
    }
    return false;
}

bool EPSHandler::read(QImage *image)
{
    qCDebug(EPSPLUGIN) << "starting...";

    int x1;
    int y1;
    int x2;
    int y2;
#ifdef EPS_PERFORMANCE_DEBUG
    QTime dt;
    dt.start();
#endif

    QIODevice *io = device();
    qint64 ps_offset;
    qint64 ps_size;

    // find start of PostScript code
    if (!seekToCodeStart(io, ps_offset, ps_size)) {
        return false;
    }

    qCDebug(EPSPLUGIN) << "Offset:" << ps_offset << "; size:" << ps_size;

    // find bounding box
    if (!bbox(io, &x1, &y1, &x2, &y2)) {
        qCDebug(EPSPLUGIN) << "no bounding box found!";
        return false;
    }

    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        qCWarning(EPSPLUGIN) << "Could not create the temporary file" << tmpFile.fileName();
        return false;
    }
    qCDebug(EPSPLUGIN) << "temporary file:" << tmpFile.fileName();

    // x1, y1 -> translation
    // x2, y2 -> new size

    x2 -= x1;
    y2 -= y1;
    qCDebug(EPSPLUGIN) << "origin point: " << x1 << "," << y1 << "  size:" << x2 << "," << y2;
    double xScale = 1.0;
    double yScale = 1.0;
    int wantedWidth = x2;
    int wantedHeight = y2;

    // create GS command line

    const QString gsExec = QStandardPaths::findExecutable(QStringLiteral("gs"));
    if (gsExec.isEmpty()) {
        qCWarning(EPSPLUGIN) << "Couldn't find gs exectuable (from GhostScript) in PATH.";
        return false;
    }

    QStringList gsArgs;
    gsArgs << QLatin1String("-sOutputFile=") + tmpFile.fileName() << QStringLiteral("-q") << QStringLiteral("-g%1x%2").arg(wantedWidth).arg(wantedHeight)
           << QStringLiteral("-dSAFER") << QStringLiteral("-dPARANOIDSAFER") << QStringLiteral("-dNOPAUSE") << QStringLiteral("-sDEVICE=ppm")
           << QStringLiteral("-c")
           << QStringLiteral(
                  "0 0 moveto "
                  "1000 0 lineto "
                  "1000 1000 lineto "
                  "0 1000 lineto "
                  "1 1 254 255 div setrgbcolor fill "
                  "0 0 0 setrgbcolor")
           << QStringLiteral("-") << QStringLiteral("-c") << QStringLiteral("showpage quit");
    qCDebug(EPSPLUGIN) << "Running gs with args" << gsArgs;

    QProcess converter;
    converter.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    converter.start(gsExec, gsArgs);
    if (!converter.waitForStarted(3000)) {
        qCWarning(EPSPLUGIN) << "Reading EPS files requires gs (from GhostScript)";
        return false;
    }

    QByteArray intro = "\n";
    intro += QByteArray::number(-qRound(x1 * xScale));
    intro += " ";
    intro += QByteArray::number(-qRound(y1 * yScale));
    intro += " translate\n";
    converter.write(intro);

    io->reset();
    if (ps_offset > 0) {
        io->seek(ps_offset);
    }

    QByteArray buffer;
    buffer.resize(4096);
    bool limited = ps_size > 0;
    qint64 remaining = ps_size;
    qint64 count = io->read(buffer.data(), buffer.size());
    while (count > 0) {
        if (limited) {
            if (count > remaining) {
                count = remaining;
            }
            remaining -= count;
        }
        converter.write(buffer.constData(), count);
        if (!limited || remaining > 0) {
            count = io->read(buffer.data(), buffer.size());
        }
    }

    converter.closeWriteChannel();
    converter.waitForFinished(-1);

    QImageReader ppmReader(tmpFile.fileName(), "ppm");
    if (ppmReader.read(image)) {
        qCDebug(EPSPLUGIN) << "success!";
#ifdef EPS_PERFORMANCE_DEBUG
        qCDebug(EPSPLUGIN) << "Loading EPS took " << (float)(dt.elapsed()) / 1000 << " seconds";
#endif
        return true;
    } else {
        qCDebug(EPSPLUGIN) << "Reading failed:" << ppmReader.errorString();
        return false;
    }
}

bool EPSHandler::write(const QImage &image)
{
    QPrinter psOut(QPrinter::PrinterResolution);
    QPainter p;

    QTemporaryFile tmpFile(QStringLiteral("XXXXXXXX.pdf"));
    if (!tmpFile.open()) {
        return false;
    }

    psOut.setCreator(QStringLiteral("KDE EPS image plugin"));
    psOut.setOutputFileName(tmpFile.fileName());
    psOut.setOutputFormat(QPrinter::PdfFormat);
    psOut.setFullPage(true);
    const double multiplier = psOut.resolution() <= 0 ? 1.0 : 72.0 / psOut.resolution();
    psOut.setPageSize(QPageSize(image.size() * multiplier, QPageSize::Point));

    // painting the pixmap to the "printer" which is a file
    p.begin(&psOut);
    p.drawImage(QPoint(0, 0), image);
    p.end();

    QProcess converter;
    converter.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    converter.setReadChannel(QProcess::StandardOutput);

    // pdftops comes with Poppler and produces much smaller EPS files than GhostScript
    QStringList pdftopsArgs;
    pdftopsArgs << QStringLiteral("-eps") << tmpFile.fileName() << QStringLiteral("-");
    qCDebug(EPSPLUGIN) << "Running pdftops with args" << pdftopsArgs;
    converter.start(QStringLiteral("pdftops"), pdftopsArgs);

    if (!converter.waitForStarted()) {
        // GhostScript produces huge files, and takes a long time doing so
        QStringList gsArgs;
        gsArgs << QStringLiteral("-q") << QStringLiteral("-P-") << QStringLiteral("-dNOPAUSE") << QStringLiteral("-dBATCH") << QStringLiteral("-dSAFER")
               << QStringLiteral("-sDEVICE=epswrite") << QStringLiteral("-sOutputFile=-") << QStringLiteral("-c") << QStringLiteral("save")
               << QStringLiteral("pop") << QStringLiteral("-f") << tmpFile.fileName();
        qCDebug(EPSPLUGIN) << "Failed to start pdftops; trying gs with args" << gsArgs;
        converter.start(QStringLiteral("gs"), gsArgs);

        if (!converter.waitForStarted(3000)) {
            qCWarning(EPSPLUGIN) << "Creating EPS files requires pdftops (from Poppler) or gs (from GhostScript)";
            return false;
        }
    }

    while (converter.bytesAvailable() || (converter.state() == QProcess::Running && converter.waitForReadyRead(2000))) {
        device()->write(converter.readAll());
    }

    return true;
}

bool EPSHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCWarning(EPSPLUGIN) << "EPSHandler::canRead() called with no device";
        return false;
    }

    qint64 oldPos = device->pos();

    QByteArray head = device->readLine(64);
    int readBytes = head.size();
    if (device->isSequential()) {
        while (readBytes > 0) {
            device->ungetChar(head[readBytes-- - 1]);
        }
    } else {
        device->seek(oldPos);
    }

    return head.contains("%!PS-Adobe");
}

QImageIOPlugin::Capabilities EPSPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    // prevent bug #397040: when on app shutdown the clipboard content is to be copied to survive end of the app,
    // QXcbIntegration looks for some QImageIOHandler to apply, querying the capabilities and picking any first.
    // At that point this plugin no longer has its requirements e.g. to run the external process, so we have to deny.
    // The capabilities seem to be queried on demand in Qt code and not cached, so it's fine to report based
    // in current dynamic state
    if (!QCoreApplication::instance()) {
        return {};
    }

    if (format == "eps" || format == "epsi" || format == "epsf") {
        return Capabilities(CanRead | CanWrite);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && EPSHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *EPSPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new EPSHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_eps_p.cpp"

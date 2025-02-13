#include "cursortheme.h"
#include <QApplication>
#include <QSettings>
#include <QCursor>
#include <QDebug>
#include <QX11Info>
#include <QImage>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>

QHash<QString, QString> CursorTheme::m_alternatives;

CursorTheme::CursorTheme(const QDir &dir, QObject *parent)
    : QObject(parent)
{
    m_dirName = dir.dirName();
    m_path = dir.path();
    m_sample = "left_ptr";

    QString cursorFile = m_path + "/cursors/left_ptr";
    QList<int> sizeList;

    XcursorImages *images = XcursorFilenameLoadAllImages(qPrintable(cursorFile));

    if (images) {
        for (int i = 0; i < images->nimage; ++i) {
            if (!sizeList.contains(images->images[i]->size))
                sizeList.append(images->images[i]->size);
        }
        XcursorImagesDestroy(images);
        std::sort(sizeList.begin(), sizeList.end());
        m_sizes = sizeList;
    }

    QSettings config(m_path + "/index.theme", QSettings::IniFormat);
    config.beginGroup("Icon Theme");
    m_name = config.value("Name").toString();
    m_sample = config.value("Example").toString();
    m_hidden = config.value("Hidden", false).toBool();
    m_inherits = config.value("Inherits").toString();
}

QString CursorTheme::name() const
{
    return m_name;
}

QString CursorTheme::path() const
{
    return m_path;
}

QString CursorTheme::id() const
{
    return m_dirName;
}

QString CursorTheme::inherits() const
{
    return m_inherits;
}

QPixmap CursorTheme::pixmap() const
{
    if (m_pixmap.isNull())
        m_pixmap = createIcon();

    return m_pixmap;
}

int CursorTheme::defaultCursorSize() const
{
    if (!QX11Info::isPlatformX11()) {
        return 32;
    }

    /* This code is basically borrowed from display.c of the XCursor library
       We can't use "int XcursorGetDefaultSize(Display *dpy)" because if
       previously the cursor size was set to a custom value, it would return
       this custom value. */
    int size = 0;
    int dpi = 0;
    Display *dpy = QX11Info::display();
    // The string "v" is owned and will be destroyed by Xlib
    char *v = XGetDefault(dpy, "Xft", "dpi");
    if (v)
        dpi = atoi(v);
    if (dpi)
        size = dpi * 16 / 72;
    if (size == 0) {
        int dim;
        if (DisplayHeight(dpy, DefaultScreen(dpy)) < DisplayWidth(dpy, DefaultScreen(dpy))) {
            dim = DisplayHeight(dpy, DefaultScreen(dpy));
        } else {
            dim = DisplayWidth(dpy, DefaultScreen(dpy));
        }
        size = dim / 48;
    }
    return size;
}

XcursorImage *CursorTheme::xcLoadImage(const QString &image, int size) const
{
    QByteArray cursorName = QFile::encodeName(image);
    QByteArray themeName  = QFile::encodeName(id());

    return XcursorLibraryLoadImage(cursorName, themeName, size);
}

XcursorImages *CursorTheme::xcLoadImages(const QString &image, int size) const
{
    QByteArray cursorName = QFile::encodeName(image);
    QByteArray themeName  = QFile::encodeName(id());

    return XcursorLibraryLoadImages(cursorName, themeName, size);
}

QImage CursorTheme::loadImage(const QString &name, int size) const
{
    if (size <= 0)
        size = defaultCursorSize();

    XcursorImage *xcimage = xcLoadImage(name, size);

    if (!xcimage)
        xcimage = xcLoadImage(findAlternative(name), size);

    if (!xcimage) {
        return QImage();
    }

    // Convert the XcursorImage to a QImage, and auto-crop it
    QImage image((uchar *)xcimage->pixels, xcimage->width, xcimage->height,
                 QImage::Format_ARGB32_Premultiplied );

    image = autoCropImage(image);
    XcursorImageDestroy(xcimage);

    return image;
}

QPixmap CursorTheme::createIcon() const
{
    QPixmap pixmap = createIcon(36 * qApp->devicePixelRatio());
    return pixmap;
}

QPixmap CursorTheme::createIcon(int size) const
{
    QPixmap pixmap;
    QImage image = loadImage(m_sample, size);

    if (image.isNull() && m_sample != QLatin1String("left_ptr"))
        image = loadImage(QStringLiteral("left_ptr"), size);

    if (!image.isNull()) {
        pixmap = QPixmap::fromImage(image);
    }

    return pixmap;
}

qulonglong CursorTheme::loadCursor(const QString &name, int size) const
{
    if (!QX11Info::isPlatformX11()) {
        return None;
    }
    if (size <= 0)
        size = defaultCursorSize();

    // Load the cursor images
    XcursorImages *images = xcLoadImages(name, size);

    if (!images)
        images = xcLoadImages(findAlternative(name), size);

    if (!images)
        return None;

    // Create the cursor
    Cursor handle = XcursorImagesLoadCursor(QX11Info::display(), images);
    XcursorImagesDestroy(images);

    return handle;
}

QString CursorTheme::findAlternative(const QString &name) const
{
    if (m_alternatives.isEmpty()) {
        m_alternatives.reserve(18);

        // Qt uses non-standard names for some core cursors.
        // If Xcursor fails to load the cursor, Qt creates it with the correct name using the
        // core protocol instead (which in turn calls Xcursor). We emulate that process here.
        // Note that there's a core cursor called cross, but it's not the one Qt expects.
        m_alternatives.insert(QStringLiteral("cross"),          QStringLiteral("crosshair"));
        m_alternatives.insert(QStringLiteral("up_arrow"),       QStringLiteral("center_ptr"));
        m_alternatives.insert(QStringLiteral("wait"),           QStringLiteral("watch"));
        m_alternatives.insert(QStringLiteral("ibeam"),          QStringLiteral("xterm"));
        m_alternatives.insert(QStringLiteral("size_all"),       QStringLiteral("fleur"));
        m_alternatives.insert(QStringLiteral("pointing_hand"),  QStringLiteral("hand2"));

        // Precomputed MD5 hashes for the hardcoded bitmap cursors in Qt and KDE.
        // Note that the MD5 hash for left_ptr_watch is for the KDE version of that cursor.
        m_alternatives.insert(QStringLiteral("size_ver"),       QStringLiteral("00008160000006810000408080010102"));
        m_alternatives.insert(QStringLiteral("size_hor"),       QStringLiteral("028006030e0e7ebffc7f7070c0600140"));
        m_alternatives.insert(QStringLiteral("size_bdiag"),     QStringLiteral("fcf1c3c7cd4491d801f1e1c78f100000"));
        m_alternatives.insert(QStringLiteral("size_fdiag"),     QStringLiteral("c7088f0f3e6c8088236ef8e1e3e70000"));
        m_alternatives.insert(QStringLiteral("whats_this"),     QStringLiteral("d9ce0ab605698f320427677b458ad60b"));
        m_alternatives.insert(QStringLiteral("split_h"),        QStringLiteral("14fef782d02440884392942c11205230"));
        m_alternatives.insert(QStringLiteral("split_v"),        QStringLiteral("2870a09082c103050810ffdffffe0204"));
        m_alternatives.insert(QStringLiteral("forbidden"),      QStringLiteral("03b6e0fcb3499374a867c041f52298f0"));
        m_alternatives.insert(QStringLiteral("left_ptr_watch"), QStringLiteral("3ecb610c1bf2410f44200f48c40d3599"));
        m_alternatives.insert(QStringLiteral("hand2"),          QStringLiteral("e29285e634086352946a0e7090d73106"));
        m_alternatives.insert(QStringLiteral("openhand"),       QStringLiteral("9141b49c8149039304290b508d208c40"));
        m_alternatives.insert(QStringLiteral("closedhand"),     QStringLiteral("05e88622050804100c20044008402080"));
    }

    return m_alternatives.value(name, QString());
}

QImage CursorTheme::autoCropImage(const QImage &image) const
{
    // Compute an autocrop rectangle for the image
    QRect r(image.rect().bottomRight(), image.rect().topLeft());
    const quint32 *pixels = reinterpret_cast<const quint32*>(image.bits());

    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            if (*(pixels++)) {
                if (x < r.left())   r.setLeft(x);
                if (x > r.right())  r.setRight(x);
                if (y < r.top())    r.setTop(y);
                if (y > r.bottom()) r.setBottom(y);
            }
        }
    }

    // Normalize the rectangle
    return image.copy(r.normalized());
}

bool CursorTheme::haveXfixes()
{
    bool result = false;

    if (!QX11Info::isPlatformX11()) {
        return result;
    }
    int event_base, error_base;
    if (XFixesQueryExtension(QX11Info::display(), &event_base, &error_base)) {
        int major, minor;
        XFixesQueryVersion(QX11Info::display(), &major, &minor);
        result = (major >= 2);
    }

    return result;
}

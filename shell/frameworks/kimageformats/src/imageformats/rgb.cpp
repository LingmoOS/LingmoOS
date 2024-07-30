/*
    kimgio module for SGI images
    SPDX-FileCopyrightText: 2004 Melchior FRANZ <mfranz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* this code supports:
 * reading:
 *     everything, except images with 1 dimension or images with
 *     mapmode != NORMAL (e.g. dithered); Images with 16 bit
 *     precision or more than 4 layers are stripped down.
 * writing:
 *     Run Length Encoded (RLE) or Verbatim (uncompressed)
 *     (whichever is smaller)
 *
 * Please report if you come across rgb/rgba/sgi/bw files that aren't
 * recognized. Also report applications that can't deal with images
 * saved by this filter.
 */

#include "rgb_p.h"
#include "util_p.h"

#include <QList>
#include <QMap>

#include <QDebug>
#include <QImage>

class RLEData : public QList<uchar>
{
public:
    RLEData()
    {
    }
    RLEData(const uchar *d, uint l, uint o)
        : _offset(o)
    {
        for (uint i = 0; i < l; i++) {
            append(d[i]);
        }
    }
    bool operator<(const RLEData &) const;
    void write(QDataStream &s);
    uint offset() const
    {
        return _offset;
    }

private:
    uint _offset;
};

class RLEMap : public QMap<RLEData, uint>
{
public:
    RLEMap()
        : _counter(0)
        , _offset(0)
    {
    }
    uint insert(const uchar *d, uint l);
    QList<const RLEData *> vector();
    void setBaseOffset(uint o)
    {
        _offset = o;
    }

private:
    uint _counter;
    uint _offset;
};

class SGIImage
{
public:
    SGIImage(QIODevice *device);
    ~SGIImage();

    bool readImage(QImage &);
    bool writeImage(const QImage &);

private:
    enum {
        NORMAL,
        DITHERED,
        SCREEN,
        COLORMAP,
    }; // colormap
    QIODevice *_dev;
    QDataStream _stream;

    quint8 _rle;
    quint8 _bpc;
    quint16 _dim;
    quint16 _xsize;
    quint16 _ysize;
    quint16 _zsize;
    quint32 _pixmin;
    quint32 _pixmax;
    char _imagename[80];
    quint32 _colormap;

    quint32 *_starttab;
    quint32 *_lengthtab;
    QByteArray _data;
    QByteArray::Iterator _pos;
    RLEMap _rlemap;
    QList<const RLEData *> _rlevector;
    uint _numrows;

    bool readData(QImage &);
    bool getRow(uchar *dest);

    void writeHeader();
    void writeRle();
    void writeVerbatim(const QImage &);
    bool scanData(const QImage &);
    uint compact(uchar *, uchar *);
    uchar intensity(uchar);
};

SGIImage::SGIImage(QIODevice *io)
    : _starttab(nullptr)
    , _lengthtab(nullptr)
{
    _dev = io;
    _stream.setDevice(_dev);
}

SGIImage::~SGIImage()
{
    delete[] _starttab;
    delete[] _lengthtab;
}

///////////////////////////////////////////////////////////////////////////////

bool SGIImage::getRow(uchar *dest)
{
    int n;
    int i;
    if (!_rle) {
        for (i = 0; i < _xsize; i++) {
            if (_pos >= _data.end()) {
                return false;
            }
            dest[i] = uchar(*_pos);
            _pos += _bpc;
        }
        return true;
    }

    for (i = 0; i < _xsize;) {
        if (_bpc == 2) {
            _pos++;
        }
        if (_pos >= _data.end()) {
            return false;
        }
        n = *_pos & 0x7f;
        if (!n) {
            break;
        }

        if (*_pos++ & 0x80) {
            for (; i < _xsize && _pos < _data.end() && n--; i++) {
                *dest++ = *_pos;
                _pos += _bpc;
            }
        } else {
            for (; i < _xsize && n--; i++) {
                *dest++ = *_pos;
            }

            _pos += _bpc;
        }
    }
    return i == _xsize;
}

bool SGIImage::readData(QImage &img)
{
    QRgb *c;
    quint32 *start = _starttab;
    QByteArray lguard(_xsize, 0);
    uchar *line = (uchar *)lguard.data();
    unsigned x;
    unsigned y;

    if (!_rle) {
        _pos = _data.begin();
    }

    for (y = 0; y < _ysize; y++) {
        if (_rle) {
            _pos = _data.begin() + *start++;
        }
        if (!getRow(line)) {
            return false;
        }
        c = (QRgb *)img.scanLine(_ysize - y - 1);
        for (x = 0; x < _xsize; x++, c++) {
            *c = qRgb(line[x], line[x], line[x]);
        }
    }

    if (_zsize == 1) {
        return true;
    }

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            if (_rle) {
                _pos = _data.begin() + *start++;
            }
            if (!getRow(line)) {
                return false;
            }
            c = (QRgb *)img.scanLine(_ysize - y - 1);
            for (x = 0; x < _xsize; x++, c++) {
                *c = qRgb(qRed(*c), line[x], line[x]);
            }
        }

        for (y = 0; y < _ysize; y++) {
            if (_rle) {
                _pos = _data.begin() + *start++;
            }
            if (!getRow(line)) {
                return false;
            }
            c = (QRgb *)img.scanLine(_ysize - y - 1);
            for (x = 0; x < _xsize; x++, c++) {
                *c = qRgb(qRed(*c), qGreen(*c), line[x]);
            }
        }

        if (_zsize == 3) {
            return true;
        }
    }

    for (y = 0; y < _ysize; y++) {
        if (_rle) {
            _pos = _data.begin() + *start++;
        }
        if (!getRow(line)) {
            return false;
        }
        c = (QRgb *)img.scanLine(_ysize - y - 1);
        for (x = 0; x < _xsize; x++, c++) {
            *c = qRgba(qRed(*c), qGreen(*c), qBlue(*c), line[x]);
        }
    }

    return true;
}

bool SGIImage::readImage(QImage &img)
{
    qint8 u8;
    qint16 u16;
    qint32 u32;

    //     qDebug() << "reading rgb ";

    // magic
    _stream >> u16;
    if (u16 != 0x01da) {
        return false;
    }

    // verbatim/rle
    _stream >> _rle;
    //     qDebug() << (_rle ? "RLE" : "verbatim");
    if (_rle > 1) {
        return false;
    }

    // bytes per channel
    _stream >> _bpc;
    //     qDebug() << "bytes per channel: " << int(_bpc);
    if (_bpc == 1) {
        ;
    } else if (_bpc == 2) {
        //         qDebug() << "dropping least significant byte";
    } else {
        return false;
    }

    // number of dimensions
    _stream >> _dim;
    //     qDebug() << "dimensions: " << _dim;
    if (_dim < 1 || _dim > 3) {
        return false;
    }

    _stream >> _xsize >> _ysize >> _zsize >> _pixmin >> _pixmax >> u32;
    //     qDebug() << "x: " << _xsize;
    //     qDebug() << "y: " << _ysize;
    //     qDebug() << "z: " << _zsize;

    // name
    _stream.readRawData(_imagename, 80);
    _imagename[79] = '\0';

    _stream >> _colormap;
    //     qDebug() << "colormap: " << _colormap;
    if (_colormap != NORMAL) {
        return false; // only NORMAL supported
    }

    for (int i = 0; i < 404; i++) {
        _stream >> u8;
    }

    if (_dim == 1) {
        //         qDebug() << "1-dimensional images aren't supported yet";
        return false;
    }

    if (_stream.atEnd()) {
        return false;
    }

    img = imageAlloc(_xsize, _ysize, QImage::Format_RGB32);
    if (img.isNull()) {
        qWarning() << "Failed to allocate image, invalid dimensions?" << QSize(_xsize, _ysize);
        return false;
    }

    if (_zsize == 0) {
        return false;
    }

    if (_zsize == 2 || _zsize == 4) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    } else if (_zsize > 4) {
        //         qDebug() << "using first 4 of " << _zsize << " channels";
        // Only let this continue if it won't cause a int overflow later
        // this is most likely a broken file anyway
        if (_ysize > std::numeric_limits<int>::max() / _zsize) {
            return false;
        }
    }

    _numrows = _ysize * _zsize;

    if (_rle) {
        uint l;
        _starttab = new quint32[_numrows];
        for (l = 0; !_stream.atEnd() && l < _numrows; l++) {
            _stream >> _starttab[l];
            _starttab[l] -= 512 + _numrows * 2 * sizeof(quint32);
        }
        for (; l < _numrows; l++) {
            _starttab[l] = 0;
        }

        _lengthtab = new quint32[_numrows];
        for (l = 0; !_stream.atEnd() && l < _numrows; l++) {
            _stream >> _lengthtab[l];
        }
    }

    if (_stream.status() != QDataStream::Ok) {
        return false;
    }

    _data = _dev->readAll();

    // sanity check
    if (_rle) {
        for (uint o = 0; o < _numrows; o++) {
            // don't change to greater-or-equal!
            if (_starttab[o] + _lengthtab[o] > (uint)_data.size()) {
                //                 qDebug() << "image corrupt (sanity check failed)";
                return false;
            }
        }
    }

    if (!readData(img)) {
        //         qDebug() << "image corrupt (incomplete scanline)";
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void RLEData::write(QDataStream &s)
{
    for (int i = 0; i < size(); i++) {
        s << at(i);
    }
}

bool RLEData::operator<(const RLEData &b) const
{
    uchar ac;
    uchar bc;
    for (int i = 0; i < qMin(size(), b.size()); i++) {
        ac = at(i);
        bc = b[i];
        if (ac != bc) {
            return ac < bc;
        }
    }
    return size() < b.size();
}

uint RLEMap::insert(const uchar *d, uint l)
{
    RLEData data = RLEData(d, l, _offset);
    Iterator it = find(data);
    if (it != end()) {
        return it.value();
    }

    _offset += l;
    return QMap<RLEData, uint>::insert(data, _counter++).value();
}

QList<const RLEData *> RLEMap::vector()
{
    QList<const RLEData *> v(size());
    for (Iterator it = begin(); it != end(); ++it) {
        v.replace(it.value(), &it.key());
    }

    return v;
}

uchar SGIImage::intensity(uchar c)
{
    if (c < _pixmin) {
        _pixmin = c;
    }
    if (c > _pixmax) {
        _pixmax = c;
    }
    return c;
}

uint SGIImage::compact(uchar *d, uchar *s)
{
    uchar *dest = d;
    uchar *src = s;
    uchar patt;
    uchar *t;
    uchar *end = s + _xsize;
    int i;
    int n;
    while (src < end) {
        for (n = 0, t = src; t + 2 < end && !(*t == t[1] && *t == t[2]); t++) {
            n++;
        }

        while (n) {
            i = n > 126 ? 126 : n;
            n -= i;
            *dest++ = 0x80 | i;
            while (i--) {
                *dest++ = *src++;
            }
        }

        if (src == end) {
            break;
        }

        patt = *src++;
        for (n = 1; src < end && *src == patt; src++) {
            n++;
        }

        while (n) {
            i = n > 126 ? 126 : n;
            n -= i;
            *dest++ = i;
            *dest++ = patt;
        }
    }
    *dest++ = 0;
    return dest - d;
}

bool SGIImage::scanData(const QImage &img)
{
    quint32 *start = _starttab;
    QByteArray lineguard(_xsize * 2, 0);
    QByteArray bufguard(_xsize, 0);
    uchar *line = (uchar *)lineguard.data();
    uchar *buf = (uchar *)bufguard.data();
    const QRgb *c;
    unsigned x;
    unsigned y;
    uint len;

    for (y = 0; y < _ysize; y++) {
        const int yPos = _ysize - y - 1; // scanline doesn't do any sanity checking
        if (yPos >= img.height()) {
            qWarning() << "Failed to get scanline for" << yPos;
            return false;
        }

        c = reinterpret_cast<const QRgb *>(img.scanLine(yPos));

        for (x = 0; x < _xsize; x++) {
            buf[x] = intensity(qRed(*c++));
        }
        len = compact(line, buf);
        *start++ = _rlemap.insert(line, len);
    }

    if (_zsize == 1) {
        return true;
    }

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            const int yPos = _ysize - y - 1;
            if (yPos >= img.height()) {
                qWarning() << "Failed to get scanline for" << yPos;
                return false;
            }

            c = reinterpret_cast<const QRgb *>(img.scanLine(yPos));
            for (x = 0; x < _xsize; x++) {
                buf[x] = intensity(qGreen(*c++));
            }
            len = compact(line, buf);
            *start++ = _rlemap.insert(line, len);
        }

        for (y = 0; y < _ysize; y++) {
            const int yPos = _ysize - y - 1;
            if (yPos >= img.height()) {
                qWarning() << "Failed to get scanline for" << yPos;
                return false;
            }

            c = reinterpret_cast<const QRgb *>(img.scanLine(yPos));
            for (x = 0; x < _xsize; x++) {
                buf[x] = intensity(qBlue(*c++));
            }
            len = compact(line, buf);
            *start++ = _rlemap.insert(line, len);
        }

        if (_zsize == 3) {
            return true;
        }
    }

    for (y = 0; y < _ysize; y++) {
        const int yPos = _ysize - y - 1;
        if (yPos >= img.height()) {
            qWarning() << "Failed to get scanline for" << yPos;
            return false;
        }

        c = reinterpret_cast<const QRgb *>(img.scanLine(yPos));
        for (x = 0; x < _xsize; x++) {
            buf[x] = intensity(qAlpha(*c++));
        }
        len = compact(line, buf);
        *start++ = _rlemap.insert(line, len);
    }

    return true;
}

void SGIImage::writeHeader()
{
    _stream << quint16(0x01da);
    _stream << _rle << _bpc << _dim;
    _stream << _xsize << _ysize << _zsize;
    _stream << _pixmin << _pixmax;
    _stream << quint32(0);

    for (int i = 0; i < 80; i++) {
        _imagename[i] = '\0';
    }
    _stream.writeRawData(_imagename, 80);

    _stream << _colormap;
    for (int i = 0; i < 404; i++) {
        _stream << quint8(0);
    }
}

void SGIImage::writeRle()
{
    _rle = 1;
    //     qDebug() << "writing RLE data";
    writeHeader();
    uint i;

    // write start table
    for (i = 0; i < _numrows; i++) {
        _stream << quint32(_rlevector[_starttab[i]]->offset());
    }

    // write length table
    for (i = 0; i < _numrows; i++) {
        _stream << quint32(_rlevector[_starttab[i]]->size());
    }

    // write data
    for (i = 0; (int)i < _rlevector.size(); i++) {
        const_cast<RLEData *>(_rlevector[i])->write(_stream);
    }
}

void SGIImage::writeVerbatim(const QImage &img)
{
    _rle = 0;
    //     qDebug() << "writing verbatim data";
    writeHeader();

    const QRgb *c;
    unsigned x;
    unsigned y;

    for (y = 0; y < _ysize; y++) {
        c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
        for (x = 0; x < _xsize; x++) {
            _stream << quint8(qRed(*c++));
        }
    }

    if (_zsize == 1) {
        return;
    }

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
            for (x = 0; x < _xsize; x++) {
                _stream << quint8(qGreen(*c++));
            }
        }

        for (y = 0; y < _ysize; y++) {
            c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
            for (x = 0; x < _xsize; x++) {
                _stream << quint8(qBlue(*c++));
            }
        }

        if (_zsize == 3) {
            return;
        }
    }

    for (y = 0; y < _ysize; y++) {
        c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
        for (x = 0; x < _xsize; x++) {
            _stream << quint8(qAlpha(*c++));
        }
    }
}

bool SGIImage::writeImage(const QImage &image)
{
    //     qDebug() << "writing "; // TODO add filename
    QImage img = image;
    if (img.allGray()) {
        _dim = 2, _zsize = 1;
    } else {
        _dim = 3, _zsize = 3;
    }

    auto hasAlpha = img.hasAlphaChannel();
    if (hasAlpha) {
        _dim = 3, _zsize++;
    }

    if (hasAlpha && img.format() != QImage::Format_ARGB32) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    } else if (!hasAlpha && img.format() != QImage::Format_RGB32) {
        img = img.convertToFormat(QImage::Format_RGB32);
    }
    if (img.isNull()) {
        //         qDebug() << "can't convert image to depth 32";
        return false;
    }

    const int w = img.width();
    const int h = img.height();

    if (w > 65535 || h > 65535) {
        return false;
    }

    _bpc = 1;
    _xsize = w;
    _ysize = h;
    _pixmin = ~0u;
    _pixmax = 0;
    _colormap = NORMAL;
    _numrows = _ysize * _zsize;
    _starttab = new quint32[_numrows];
    _rlemap.setBaseOffset(512 + _numrows * 2 * sizeof(quint32));

    if (!scanData(img)) {
        //         qDebug() << "this can't happen";
        return false;
    }

    _rlevector = _rlemap.vector();

    long verbatim_size = _numrows * _xsize;
    long rle_size = _numrows * 2 * sizeof(quint32);
    for (int i = 0; i < _rlevector.size(); i++) {
        rle_size += _rlevector[i]->size();
    }

    if (verbatim_size <= rle_size) {
        writeVerbatim(img);
    } else {
        writeRle();
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

RGBHandler::RGBHandler()
{
}

bool RGBHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("rgb");
        return true;
    }
    return false;
}

bool RGBHandler::read(QImage *outImage)
{
    SGIImage sgi(device());
    return sgi.readImage(*outImage);
}

bool RGBHandler::write(const QImage &image)
{
    SGIImage sgi(device());
    return sgi.writeImage(image);
}

bool RGBHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("RGBHandler::canRead() called with no device");
        return false;
    }

    const qint64 oldPos = device->pos();
    const QByteArray head = device->readLine(64);
    int readBytes = head.size();

    if (device->isSequential()) {
        while (readBytes > 0) {
            device->ungetChar(head[readBytes-- - 1]);
        }

    } else {
        device->seek(oldPos);
    }

    return head.size() >= 4 && head.startsWith("\x01\xda") && (head[2] == 0 || head[2] == 1) && (head[3] == 1 || head[3] == 2);
}

///////////////////////////////////////////////////////////////////////////////

QImageIOPlugin::Capabilities RGBPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "rgb" || format == "rgba" || format == "bw" || format == "sgi") {
        return Capabilities(CanRead | CanWrite);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && RGBHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QImageIOHandler *RGBPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new RGBHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_rgb_p.cpp"

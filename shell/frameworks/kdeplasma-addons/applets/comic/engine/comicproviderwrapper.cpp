/*
 *   SPDX-FileCopyrightText: 2008 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "comicproviderwrapper.h"
#include "comic_debug.h"
#include "comicproviderkross.h"

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <QFile>
#include <QFileInfo>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QJSValueList>
#include <QPainter>
#include <QStandardPaths>
#include <QStringDecoder>
#include <QTimer>
#include <QUrl>

QStringList ComicProviderWrapper::mExtensions;

ImageWrapper::ImageWrapper(QObject *parent, const QByteArray &data)
    : QObject(parent)
    , mImage(QImage::fromData(data))
    , mRawData(data)
{
    resetImageReader();
}

QImage ImageWrapper::image() const
{
    return mImage;
}

void ImageWrapper::setImage(const QImage &image)
{
    mImage = image;
    mRawData.clear();

    resetImageReader();
}

QByteArray ImageWrapper::rawData() const
{
    if (mRawData.isNull()) {
        QBuffer buffer(&mRawData);
        mImage.save(&buffer);
    }

    return mRawData;
}

void ImageWrapper::setRawData(const QByteArray &rawData)
{
    mRawData = rawData;
    mImage = QImage::fromData(mRawData);

    resetImageReader();
}

void ImageWrapper::resetImageReader()
{
    if (mBuffer.isOpen()) {
        mBuffer.close();
    }
    rawData(); // to update the rawData if needed
    mBuffer.setBuffer(&mRawData);
    mBuffer.open(QIODevice::ReadOnly);
    mImageReader.setDevice(&mBuffer);
}

int ImageWrapper::imageCount() const
{
    return mImageReader.imageCount();
}

QImage ImageWrapper::read()
{
    return mImageReader.read();
}

DateWrapper::DateWrapper(const QDate &date)
    : mDate(date)
{
}

QDate DateWrapper::date() const
{
    return mDate;
}

void DateWrapper::setDate(const QDate &date)
{
    mDate = date;
}

QDate DateWrapper::fromVariant(const QVariant &variant)
{
    if (variant.typeId() == QMetaType::QDate || variant.typeId() == QMetaType::QDateTime) {
        return variant.toDate();
    } else if (variant.typeId() == QMetaType::QString) {
        return QDate::fromString(variant.toString(), Qt::ISODate);
    } else {
        if (variant.canConvert<DateWrapper>()) {
            return variant.value<DateWrapper>().date();
        }
    }
    return QDate();
}

DateWrapper DateWrapper::addDays(int ndays)
{
    return DateWrapper(mDate.addDays(ndays));
}

DateWrapper DateWrapper::addMonths(int nmonths)
{
    return DateWrapper(mDate.addMonths(nmonths));
}

DateWrapper DateWrapper::addYears(int nyears)
{
    return DateWrapper(mDate.addYears(nyears));
}

int DateWrapper::day() const
{
    return mDate.day();
}

int DateWrapper::dayOfWeek() const
{
    return mDate.dayOfWeek();
}

int DateWrapper::dayOfYear() const
{
    return mDate.dayOfYear();
}

int DateWrapper::daysInMonth() const
{
    return mDate.daysInMonth();
}

int DateWrapper::daysInYear() const
{
    return mDate.daysInYear();
}

int DateWrapper::daysTo(const QVariant d) const
{
    return mDate.daysTo(fromVariant(d));
}

bool DateWrapper::isNull() const
{
    return mDate.isNull();
}

bool DateWrapper::isValid() const
{
    return mDate.isValid();
}

int DateWrapper::month() const
{
    return mDate.month();
}

bool DateWrapper::setDate(int year, int month, int day)
{
    return mDate.setDate(year, month, day);
}

int DateWrapper::toJulianDay() const
{
    return mDate.toJulianDay();
}

QString DateWrapper::toString(const QString &format) const
{
    return mDate.toString(format);
}

QString DateWrapper::toString(int format) const
{
    return mDate.toString((Qt::DateFormat)format);
}

int DateWrapper::weekNumber() const
{
    return mDate.weekNumber();
}

int DateWrapper::year() const
{
    return mDate.year();
}

StaticDateWrapper::StaticDateWrapper(QObject *parent)
    : QObject(parent)
{
}

DateWrapper StaticDateWrapper::currentDate()
{
    return DateWrapper(QDate::currentDate());
}

DateWrapper StaticDateWrapper::fromJulianDay(int jd)
{
    return DateWrapper(QDate::fromJulianDay(jd));
}

DateWrapper StaticDateWrapper::fromString(const QString &string, int format)
{
    return DateWrapper(QDate::fromString(string, (Qt::DateFormat)format));
}

DateWrapper StaticDateWrapper::fromString(const QString &string, const QString &format)
{
    return DateWrapper(QDate::fromString(string, format));
}

bool StaticDateWrapper::isLeapYear(int year)
{
    return QDate::isLeapYear(year);
}

bool StaticDateWrapper::isValid(int year, int month, int day)
{
    return QDate::isValid(year, month, day);
}

QString StaticDateWrapper::longDayName(int weekday)
{
    return QLocale::system().dayName(weekday, QLocale::LongFormat);
}

QString StaticDateWrapper::longMonthName(int month)
{
    return QLocale::system().monthName(month, QLocale::LongFormat);
}

QString StaticDateWrapper::shortDayName(int weekday)
{
    return QLocale::system().dayName(weekday, QLocale::ShortFormat);
}

QString StaticDateWrapper::shortMonthName(int month)
{
    return QLocale::system().monthName(month, QLocale::ShortFormat);
}

ComicProviderWrapper::ComicProviderWrapper(ComicProviderKross *parent)
    : QObject(parent)
    , mProvider(parent)
    , mKrossImage(nullptr)
    , mPackage(nullptr)
    , mRequests(0)
    , mIdentifierSpecified(false)
    , mIsLeftToRight(true)
    , mIsTopToBottom(true)
{
    QTimer::singleShot(0, this, &ComicProviderWrapper::init);
}

ComicProviderWrapper::~ComicProviderWrapper()
{
    delete mPackage;
}

void ComicProviderWrapper::init()
{
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                QLatin1String("plasma/comics/") + mProvider->pluginName() + QLatin1Char('/'),
                                                QStandardPaths::LocateDirectory);
    qCDebug(PLASMA_COMIC) << "ComicProviderWrapper::init() package is" << mProvider->pluginName() << " at " << path;

    if (!path.isEmpty()) {
        mPackage = new KPackage::Package(KPackage::PackageLoader::self()->loadPackageStructure(QStringLiteral("Plasma/Comic")));
        mPackage->setPath(path);

        if (mPackage->isValid()) {
            m_engine = new QJSEngine(this);
            QString mainscript = mPackage->filePath("mainscript");
            QFile f(mainscript);
            if (f.open(QFile::ReadOnly)) {
                m_engine->globalObject().setProperty("Comic", m_engine->newQMetaObject(&ComicProviderWrapper::staticMetaObject));
                auto obj = m_engine->newQObject(this);

                // If we set the comic in the global object we can not access the staticMetaObject
                // consequently the values have to be written manually
                obj.setProperty("Page", ComicProvider::Page);
                obj.setProperty("Image", ComicProvider::Image);
                obj.setProperty("User", ComicProvider::User);
                obj.setProperty("Left", ComicProviderWrapper::Left);
                obj.setProperty("Top", ComicProviderWrapper::Top);
                obj.setProperty("Right", ComicProviderWrapper::Right);
                obj.setProperty("Bottom", ComicProviderWrapper::Bottom);
                obj.setProperty("DateIdentifier", (int)IdentifierType::DateIdentifier);
                obj.setProperty("NumberIdentifier", (int)IdentifierType::NumberIdentifier);
                obj.setProperty("StringIdentifier", (int)IdentifierType::StringIdentifier);

                m_engine->globalObject().setProperty("comic", obj);
                m_engine->globalObject().setProperty("date", m_engine->newQObject(new StaticDateWrapper(this)));
                m_engine->evaluate("var print = comic.print");
                mIdentifierSpecified = !mProvider->isCurrent();
                m_engine->evaluate(f.readAll(), mainscript);
                QJSValueIterator it(m_engine->globalObject());
                while (it.hasNext()) {
                    it.next();
                    if (it.value().isCallable()) {
                        mFunctions << it.name();
                    }
                }
                setIdentifierToDefault();
                callFunction(QStringLiteral("init"));
            }
        }
    }
}

IdentifierType ComicProviderWrapper::identifierType() const
{
    IdentifierType result = IdentifierType::StringIdentifier;
    const QString type = mProvider->description().value(QLatin1String("X-KDE-PlasmaComicProvider-SuffixType"));
    if (type == QLatin1String("Date")) {
        result = IdentifierType::DateIdentifier;
    } else if (type == QLatin1String("Number")) {
        result = IdentifierType::NumberIdentifier;
    } else if (type == QLatin1String("String")) {
        result = IdentifierType::StringIdentifier;
    }
    return result;
}

QImage ComicProviderWrapper::comicImage()
{
    ImageWrapper *img = qobject_cast<ImageWrapper *>(callFunction(QLatin1String("image")).value<QObject *>());
    if (functionCalled() && img) {
        return img->image();
    }
    if (mKrossImage) {
        return mKrossImage->image();
    }
    return QImage();
}

QJSValue ComicProviderWrapper::identifierToScript(const QVariant &identifier)
{
    if (identifierType() == IdentifierType::DateIdentifier && identifier.typeId() != QMetaType::Bool) {
        return m_engine->toScriptValue(DateWrapper(identifier.toDate()));
    }
    return m_engine->toScriptValue(identifier);
}

QVariant ComicProviderWrapper::identifierFromScript(const QJSValue &identifier) const
{
    if (identifier.toVariant().canConvert<DateWrapper>()) {
        return identifier.toVariant().value<DateWrapper>().date();
    }
    return identifier.toVariant();
}

void ComicProviderWrapper::checkIdentifier(QVariant *identifier)
{
    switch (identifierType()) {
    case IdentifierType::DateIdentifier:
        if (!mLastIdentifier.isNull() && !identifier->isNull() && (!mIdentifierSpecified || identifier->toDate() > mLastIdentifier.toDate())) {
            *identifier = mLastIdentifier;
        }
        if (!mFirstIdentifier.isNull() && !identifier->isNull() && identifier->toDate() < mFirstIdentifier.toDate()) {
            *identifier = mFirstIdentifier;
        }
        break;
    case IdentifierType::NumberIdentifier:
        if (!mLastIdentifier.isNull() && !identifier->isNull() && (!mIdentifierSpecified || identifier->toInt() > mLastIdentifier.toInt())) {
            *identifier = mLastIdentifier;
        }
        if (!mFirstIdentifier.isNull() && !identifier->isNull() && identifier->toInt() < mFirstIdentifier.toInt()) {
            *identifier = mFirstIdentifier;
        }
        break;
    case IdentifierType::StringIdentifier:
        if (!mLastIdentifier.isNull() && !mLastIdentifier.toString().isEmpty() && !mIdentifierSpecified) {
            *identifier = mLastIdentifier;
        }
        break;
    }
}

void ComicProviderWrapper::setIdentifierToDefault()
{
    switch (identifierType()) {
    case IdentifierType::DateIdentifier:
        mIdentifier = mProvider->requestedDate();
        mLastIdentifier = QDate::currentDate();
        break;
    case IdentifierType::NumberIdentifier:
        mIdentifier = mProvider->requestedNumber();
        mFirstIdentifier = 1;
        break;
    case IdentifierType::StringIdentifier:
        mIdentifier = mProvider->requestedString();
        break;
    }
}

bool ComicProviderWrapper::identifierSpecified() const
{
    return mIdentifierSpecified;
}

bool ComicProviderWrapper::isLeftToRight() const
{
    return mIsLeftToRight;
}

void ComicProviderWrapper::setLeftToRight(bool ltr)
{
    mIsLeftToRight = ltr;
}

bool ComicProviderWrapper::isTopToBottom() const
{
    return mIsTopToBottom;
}

void ComicProviderWrapper::setTopToBottom(bool ttb)
{
    mIsTopToBottom = ttb;
}

QString ComicProviderWrapper::textCodec() const
{
    return QString::fromLatin1(mTextCodec);
}

void ComicProviderWrapper::setTextCodec(const QString &textCodec)
{
    mTextCodec = textCodec.toLatin1();
}

QString ComicProviderWrapper::comicAuthor() const
{
    return mProvider->comicAuthor();
}

void ComicProviderWrapper::setComicAuthor(const QString &author)
{
    mProvider->setComicAuthor(author);
}

QString ComicProviderWrapper::websiteUrl() const
{
    return mWebsiteUrl;
}

void ComicProviderWrapper::setWebsiteUrl(const QString &websiteUrl)
{
    mWebsiteUrl = websiteUrl;
}

QString ComicProviderWrapper::shopUrl() const
{
    return mShopUrl;
}

void ComicProviderWrapper::setShopUrl(const QString &shopUrl)
{
    mShopUrl = shopUrl;
}

QString ComicProviderWrapper::title() const
{
    return mTitle;
}

void ComicProviderWrapper::setTitle(const QString &title)
{
    mTitle = title;
}

QString ComicProviderWrapper::additionalText() const
{
    return mAdditionalText;
}

void ComicProviderWrapper::setAdditionalText(const QString &additionalText)
{
    mAdditionalText = additionalText;
}

QJSValue ComicProviderWrapper::identifier()
{
    return identifierToScript(mIdentifier);
}

void ComicProviderWrapper::setIdentifier(const QJSValue &identifier)
{
    mIdentifier = identifierFromScript(identifier);
    checkIdentifier(&mIdentifier);
}

QJSValue ComicProviderWrapper::nextIdentifier()
{
    return identifierToScript(mNextIdentifier);
}

void ComicProviderWrapper::setNextIdentifier(const QJSValue &nextIdentifier)
{
    mNextIdentifier = identifierFromScript(nextIdentifier);
    if (mNextIdentifier == mIdentifier) {
        mNextIdentifier.clear();
        qCWarning(PLASMA_COMIC) << "Next identifier is the same as the current one, clearing next identifier.";
    }
}

QJSValue ComicProviderWrapper::previousIdentifier()
{
    return identifierToScript(mPreviousIdentifier);
}

void ComicProviderWrapper::setPreviousIdentifier(const QJSValue &previousIdentifier)
{
    mPreviousIdentifier = identifierFromScript(previousIdentifier);
    if (mPreviousIdentifier == mIdentifier) {
        mPreviousIdentifier.clear();
        qCWarning(PLASMA_COMIC) << "Previous identifier is the same as the current one, clearing previous identifier.";
    }
}

QJSValue ComicProviderWrapper::firstIdentifier()
{
    return identifierToScript(mFirstIdentifier);
}

void ComicProviderWrapper::setFirstIdentifier(const QJSValue &firstIdentifier)
{
    switch (identifierType()) {
    case IdentifierType::DateIdentifier:
        mProvider->setFirstStripDate(DateWrapper::fromVariant(QVariant::fromValue(firstIdentifier.toQObject())));
        break;
    case IdentifierType::NumberIdentifier:
        mProvider->setFirstStripNumber(firstIdentifier.toInt());
        break;
    case IdentifierType::StringIdentifier:
        break;
    }
    mFirstIdentifier = identifierFromScript(firstIdentifier);
    checkIdentifier(&mIdentifier);
}

QJSValue ComicProviderWrapper::lastIdentifier()
{
    return identifierToScript(mLastIdentifier);
}

void ComicProviderWrapper::setLastIdentifier(const QJSValue &lastIdentifier)
{
    mLastIdentifier = identifierFromScript(lastIdentifier);
    checkIdentifier(&mIdentifier);
}

QVariant ComicProviderWrapper::identifierVariant() const
{
    return mIdentifier;
}

QVariant ComicProviderWrapper::firstIdentifierVariant() const
{
    return mFirstIdentifier;
}

QVariant ComicProviderWrapper::lastIdentifierVariant() const
{
    return mLastIdentifier;
}

QVariant ComicProviderWrapper::nextIdentifierVariant() const
{
    // either handle both previousIdentifier and nextIdentifier or handle none
    if (mPreviousIdentifier.isNull() && mNextIdentifier.isNull()) {
        switch (identifierType()) {
        case IdentifierType::DateIdentifier:
            if ((mLastIdentifier.isNull() && mIdentifier.toDate() < QDate::currentDate())
                || (!mLastIdentifier.isNull() && mIdentifier.toDate() < mLastIdentifier.toDate())) {
                return mIdentifier.toDate().addDays(1);
            } else {
                return false;
            }
        case IdentifierType::NumberIdentifier:
            if (mLastIdentifier.isNull() || mIdentifier.toInt() < mLastIdentifier.toInt()) {
                return mIdentifier.toInt() + 1;
            } else {
                return false;
            }
        case IdentifierType::StringIdentifier:
            break;
        }
        // check if the nextIdentifier is correct
    } else if (!mNextIdentifier.isNull()) {
        // no nextIdentifier if mIdentifier == mLastIdentifier or if no identifier has been specified
        switch (identifierType()) {
        case IdentifierType::DateIdentifier:
            if ((!mLastIdentifier.isNull() && (mIdentifier.toDate() == mLastIdentifier.toDate())) || !mIdentifierSpecified) {
                return false;
            }
            break;
        case IdentifierType::NumberIdentifier:
            if ((!mLastIdentifier.isNull() && (mIdentifier.toInt() == mLastIdentifier.toInt())) || !mIdentifierSpecified) {
                return false;
            }
            break;
        case IdentifierType::StringIdentifier:
            if (!mIdentifierSpecified) {
                return false;
            }
            break;
        }
    }
    return mNextIdentifier;
}

QVariant ComicProviderWrapper::previousIdentifierVariant() const
{
    // either handle both previousIdentifier and nextIdentifier or handle none
    if (mPreviousIdentifier.isNull() && mNextIdentifier.isNull()) {
        switch (identifierType()) {
        case IdentifierType::DateIdentifier:
            if (mFirstIdentifier.isNull() || mIdentifier.toDate() > mFirstIdentifier.toDate()) {
                return mIdentifier.toDate().addDays(-1);
            } else {
                return false;
            }
        case IdentifierType::NumberIdentifier:
            if ((mFirstIdentifier.isNull() && mIdentifier.toInt() > 1) || (!mFirstIdentifier.isNull() && mIdentifier.toInt() > mFirstIdentifier.toInt())) {
                return mIdentifier.toInt() - 1;
            } else {
                return false;
            }
        case IdentifierType::StringIdentifier:
            break;
        }
    } else if (!mPreviousIdentifier.isNull()) {
        // no previousIdentifier if mIdentifier == mFirstIdentifier
        switch (identifierType()) {
        case IdentifierType::DateIdentifier:
            if (!mFirstIdentifier.isNull() && (mIdentifier.toDate() == mFirstIdentifier.toDate())) {
                return false;
            }
            break;
        case IdentifierType::NumberIdentifier:
            if (!mFirstIdentifier.isNull() && (mIdentifier.toInt() == mFirstIdentifier.toInt())) {
                return false;
            }
            break;
        case IdentifierType::StringIdentifier:
            break;
        }
    }
    return mPreviousIdentifier;
}

void ComicProviderWrapper::pageRetrieved(int id, const QByteArray &data)
{
    --mRequests;
    if (id == ComicProvider::Image) {
        mKrossImage = new ImageWrapper(this, data);
        callFunction(QLatin1String("pageRetrieved"), {id, m_engine->newQObject(mKrossImage)});
        if (mRequests < 1) { // Don't finish if we still have pageRequests
            finished();
        }
    } else {
        QStringDecoder codec(mTextCodec);
        if (mTextCodec.isEmpty() || !codec.isValid()) {
            codec = QStringDecoder(QStringDecoder::encodingForHtml(data).value_or(QStringDecoder::Utf8));
        }
        QString html = codec.decode(data);

        callFunction(QLatin1String("pageRetrieved"), {id, html});
    }
}

void ComicProviderWrapper::pageError(int id, const QString &message)
{
    --mRequests;
    callFunction(QLatin1String("pageError"), {id, message});
    if (!functionCalled()) {
        Q_EMIT mProvider->error(mProvider);
    }
}

void ComicProviderWrapper::redirected(int id, const QUrl &newUrl)
{
    --mRequests;
    callFunction(QLatin1String("redirected"), {id, newUrl.toString()});
    if (mRequests < 1) { // Don't finish while there are still requests
        finished();
    }
}

void ComicProviderWrapper::finished() const
{
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Author").leftJustified(22, QLatin1Char('.')) << comicAuthor();
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Website URL").leftJustified(22, QLatin1Char('.')) << mWebsiteUrl;
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Shop URL").leftJustified(22, QLatin1Char('.')) << mShopUrl;
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Title").leftJustified(22, QLatin1Char('.')) << mTitle;
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Additional Text").leftJustified(22, QLatin1Char('.')) << mAdditionalText;
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Identifier").leftJustified(22, QLatin1Char('.')) << mIdentifier;
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("First Identifier").leftJustified(22, QLatin1Char('.')) << mFirstIdentifier;
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Last Identifier").leftJustified(22, QLatin1Char('.')) << mLastIdentifier;
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Next Identifier").leftJustified(22, QLatin1Char('.')) << mNextIdentifier;
    qCDebug(PLASMA_COMIC) << QString::fromLatin1("Previous Identifier").leftJustified(22, QLatin1Char('.')) << mPreviousIdentifier;
    Q_EMIT mProvider->finished(mProvider);
}

void ComicProviderWrapper::error() const
{
    Q_EMIT mProvider->error(mProvider);
}

void ComicProviderWrapper::requestPage(const QString &url, int id, const QVariantMap &infos)
{
    QMap<QString, QString> map;

    for (auto it = infos.begin(), end = infos.end(); it != end; ++it) {
        map[it.key()] = it.value().toString();
    }
    mProvider->requestPage(QUrl(url), id, map);
    ++mRequests;
}

void ComicProviderWrapper::requestRedirectedUrl(const QString &url, int id, const QVariantMap &infos)
{
    QMap<QString, QString> map;

    for (auto it = infos.begin(), end = infos.end(); it != end; ++it) {
        map[it.key()] = it.value().toString();
    }
    mProvider->requestRedirectedUrl(QUrl(url), id, map);
    ++mRequests;
}

bool ComicProviderWrapper::functionCalled() const
{
    return mFuncFound;
}

QVariant ComicProviderWrapper::callFunction(const QString &name, const QJSValueList &args)
{
    if (m_engine) {
        mFuncFound = mFunctions.contains(name);
        if (mFuncFound) {
            auto val = m_engine->globalObject().property(name).call(args);
            if (val.isError()) {
                qCWarning(PLASMA_COMIC) << "Error when calling function" << name << "with arguments" << QVariant::fromValue(args) << val.toString();
                return QVariant();
            } else {
                return val.toVariant();
            }
        }
    }
    return QVariant();
}

void ComicProviderWrapper::combine(const QVariant &image, PositionType position)
{
    if (!mKrossImage) {
        return;
    }

    QImage header;
    if (image.typeId() == QMetaType::QString) {
        const QString path(mPackage->filePath("images", image.toString()));
        if (QFile::exists(path)) {
            header = QImage(path);
        } else {
            return;
        }
    } else {
        ImageWrapper *img = qobject_cast<ImageWrapper *>(image.value<QObject *>());
        if (img) {
            header = img->image();
        } else {
            return;
        }
    }
    const QImage comic = mKrossImage->image();
    int height = 0;
    int width = 0;

    switch (position) {
    case Top:
    case Bottom:
        height = header.height() + comic.height();
        width = (header.width() >= comic.width()) ? header.width() : comic.width();
        break;
    case Left:
    case Right:
        height = (header.height() >= comic.height()) ? header.height() : comic.height();
        width = header.width() + comic.width();
        break;
    }

    QImage img = QImage(QSize(width, height), QImage::Format_RGB32);
    img.fill(header.pixel(QPoint(0, 0)));

    QPainter painter(&img);

    // center and draw the Images
    QPoint headerPos;
    QPoint comicPos;

    switch (position) {
    case Top:
        headerPos = QPoint(((width - header.width()) / 2), 0);
        comicPos = QPoint(((width - comic.width()) / 2), header.height());
        break;
    case Bottom:
        headerPos = QPoint(((width - header.width()) / 2), comic.height());
        comicPos = QPoint(((width - comic.width()) / 2), 0);
        break;
    case Left:
        headerPos = QPoint(0, ((height - header.height()) / 2));
        comicPos = QPoint(header.width(), ((height - comic.height()) / 2));
        break;
    case Right:
        headerPos = QPoint(comic.width(), ((height - header.height()) / 2));
        comicPos = QPoint(0, ((height - comic.height()) / 2));
        break;
    }
    painter.drawImage(headerPos, header);
    painter.drawImage(comicPos, comic);
    mKrossImage->setImage(img);
}

QObject *ComicProviderWrapper::image()
{
    return qobject_cast<QObject *>(mKrossImage);
}

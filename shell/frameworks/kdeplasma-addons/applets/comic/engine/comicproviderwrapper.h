/*
 *   SPDX-FileCopyrightText: 2008 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef COMICPROVIDERWRAPPER_H
#define COMICPROVIDERWRAPPER_H

#include "comicprovider.h"
#include "types.h"

#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QImageReader>
#include <QJSValue>

namespace KPackage
{
class Package;
}
class ComicProviderKross;
class QJSEngine;

class ImageWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage)
    Q_PROPERTY(QByteArray rawData READ rawData WRITE setRawData)
public:
    explicit ImageWrapper(QObject *parent = nullptr, const QByteArray &image = QByteArray());

    QImage image() const;
    /**
     * Sets the image, rawData is changed to the new set image
     */
    void setImage(const QImage &image);
    QByteArray rawData() const;

    /**
     * Sets the rawData, image is changed to the new rawData
     */
    void setRawData(const QByteArray &rawData);

public Q_SLOTS:
    /**
     * Returns the numbers of images contained in the image
     * 0 if there is just one image, > 0 if the image format supports animation (the number of frames),
     * -1 if there was an error
     * @since 4600
     */
    int imageCount() const;

    /**
     * Returns an image, if it did not work an null image is returned
     * For animations returns the next frame upon each call, if there are no frames left returns a null image
     * @see imageCount()
     * @since 4600
     */
    QImage read();

private:
    void resetImageReader();

private:
    QImage mImage;
    mutable QByteArray mRawData;
    QBuffer mBuffer;
    QImageReader mImageReader;
};

class DateWrapper
{
    Q_GADGET
    Q_PROPERTY(QDate date READ date WRITE setDate)
public:
    explicit DateWrapper(const QDate &date = QDate());

    QDate date() const;
    void setDate(const QDate &date);
    static QDate fromVariant(const QVariant &variant);

    Q_INVOKABLE QString toString(const QString &format) const;
    Q_INVOKABLE QString toString(int format = 0) const;

public Q_SLOTS:
    DateWrapper addDays(int ndays);
    DateWrapper addMonths(int nmonths);
    DateWrapper addYears(int nyears);
    int day() const;
    int dayOfWeek() const;
    int dayOfYear() const;
    int daysInMonth() const;
    int daysInYear() const;
    int daysTo(const QVariant d) const;
    bool isNull() const;
    bool isValid() const;
    int month() const;
    bool setDate(int year, int month, int day);
    int toJulianDay() const;
    int weekNumber() const;
    int year() const;

private:
    QDate mDate;
};

class StaticDateWrapper : public QObject
{
    Q_OBJECT
public:
    explicit StaticDateWrapper(QObject *parent = nullptr);

public Q_SLOTS:
    DateWrapper currentDate();
    DateWrapper fromJulianDay(int jd);
    DateWrapper fromString(const QString &string, int format = Qt::TextDate);
    DateWrapper fromString(const QString &string, const QString &format);
    bool isLeapYear(int year);
    bool isValid(int year, int month, int day);
    QString longDayName(int weekday);
    QString longMonthName(int month);
    QString shortDayName(int weekday);
    QString shortMonthName(int month);
};

class ComicProviderWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool identifierSpecified READ identifierSpecified)
    Q_PROPERTY(QString textCodec READ textCodec WRITE setTextCodec)
    Q_PROPERTY(QString comicAuthor READ comicAuthor WRITE setComicAuthor)
    Q_PROPERTY(QString websiteUrl READ websiteUrl WRITE setWebsiteUrl)
    Q_PROPERTY(QString shopUrl READ shopUrl WRITE setShopUrl)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString additionalText READ additionalText WRITE setAdditionalText)
    Q_PROPERTY(QJSValue identifier READ identifier WRITE setIdentifier)
    Q_PROPERTY(QJSValue nextIdentifier READ nextIdentifier WRITE setNextIdentifier)
    Q_PROPERTY(QJSValue previousIdentifier READ previousIdentifier WRITE setPreviousIdentifier)
    Q_PROPERTY(QJSValue firstIdentifier READ firstIdentifier WRITE setFirstIdentifier)
    Q_PROPERTY(QJSValue lastIdentifier READ lastIdentifier WRITE setLastIdentifier)
    Q_PROPERTY(bool isLeftToRight READ isLeftToRight WRITE setLeftToRight)
    Q_PROPERTY(bool isTopToBottom READ isTopToBottom WRITE setTopToBottom)
    Q_PROPERTY(int apiVersion READ apiVersion)
public:
    enum PositionType {
        Left = 0,
        Top,
        Right,
        Bottom,
    };
    Q_ENUM(PositionType)

    enum RedirectedUrlType {
        PreviousUrl = 0,
        CurrentUrl = 1,
        NextUrl = 2,
        FirstUrl = 3,
        LastUrl = 4,
        UserUrl = 10,
    };
    Q_ENUM(RedirectedUrlType)

    explicit ComicProviderWrapper(ComicProviderKross *parent);
    ~ComicProviderWrapper() override;

    int apiVersion() const
    {
        return 4600;
    }

    Q_INVOKABLE void print(const QJSValue &str)
    {
        qWarning() << str.toString();
    }

    IdentifierType identifierType() const;
    QImage comicImage();
    void pageRetrieved(int id, const QByteArray &data);
    void pageError(int id, const QString &message);
    void redirected(int id, const QUrl &newUrl);

    bool identifierSpecified() const;
    QString textCodec() const;
    void setTextCodec(const QString &textCodec);
    QString comicAuthor() const;
    void setComicAuthor(const QString &author);
    QString websiteUrl() const;
    void setWebsiteUrl(const QString &websiteUrl);
    QString shopUrl() const;
    void setShopUrl(const QString &shopUrl);
    QString title() const;
    void setTitle(const QString &title);
    QString additionalText() const;
    void setAdditionalText(const QString &additionalText);
    QJSValue identifier();
    void setIdentifier(const QJSValue &identifier);
    QJSValue nextIdentifier();
    void setNextIdentifier(const QJSValue &nextIdentifier);
    QJSValue previousIdentifier();
    void setPreviousIdentifier(const QJSValue &previousIdentifier);
    QJSValue firstIdentifier();
    void setFirstIdentifier(const QJSValue &firstIdentifier);
    QJSValue lastIdentifier();
    void setLastIdentifier(const QJSValue &lastIdentifier);
    bool isLeftToRight() const;
    void setLeftToRight(bool ltr);
    bool isTopToBottom() const;
    void setTopToBottom(bool ttb);

    QVariant identifierVariant() const;
    QVariant firstIdentifierVariant() const;
    QVariant lastIdentifierVariant() const;
    QVariant nextIdentifierVariant() const;
    QVariant previousIdentifierVariant() const;

public Q_SLOTS:
    void finished() const;
    void error() const;

    void requestPage(const QString &url, int id, const QVariantMap &infos = QVariantMap());
    void requestRedirectedUrl(const QString &url, int id, const QVariantMap &infos = QVariantMap());
    void combine(const QVariant &image, PositionType position = Top);
    QObject *image();

    void init();

protected:
    QVariant callFunction(const QString &name, const QJSValueList &args = {});
    bool functionCalled() const;
    QJSValue identifierToScript(const QVariant &identifier);
    QVariant identifierFromScript(const QJSValue &identifier) const;
    void setIdentifierToDefault();
    void checkIdentifier(QVariant *identifier);

private:
    QJSEngine *m_engine = nullptr;
    ComicProviderKross *mProvider;
    QStringList mFunctions;
    bool mFuncFound;
    ImageWrapper *mKrossImage;
    static QStringList mExtensions;
    KPackage::Package *mPackage;

    QByteArray mTextCodec;
    QString mWebsiteUrl;
    QString mShopUrl;
    QString mTitle;
    QString mAdditionalText;
    QVariant mIdentifier;
    QVariant mNextIdentifier;
    QVariant mPreviousIdentifier;
    QVariant mFirstIdentifier;
    QVariant mLastIdentifier;
    int mRequests;
    bool mIdentifierSpecified;
    bool mIsLeftToRight;
    bool mIsTopToBottom;
};

#endif

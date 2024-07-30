/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMIC_H
#define COMIC_H

#include <QDate>
#include <QUrl>

#include <Plasma/Applet>

#include "activecomicmodel.h"
#include "comicdata.h"
#include "engine/comic.h"

class CheckNewStrips;
class ComicModel;
class ConfigWidget;
class QAction;
class KJob;
class QAction;
class QSortFilterProxyModel;
class QTimer;
class SavingDir;

class ComicApplet : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QObject *comicsModel READ comicsModel NOTIFY comicModelChanged)
    Q_PROPERTY(QObject *availableComicsModel READ availableComicsModel CONSTANT)
    Q_PROPERTY(bool showComicUrl READ showComicUrl WRITE setShowComicUrl NOTIFY showComicUrlChanged)
    Q_PROPERTY(bool showComicAuthor READ showComicAuthor WRITE setShowComicAuthor NOTIFY showComicAuthorChanged)
    Q_PROPERTY(bool showComicTitle READ showComicTitle WRITE setShowComicTitle NOTIFY showComicTitleChanged)
    Q_PROPERTY(bool showComicIdentifier READ showComicIdentifier WRITE setShowComicIdentifier NOTIFY showComicIdentifierChanged)
    Q_PROPERTY(bool showErrorPicture READ showErrorPicture WRITE setShowErrorPicture NOTIFY showErrorPictureChanged)
    Q_PROPERTY(bool arrowsOnHover READ arrowsOnHover WRITE setArrowsOnHover NOTIFY arrowsOnHoverChanged)
    Q_PROPERTY(bool middleClick READ middleClick WRITE setMiddleClick NOTIFY middleClickChanged)
    Q_PROPERTY(QVariantMap comicData READ comicData NOTIFY comicDataChanged)
    Q_PROPERTY(bool showActualSize READ showActualSize WRITE setShowActualSize NOTIFY showActualSizeChanged)
    Q_PROPERTY(QStringList tabIdentifiers READ tabIdentifiers WRITE setTabIdentifiers NOTIFY tabIdentifiersChanged)
    Q_PROPERTY(int checkNewComicStripsInterval READ checkNewComicStripsInterval WRITE setCheckNewComicStripsInterval NOTIFY checkNewComicStripsIntervalChanged)
    Q_PROPERTY(int maxComicLimit READ maxComicLimit WRITE setMaxComicLimit NOTIFY maxComicLimitChanged)

public:
    ComicApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~ComicApplet() override;

    void init() override;
    QList<QAction *> contextualActions() override;

    // For QML
    QObject *comicsModel() const;
    QObject *availableComicsModel() const;
    QVariantMap comicData() const;

    QStringList tabIdentifiers() const;
    void setTabIdentifiers(const QStringList &tabs);

    bool showComicUrl() const;
    void setShowComicUrl(bool show);

    bool showComicAuthor() const;
    void setShowComicAuthor(bool show);

    bool showComicTitle() const;
    void setShowComicTitle(bool show);

    bool showComicIdentifier() const;
    void setShowComicIdentifier(bool show);

    bool showErrorPicture() const;
    void setShowErrorPicture(bool show);

    bool arrowsOnHover() const;
    void setArrowsOnHover(bool show);

    bool middleClick() const;
    void setMiddleClick(bool show);

    bool showActualSize() const;
    void setShowActualSize(bool show);

    int checkNewComicStripsInterval() const;
    void setCheckNewComicStripsInterval(int interval);

    void setMaxComicLimit(int limit);
    int maxComicLimit() const;
    // End for QML

Q_SIGNALS:
    void comicModelChanged();
    void showComicUrlChanged();
    void showComicAuthorChanged();
    void showComicTitleChanged();
    void showComicIdentifierChanged();
    void showErrorPictureChanged();
    void arrowsOnHoverChanged();
    void middleClickChanged();
    void comicDataChanged();
    void tabHighlightRequest(const QString &id, bool highlight);
    void showNextNewStrip();
    void showActualSizeChanged();
    void tabIdentifiersChanged();
    void checkNewComicStripsIntervalChanged();
    void maxComicLimitChanged();

private Q_SLOTS:
    void slotTabChanged(const QString &newIdentifier);
    void slotNextDay();
    void slotPreviousDay();
    void slotFirstDay();
    void slotCurrentDay();
    void slotFoundLastStrip(int index, const QString &identifier, const QString &suffix);
    void slotGoJump();
    void slotSaveComicAs();
    void slotScaleToContent();
    void slotShop();
    void slotWebsite();
    void slotStorePosition();
    void checkDayChanged();

public Q_SLOTS:
    void configChanged() override;
    void saveConfig();
    Q_INVOKABLE void updateComic(const QString &identifierSuffix = QString());

    Q_INVOKABLE void goJump()
    {
        slotGoJump();
    }

    Q_INVOKABLE void shop()
    {
        slotShop();
    }

    Q_INVOKABLE void tabChanged(const QString &newIdentifier)
    {
        slotTabChanged(newIdentifier);
    }

    Q_INVOKABLE void loadProviders();
    Q_INVOKABLE void positionFullView(QWindow *window);

private:
    void changeComic(bool differentComic);
    void updateUsedComics();
    void updateContextMenu();
    void updateView();
    void refreshComicData();
    void setTabHighlighted(const QString &id, bool highlight);
    bool isTabHighlighted(const QString &id) const;
    void dataUpdated(const ComicMetaData &data);

private:
    ComicModel *mModel;
    QString mPreviousFailedIdentifier;
    QSortFilterProxyModel *mProxy;
    ActiveComicModel *mActiveComicModel;
    QVariantMap mComicData;

    QDate mCurrentDay;

    QString mIdentifierError;
    QString mOldSource;
    ConfigWidget *mConfigWidget;
    bool mDifferentComic;
    bool mShowComicUrl;
    bool mShowComicAuthor;
    bool mShowComicTitle;
    bool mShowComicIdentifier;
    bool mShowErrorPicture;
    bool mArrowsOnHover;
    bool mMiddleClick;
    int mCheckNewComicStripsInterval;
    int mMaxComicLimit;
    CheckNewStrips *mCheckNewStrips;
    QTimer *mDateChangedTimer;
    QList<QAction *> mActions;
    QAction *mActionGoFirst;
    QAction *mActionGoLast;
    QAction *mActionGoJump;
    QAction *mActionScaleContent;
    QAction *mActionWebsite;
    QAction *mActionShop;
    QAction *mActionStorePosition;
    QAction *mActionNextNewStripTab;
    QAction *mActionSaveComicAs;
    QAction *mActionCreateComicBook;
    QSizeF mMaxSize;
    QSizeF mLastSize;
    QSizeF mIdealSize;
    ComicEngine *const mEngine;

    // Tabs
    bool mTabAdded;
    QStringList mTabIdentifier;

    ComicData mCurrent;
    SavingDir *mSavingDir;
};

#endif

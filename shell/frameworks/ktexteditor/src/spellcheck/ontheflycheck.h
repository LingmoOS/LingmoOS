/*
    SPDX-FileCopyrightText: 2008-2010 Michel Ludwig <michel.ludwig@kdemail.net>
    SPDX-FileCopyrightText: 2009 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ONTHEFLYCHECK_H
#define ONTHEFLYCHECK_H

#include <QList>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QString>
#include <map>

#include <sonnet/speller.h>

#include "katedocument.h"

namespace Sonnet
{
class BackgroundChecker;
}

class KateOnTheFlyChecker : public QObject, private KTextEditor::MovingRangeFeedback
{
    enum ModificationType { TEXT_INSERTED = 0, TEXT_REMOVED };

    typedef QPair<KTextEditor::MovingRange *, QString> SpellCheckItem;
    typedef QList<KTextEditor::MovingRange *> MovingRangeList;
    typedef QPair<KTextEditor::MovingRange *, QString> MisspelledItem;
    typedef QList<MisspelledItem> MisspelledList;

    typedef QPair<ModificationType, KTextEditor::MovingRange *> ModificationItem;
    typedef QList<ModificationItem> ModificationList;

public:
    explicit KateOnTheFlyChecker(KTextEditor::DocumentPrivate *document);
    ~KateOnTheFlyChecker() override;

    QPair<KTextEditor::Range, QString> getMisspelledItem(const KTextEditor::Cursor cursor) const;
    QString dictionaryForMisspelledRange(KTextEditor::Range range) const;
    MovingRangeList installedMovingRanges(KTextEditor::Range range) const;

    void clearMisspellingForWord(const QString &word);

public:
    void textInserted(KTextEditor::Document *document, KTextEditor::Range range);
    void textRemoved(KTextEditor::Document *document, KTextEditor::Range range);

    void updateConfig();
    void refreshSpellCheck(KTextEditor::Range range = KTextEditor::Range::invalid());

    void updateInstalledMovingRanges(KTextEditor::ViewPrivate *view);

protected:
    KTextEditor::DocumentPrivate *const m_document;
    Sonnet::Speller m_speller;
    QList<SpellCheckItem> m_spellCheckQueue;
    Sonnet::BackgroundChecker *m_backgroundChecker;
    SpellCheckItem m_currentlyCheckedItem;
    MisspelledList m_misspelledList;
    ModificationList m_modificationList;
    KTextEditor::DocumentPrivate::OffsetList m_currentDecToEncOffsetList;
    std::map<KTextEditor::View *, KTextEditor::Range> m_displayRangeMap;

    void freeDocument();

    void queueLineSpellCheck(KTextEditor::DocumentPrivate *document, int line);
    /**
     * 'range' must be on a single line
     **/
    void queueLineSpellCheck(KTextEditor::Range range, const QString &dictionary);
    void queueSpellCheckVisibleRange(KTextEditor::Range range);
    void queueSpellCheckVisibleRange(KTextEditor::ViewPrivate *view, KTextEditor::Range range);

    void addToSpellCheckQueue(KTextEditor::Range range, const QString &dictionary);
    void addToSpellCheckQueue(KTextEditor::MovingRange *range, const QString &dictionary);

    QTimer *m_viewRefreshTimer;
    QPointer<KTextEditor::ViewPrivate> m_refreshView;

    virtual void removeRangeFromEverything(KTextEditor::MovingRange *range);
    bool removeRangeFromCurrentSpellCheck(KTextEditor::MovingRange *range);
    bool removeRangeFromSpellCheckQueue(KTextEditor::MovingRange *range);
    void rangeEmpty(KTextEditor::MovingRange *range) override;
    void rangeInvalid(KTextEditor::MovingRange *range) override;
    void caretEnteredRange(KTextEditor::MovingRange *range, KTextEditor::View *view) override;
    void caretExitedRange(KTextEditor::MovingRange *range, KTextEditor::View *view) override;

    KTextEditor::Range findWordBoundaries(const KTextEditor::Cursor begin, const KTextEditor::Cursor end);

    void deleteMovingRange(KTextEditor::MovingRange *range);
    void deleteMovingRanges(const QList<KTextEditor::MovingRange *> &list);
    void deleteMovingRangeQuickly(KTextEditor::MovingRange *range);
    void stopCurrentSpellCheck();

protected:
    void performSpellCheck();
    void addToDictionary(const QString &word);
    void addToSession(const QString &word);
    void misspelling(const QString &word, int start);
    void spellCheckDone();

    void viewDestroyed(QObject *obj);
    void addView(KTextEditor::Document *document, KTextEditor::View *view);
    void removeView(KTextEditor::View *view);

    void restartViewRefreshTimer(KTextEditor::ViewPrivate *view);
    void viewRefreshTimeout();

    void handleModifiedRanges();
    void handleInsertedText(KTextEditor::Range range);
    void handleRemovedText(KTextEditor::Range range);
    void handleRespellCheckBlock(int start, int end);
    bool removeRangeFromModificationList(KTextEditor::MovingRange *range);
    void clearModificationList();
};

#endif

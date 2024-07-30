/*
 *   SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef klinespellchecking_h
#define klinespellchecking_h

#include <KLineEdit>

class QAction;

class KLineSpellChecking : public KLineEdit
{
    Q_OBJECT
public:
    explicit KLineSpellChecking(QWidget *parent = nullptr);
    ~KLineSpellChecking() override;

    void highLightWord(int length, int pos);

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

private Q_SLOTS:
    void slotCheckSpelling();
    void slotSpellCheckDone(const QString &s);
    void spellCheckerMisspelling(const QString &text, int pos);
    void spellCheckerCorrected(const QString &, int, const QString &);
    void spellCheckerFinished();

private:
    QAction *m_spellAction;
};

#endif

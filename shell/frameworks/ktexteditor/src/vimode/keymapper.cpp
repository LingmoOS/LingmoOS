/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "globalstate.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "macrorecorder.h"
#include "mappings.h"
#include <vimode/inputmodemanager.h>
#include <vimode/keymapper.h>
#include <vimode/keyparser.h>

#include <QTimer>

using namespace KateVi;

KeyMapper::KeyMapper(InputModeManager *kateViInputModeManager, KTextEditor::DocumentPrivate *doc)
    : m_viInputModeManager(kateViInputModeManager)
    , m_doc(doc)
{
    m_mappingTimer = new QTimer(this);
    m_doNotExpandFurtherMappings = false;
    m_timeoutlen = 1000; // FIXME: make configurable
    m_doNotMapNextKeypress = false;
    m_numMappingsBeingExecuted = 0;
    m_isPlayingBackRejectedKeys = false;
    connect(m_mappingTimer, &QTimer::timeout, this, &KeyMapper::mappingTimerTimeOut);
}

void KeyMapper::executeMapping()
{
    m_mappingKeys.clear();
    m_mappingTimer->stop();
    m_numMappingsBeingExecuted++;
    const QString mappedKeypresses =
        m_viInputModeManager->globalState()->mappings()->get(Mappings::mappingModeForCurrentViMode(m_viInputModeManager->inputAdapter()),
                                                             m_fullMappingMatch,
                                                             false,
                                                             true);
    if (!m_viInputModeManager->globalState()->mappings()->isRecursive(Mappings::mappingModeForCurrentViMode(m_viInputModeManager->inputAdapter()),
                                                                      m_fullMappingMatch)) {
        m_doNotExpandFurtherMappings = true;
    }
    m_doc->editStart();
    m_viInputModeManager->feedKeyPresses(mappedKeypresses);
    m_doNotExpandFurtherMappings = false;
    m_doc->editEnd();
    m_numMappingsBeingExecuted--;
}

void KeyMapper::playBackRejectedKeys()
{
    m_isPlayingBackRejectedKeys = true;
    const QString mappingKeys = m_mappingKeys;
    m_mappingKeys.clear();
    m_viInputModeManager->feedKeyPresses(mappingKeys);
    m_isPlayingBackRejectedKeys = false;
}

void KeyMapper::setMappingTimeout(int timeoutMS)
{
    m_timeoutlen = timeoutMS;
}

void KeyMapper::mappingTimerTimeOut()
{
    if (!m_fullMappingMatch.isNull()) {
        executeMapping();
    } else {
        playBackRejectedKeys();
    }
    m_mappingKeys.clear();
}

bool KeyMapper::handleKeypress(QChar key)
{
    if (!m_doNotExpandFurtherMappings && !m_doNotMapNextKeypress && !m_isPlayingBackRejectedKeys) {
        m_mappingKeys.append(key);

        bool isPartialMapping = false;
        bool isFullMapping = false;
        m_fullMappingMatch.clear();
        const auto mappingMode = Mappings::mappingModeForCurrentViMode(m_viInputModeManager->inputAdapter());
        const auto mappings = m_viInputModeManager->globalState()->mappings()->getAll(mappingMode, false, true);
        for (const QString &mapping : mappings) {
            if (mapping.startsWith(m_mappingKeys)) {
                if (mapping == m_mappingKeys) {
                    isFullMapping = true;
                    m_fullMappingMatch = mapping;
                } else {
                    isPartialMapping = true;
                }
            }
        }
        if (isFullMapping && !isPartialMapping) {
            // Great - m_mappingKeys is a mapping, and one that can't be extended to
            // a longer one - execute it immediately.
            executeMapping();
            return true;
        }
        if (isPartialMapping) {
            // Need to wait for more characters (or a timeout) before we decide what to
            // do with this.
            m_mappingTimer->start(m_timeoutlen);
            m_mappingTimer->setSingleShot(true);
            return true;
        }
        // We've been swallowing all the keypresses meant for m_keys for our mapping keys; now that we know
        // this cannot be a mapping, restore them.
        Q_ASSERT(!isPartialMapping && !isFullMapping);
        const bool isUserKeypress = !m_viInputModeManager->macroRecorder()->isReplaying() && !isExecutingMapping();
        if (isUserKeypress && m_mappingKeys.size() == 1) {
            // Ugh - unpleasant complication starting with Qt 5.5-ish - it is no
            // longer possible to replay QKeyEvents in such a way that shortcuts
            // are triggered, so if we want to correctly handle a shortcut (e.g.
            // ctrl+s for e.g. Save), we can no longer pop it into m_mappingKeys
            // then immediately playBackRejectedKeys() (as this will not trigger
            // the e.g. Save shortcut) - the best we can do is, if e.g. ctrl+s is
            // not part of any mapping, immediately return false, *not* calling
            // playBackRejectedKeys() and clearing m_mappingKeys ourselves.
            // If e.g. ctrl+s *is* part of a mapping, then if the mapping is
            // rejected, the played back e.g. ctrl+s does not trigger the e.g.
            // Save shortcut. Likewise, we can no longer have e.g. ctrl+s inside
            // mappings or macros - the e.g. Save shortcut will not be triggered!
            // Altogether, a pretty disastrous change from Qt's old behaviour -
            // either they "fix" it (although it could be argued that being able
            // to trigger shortcuts from QKeyEvents was never the desired behaviour)
            // or we try to emulate Shortcut-handling ourselves :(
            m_mappingKeys.clear();
            return false;
        } else {
            playBackRejectedKeys();
            return true;
        }
    }
    m_doNotMapNextKeypress = false;
    return false;
}

void KeyMapper::setDoNotMapNextKeypress()
{
    m_doNotMapNextKeypress = true;
}

bool KeyMapper::isExecutingMapping() const
{
    return m_numMappingsBeingExecuted > 0;
}

bool KeyMapper::isPlayingBackRejectedKeys() const
{
    return m_isPlayingBackRejectedKeys;
}

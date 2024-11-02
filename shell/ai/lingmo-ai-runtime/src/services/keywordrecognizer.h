/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SERVICE_KEYWORDRECOGNIZER_H
#define SERVICE_KEYWORDRECOGNIZER_H

#include "services/keywordrecognizerglue.h"
#include "engine/aienginepluginmanager.h"
#include "microphone/microphone.h"
#include "services/keywordrecognizerglue.h"

#include <lingmo-ai/ai-engine/large-model/endsidespeechengine.h>
#include <gio/gio.h>

class KeywordRecognizer
{
public:
    KeywordRecognizer(GDBusConnection &connection,
                      ai_engine::AiEnginePluginManager &aiEnginePluginManager);
    ~KeywordRecognizer();

private:
    void exportSkeleton();
    void unexportSkeleton();

    static bool onInitKeywordRecognizer(AisdkKeywordRecognizer *delegate,
                                        GDBusMethodInvocation *invocation, gpointer userData);
    static bool onDestroyKeywordRecognizer(AisdkKeywordRecognizer *delegate,
                                           GDBusMethodInvocation *invocation, gpointer userData);
    static bool onStartKeywordRecognizer(AisdkKeywordRecognizer *delegate,
                                         GDBusMethodInvocation *invocation, gpointer userData);
    static bool onStopKeywordRecognizer(AisdkKeywordRecognizer *delegate,
                                        GDBusMethodInvocation *invocation, gpointer userData);

    static void onEmitResultSignal(KeywordRecognizer *recognizer, ai_engine::lm::speech::RecognitionResult result);

private:
    bool isExported_ = false;

    GDBusConnection &connection_;
    ai_engine::AiEnginePluginManager &aiEnginePluginManager_;
    AisdkKeywordRecognizer *delegate_ = nullptr;

    std::unique_ptr<ai_engine::lm::speech::AbstractSpeechEngine> speechEngine_ = nullptr;
    ai_engine::lm::speech::AbstractEndSideSpeechEngine *endSideSpeechEngine_ = nullptr;

    Microphone *microphone_ = nullptr;

    const std::string engineName{"ondevice"};
};

#endif
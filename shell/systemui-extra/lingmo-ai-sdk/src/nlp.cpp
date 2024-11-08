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

#include "nlp.h"
#include <string>
#include "nlpsession.h"

#include "settings/globalsettings.h"

static thread_local std::string g_nlp_last_error_message;

NlpResult nlp_create_session(NlpSession *session)
{
    if (!session) {
        return NLP_SESSION_ERROR;
    }
    nlp::NlpSession *nlpSession = new nlp::NlpSession();
    *session = nlpSession;
    return NLP_SUCCESS;
}

void nlp_destroy_session(NlpSession session)
{
    nlp::NlpSession *nlpSession = static_cast<nlp::NlpSession *>(session);
    if (nlpSession) {
        delete nlpSession;
    }
}

NlpResult nlp_init_session(NlpSession session)
{
    nlp::NlpSession *nlpSession = static_cast<nlp::NlpSession *>(session);
    if (!nlpSession) {
        return NLP_SESSION_ERROR;
    }

    const auto& nlpSettings = aisdk::GlobalSettings::getInstance().getChildSettings(CAPABILITY_NLP);

    const std::string model = nlpSettings.getModel(nlpSettings.getDeployPolicy());
    fprintf(stdout, "nlp deploy policy:%i,  model name: %s\n",
            (int)nlpSettings.getDeployPolicy(), model.c_str());

    auto config = nlpSettings.getModelConfig(nlpSettings.getDeployPolicy(), model);
    return nlpSession->init(model, config);
}


void nlp_set_result_callback(NlpSession session, NlpResultCallback callback, void *user_data)
{
    nlp::NlpSession *nlpSession = static_cast<nlp::NlpSession *>(session);
    if (!nlpSession) {
        return;
    }
    nlpSession->setResultCallback(callback, user_data);
}

void nlp_set_session_actor(NlpSession session, int actor_prompt_id)
{
    nlp::NlpSession *nlpSession = static_cast<nlp::NlpSession *>(session);
    if (!nlpSession) {
        return;
    }
    nlpSession->setActor(actor_prompt_id);
}

void nlp_set_context_size(NlpSession session, int size)
{
    nlp::NlpSession *nlpSession = static_cast<nlp::NlpSession *>(session);
    if (!nlpSession) {
        return;
    }

    return nlpSession->setContext(size);
}

void nlp_clear_context(NlpSession session)
{
    nlp::NlpSession *nlpSession = static_cast<nlp::NlpSession *>(session);
    if (!nlpSession) {
        return;
    }

    return nlpSession->clearContext();
}

NlpResult nlp_text_chat(NlpSession session, const char *message)
{
    return NLP_SESSION_ERROR;
}

NlpResult nlp_text_chat_async(NlpSession session, const char * message)
{
    if (!message) {
        fprintf(stderr, "nlp chat error: message is nullptr!\n");
        return NLP_PARAM_ERROR;
    }
    nlp::NlpSession *nlpSession = static_cast<nlp::NlpSession *>(session);
    if (!nlpSession) {
        return NLP_SESSION_ERROR;
    }

    return nlpSession->chat(message);
}

NlpResult nlp_stop_chat(NlpSession session)
{
    nlp::NlpSession *nlpSession = static_cast<nlp::NlpSession *>(session);
    if (!nlpSession) {
        return NLP_SESSION_ERROR;
    }

    nlpSession->stopChat();

    return NLP_SUCCESS;
}

const char *nlp_last_error_message()
{
    return g_nlp_last_error_message.c_str();
}

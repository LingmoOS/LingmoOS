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
#include "modelconfig.h"
#include "baiduremotemodel.h"
#include "erniebot4session.h"

static thread_local std::string g_nlp_last_error_message;

NlpResult nlp_create_session(NlpSession* session)
{
    if (get_default_remote_model_vendor() == BAIDU) {
        ERNIEBot4Session* ernieBot4Session = new ERNIEBot4Session();
        auto errorCode = ernieBot4Session->getResultCode();
        if (errorCode != NLP_SUCCESS) {
            g_nlp_last_error_message = ernieBot4Session->lastErrorMessage();
            delete ernieBot4Session;
            return (NlpResult)errorCode;
        }
        *session = ernieBot4Session;
    }

    return NLP_SUCCESS;
}

void nlp_destory_session(NlpSession session)
{
    if (get_default_remote_model_vendor() == BAIDU) {
        ERNIEBot4Session* baiduSession = (ERNIEBot4Session*)(session);
        if (baiduSession) {
            delete baiduSession;
        }
    }
}

void nlp_set_result_callback(NlpSession session, NlpCallback callback, void* user_data)
{
    if(!session) {
        return;
    }
    ((NLPSession_*)session)->setJsonCallback(callback);
    ((NLPSession_*)session)->setUserData(user_data);
}

void nlp_set_context_size(NlpSession session, int size)
{
    if(!session) {
        return;
    }
    ((NLPSession_*)session)->setContextSize(size);
}

void nlp_clear_context(NlpSession session)
{
    if(!session) {
        return;
    }
    ((NLPSession_*)session)->getContext().clear();
}

NlpResult nlp_text_chat(NlpSession session, const char* message)
{
    NLPSession_* nlpSession = (NLPSession_*)session;

    if(!nlpSession) {
        return NLP_SESSION_ERROR;
    }
    if(!nlpSession->getJsonCallback()) {
        return NLP_SESSION_ERROR;
    }

    auto ret = nlpSession->chat(message);
    if (ret != NLP_SUCCESS) {
        g_nlp_last_error_message = nlpSession->lastErrorMessage();
    }

    return ret;
}

const char *nlp_last_error_message()
{
    return g_nlp_last_error_message.c_str();
}

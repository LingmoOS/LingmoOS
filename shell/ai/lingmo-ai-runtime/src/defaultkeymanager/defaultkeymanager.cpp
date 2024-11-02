#include "defaultkeymanager.h"
#include "engine/library.h"
#include <cstdio>
#include <utility>

DefaultKeyManager &DefaultKeyManager::instance() {
    static DefaultKeyManager instance;
    return instance;
}

DefaultKeyManager::DefaultKeyManager() {
    loadFunctions();
}


std::string DefaultKeyManager::defaultNlpKey() {
    if (nlpKeyGetter_) {
        return nlpKeyGetter_();
    }

    std::fprintf(stderr, "Get nlpKey failed.\n");
    return {};
}
std::string DefaultKeyManager::defaultVisionKey() {
    if (visionKeyGetter_) {
        return visionKeyGetter_();
    }

    std::fprintf(stderr, "Get visionKey failed.\n");
    return {};
}

std::string DefaultKeyManager::defaultSpeechKey() {
    if (speechKeyGetter_) {
        return speechKeyGetter_();
    }
    std::fprintf(stderr, "Get speechKey failed.\n");
    return {};
}

void DefaultKeyManager::updateNlpTokens(const std::string &message) {
    if (tokenUpdater_) {
        tokenUpdater_(message.c_str());
    }
    std::fprintf(stderr, "updateNlpTokens failed.\n");
}

bool DefaultKeyManager::isNlpLimited() {
    if (nlpLimiter_) {
        return nlpLimiter_();
    }
    return true;
}

void DefaultKeyManager::updateText2ImageCount(int imageNumber) {
    if (text2ImageCountUpdater_) {
        text2ImageCountUpdater_(imageNumber);
        return;
    }
    std::fprintf(stderr, "updateText2ImageCount failed.\n");
}

bool DefaultKeyManager::isText2ImageLimit() {
    if (text2ImageLimiter_) {
        return text2ImageLimiter_();
    }
    return true;
}

void DefaultKeyManager::updateRealTimeSpeechRecongnitionTime(int time) {
    if (realTimeSpeechRecongnitionTimeCountUpdater_) {
        realTimeSpeechRecongnitionTimeCountUpdater_(time);
        return;
    }
    std::fprintf(stderr, "updateRealTimeSpeechRecongnitionTime failed.\n");
}

bool DefaultKeyManager::isRealTimeSpeechRecongnitionLimit() {
    if (realTimeSpeechRecognitionLimiter_) {
        return realTimeSpeechRecognitionLimiter_();
    }
    return true;
}

void DefaultKeyManager::updateOnceSpeechRecognitionCount(int count) {
    if (onceSpeechRecognitionCountUpdater_) {
        onceSpeechRecognitionCountUpdater_(count);
        return;
    }
    std::fprintf(stderr, "updateOnceSpeechRecognitionCount failed.\n");
}

bool DefaultKeyManager::isOnceSpeechRecognitionLimit() {
    if (onceSpeechRecognitionLimiter_) {
        return onceSpeechRecognitionLimiter_();
    }
    return true;
}

void DefaultKeyManager::updateSynthesizingCount(int number) {
    if (synthesizingCountUpdater_) {
        synthesizingCountUpdater_(number);
        return;
    }
    std::fprintf(stderr, "updateSynthesizingCount failed.\n");
}

bool DefaultKeyManager::isSynthesizingLimit() {
    if (synthesizingLimiter_) {
        return synthesizingLimiter_();
    }
    return false;
}

void DefaultKeyManager::loadFunctions() {
    ai_engine::Library lib("liblingmo-ai-default-key.so");
    if (!lib.load()) {
        std::fprintf(stderr, "load liblingmo-ai-default-key.so failed: %s\n", lib.error().c_str());
        return;
    }

    auto *getNlpKeyFunc = lib.resolve("getNlpKey");
    if (!getNlpKeyFunc) {
        std::fprintf(stderr, "resolve getNlpKey failed: %s\n", lib.error().c_str());
        return;
    }
    nlpKeyGetter_ = std::move(*(ai_engine::Library::toFunction<const char*()>(getNlpKeyFunc)));

    auto *nlpLimitFunc = lib.resolve("isNlpTokensLimit");
    if (!nlpLimitFunc) {
        std::fprintf(stderr, "resolve isNlpTokensLimit failed: %s\n", lib.error().c_str());
        return;
    }
    nlpLimiter_ = std::move(*(ai_engine::Library::toFunction<bool()>(nlpLimitFunc)));

    auto *getSpeechKeyFunc = lib.resolve("getSpeechKey");
    if (!getSpeechKeyFunc) {
        std::fprintf(stderr, "resolve getSpeechKey failed: %s\n", lib.error().c_str());
        return;
    }
    speechKeyGetter_ = *(ai_engine::Library::toFunction<const char*()>(getSpeechKeyFunc));

    auto *getVisionKeyFunc = lib.resolve("getVisionKey");
    if (!getVisionKeyFunc) {
        std::fprintf(stderr, "resolve getVisionKey failed: %s\n", lib.error().c_str());
        return;
    }
    visionKeyGetter_ = *(ai_engine::Library::toFunction<const char*()>(getVisionKeyFunc));

    auto *text2ImageLimitFunc = lib.resolve("isText2ImageLimit");
    if (!text2ImageLimitFunc) {
        std::fprintf(stderr, "resolve isText2ImageLimit failed: %s\n", lib.error().c_str());
        return;
    }
    text2ImageLimiter_ = std::move(*(ai_engine::Library::toFunction<bool()>(text2ImageLimitFunc)));

    auto *updateTokensFunc = lib.resolve("updateNlpTokens");
    if (!updateTokensFunc) {
        std::fprintf(stderr, "resolve updateNlpTokens failed: %s\n", lib.error().c_str());
        return;
    }
    tokenUpdater_ = *(ai_engine::Library::toFunction<void(const char*)>(updateTokensFunc));

    auto *text2ImageCountUpdateFunc = lib.resolve("updateText2ImageCount");
    if (!text2ImageCountUpdateFunc) {
        std::fprintf(stderr, "resolve text2ImageCountUpdateFunc failed: %s\n", lib.error().c_str());
        return;
    }
    text2ImageCountUpdater_ = *(ai_engine::Library::toFunction<void(int)>(text2ImageCountUpdateFunc));

    auto *realTimeSpeechRecongnitionTimeCountFunc = lib.resolve("updateRealTimeSpeechRecongnitionTime");
    if (!realTimeSpeechRecongnitionTimeCountFunc) {
        std::fprintf(stderr, "resolve updateRealTimeSpeechRecongnitionTime failed: %s\n", lib.error().c_str());
        return;
    }
    realTimeSpeechRecongnitionTimeCountUpdater_ =
        *(ai_engine::Library::toFunction<void(int)>(realTimeSpeechRecongnitionTimeCountFunc));

    auto *isRealTimeSpeechRecongnitionLimitFunc = lib.resolve("isRealTimeSpeechRecongnitionLimit");
    if (!isRealTimeSpeechRecongnitionLimitFunc) {
        std::fprintf(stderr, "resolve isRealTimeSpeechRecongnitionLimit failed: %s\n", lib.error().c_str());
        return;
    }
    realTimeSpeechRecognitionLimiter_ = std::move(
        *(ai_engine::Library::toFunction<bool()>(isRealTimeSpeechRecongnitionLimitFunc)));

    auto *onceSpeechRecognitionCountFunc = lib.resolve("updateOnceSpeechRecognitionCount");
    if (!onceSpeechRecognitionCountFunc) {
        std::fprintf(stderr, "resolve updateOnceSpeechRecognitionCount failed: %s\n", lib.error().c_str());
        return;
    }
    onceSpeechRecognitionCountUpdater_ =
        *(ai_engine::Library::toFunction<void(int)>(onceSpeechRecognitionCountFunc));

    auto *isOnceSpeechRecognitionLimiFunc = lib.resolve("isOnceSpeechRecognitionLimit");
    if (!isOnceSpeechRecognitionLimiFunc) {
        std::fprintf(stderr, "resolve isOnceSpeechRecognitionLimit failed: %s\n", lib.error().c_str());
        return;
    }
    onceSpeechRecognitionLimiter_ = std::move(
        *(ai_engine::Library::toFunction<bool()>(isOnceSpeechRecognitionLimiFunc)));

    auto *synthesizingCountFunc = lib.resolve("updateSynthesizingCount");
    if (!synthesizingCountFunc) {
        std::fprintf(stderr, "resolve updateSynthesizingCount failed: %s\n", lib.error().c_str());
        return;
    }
    synthesizingCountUpdater_ =
        *(ai_engine::Library::toFunction<void(int)>(synthesizingCountFunc));

    auto *isSynthesizingLimitLimiFunc = lib.resolve("isSynthesizingLimit");
    if (!isSynthesizingLimitLimiFunc) {
        std::fprintf(stderr, "resolve isSynthesizingLimit failed: %s\n", lib.error().c_str());
        return;
    }
    synthesizingLimiter_ = std::move(
        *(ai_engine::Library::toFunction<bool()>(isSynthesizingLimitLimiFunc)));
}

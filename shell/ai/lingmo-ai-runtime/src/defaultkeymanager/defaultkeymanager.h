#ifndef DEFAULTKEYMANAGER_H
#define DEFAULTKEYMANAGER_H

#include <functional>
#include <string>
class DefaultKeyManager
{
public:
    static DefaultKeyManager &instance();
    DefaultKeyManager(const DefaultKeyManager &) = delete;
    DefaultKeyManager &operator=(const DefaultKeyManager &) = delete;

    std::string defaultNlpKey();
    std::string defaultSpeechKey();
    std::string defaultVisionKey();

    void updateNlpTokens(const std::string &message);
    bool isNlpLimited();

    void updateText2ImageCount(int imageNumber);
    bool isText2ImageLimit();

    void updateRealTimeSpeechRecongnitionTime(int time);
    bool isRealTimeSpeechRecongnitionLimit();

    void updateOnceSpeechRecognitionCount(int count);
    bool isOnceSpeechRecognitionLimit();

    void updateSynthesizingCount(int number);
    bool isSynthesizingLimit();

private:
    DefaultKeyManager();
    void loadFunctions();

private:
    using KeyGetter = std::function<const char*()>;
    using TokenUpdater = std::function<void(const char*)>;
    using CountUpdater = std::function<void(int)>;
    using AiCapabilityLimiter = std::function<bool()>;

    KeyGetter nlpKeyGetter_;
    KeyGetter speechKeyGetter_;
    KeyGetter visionKeyGetter_;
    TokenUpdater tokenUpdater_;
    CountUpdater text2ImageCountUpdater_;
    CountUpdater realTimeSpeechRecongnitionTimeCountUpdater_;
    CountUpdater onceSpeechRecognitionCountUpdater_;
    CountUpdater synthesizingCountUpdater_;
    AiCapabilityLimiter nlpLimiter_;
    AiCapabilityLimiter speechLimiter_;
    AiCapabilityLimiter text2ImageLimiter_;
    AiCapabilityLimiter onceSpeechRecognitionLimiter_;
    AiCapabilityLimiter realTimeSpeechRecognitionLimiter_;
    AiCapabilityLimiter synthesizingLimiter_;
};

#endif // DEFAULTKEYMANAGER_H

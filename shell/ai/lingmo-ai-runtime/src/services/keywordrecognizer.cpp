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

#include "services/keywordrecognizer.h"

#include <iostream>
#include <mutex>
#include <thread>

#include "services/threadpool/async.h"

static const char *objectPath_ =
	"/org/lingmo/aisdk/keywordrecognizer";

static std::mutex samplesMutex_;
static std::vector<char> samples;
static bool start = false;

static void transformToPcmform(std::vector<char> &audioData, int32_t sampling_rate,
							   const float *samples, int32_t n)
{
	std::vector<int16_t> samples_int16(n);
	for (int32_t i = 0; i != n; ++i)
	{
		samples_int16[i] = samples[i] * 32676;
	}

	audioData.resize(samples_int16.size() * sizeof(int16_t));
	memcpy(audioData.data(),
		   reinterpret_cast<const char *>(samples_int16.data()),
		   samples_int16.size() * sizeof(int16_t));
}

static int callback(const void *input, void *output,
					unsigned long frameCount,
					const PaStreamCallbackTimeInfo *timeInfo,
					PaStreamCallbackFlags statusFlags,
					void *userData)
{
	std::lock_guard<std::mutex> lock(samplesMutex_);

	const float *data = reinterpret_cast<const float *>(input);
	static std::vector<char> test;
	transformToPcmform(test, 16000, data, frameCount);
	samples.insert(samples.end(), test.begin(), test.end());
	return start ? 0 : 1;
}

KeywordRecognizer::KeywordRecognizer(GDBusConnection &connection,
									 ai_engine::AiEnginePluginManager &aiEnginePluginManager) : connection_(connection),
																								aiEnginePluginManager_(aiEnginePluginManager)
{
	exportSkeleton();
}

KeywordRecognizer::~KeywordRecognizer()
{
	unexportSkeleton();
}

void KeywordRecognizer::exportSkeleton()
{
	delegate_ = aisdk_keyword_recognizer_skeleton_new();

	g_signal_connect(delegate_, "handle-init-keyword-recognizer",
					 G_CALLBACK(onInitKeywordRecognizer), this);

	g_signal_connect(delegate_, "handle-destroy-keyword-recognizer",
					 G_CALLBACK(onDestroyKeywordRecognizer), this);

	g_signal_connect(delegate_, "handle-start-keyword-recognizer",
					 G_CALLBACK(onStartKeywordRecognizer), this);

	g_signal_connect(delegate_, "handle-stop-keyword-recognizer",
					 G_CALLBACK(onStopKeywordRecognizer), this);

	GError *error = nullptr;

	isExported_ = g_dbus_interface_skeleton_export(
		G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath_,
		&error);
	if (!isExported_)
	{
		g_printerr("Error exporting skeleton at address %s: %s\n",
				   objectPath_, error->message);
		g_error_free(error);
	}
}

void KeywordRecognizer::unexportSkeleton()
{
	if (delegate_ == nullptr)
	{
		return;
	}

	if (isExported_)
	{
		g_dbus_interface_skeleton_unexport_from_connection(
			G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
	}

	g_object_unref(delegate_);
}
bool KeywordRecognizer::onInitKeywordRecognizer(AisdkKeywordRecognizer *delegate,
												GDBusMethodInvocation *invocation, gpointer userData)
{
	std::cout << "init KeywordRecognizer." << std::endl;
	KeywordRecognizer *recognizer = reinterpret_cast<KeywordRecognizer *>(userData);

	recognizer->microphone_ = new Microphone(callback);

	recognizer->speechEngine_ = recognizer->aiEnginePluginManager_.createSpeechEngine(recognizer->engineName);
	if (recognizer->speechEngine_ == nullptr)
	{
		std::cout << "speech engine create error" << std::endl;
		aisdk_keyword_recognizer_complete_init_keyword_recognizer(delegate,invocation);
		return true;
	}

	recognizer->endSideSpeechEngine_ = dynamic_cast<ai_engine::lm::speech::AbstractEndSideSpeechEngine *>(recognizer->speechEngine_.get());
	if (recognizer->endSideSpeechEngine_ == nullptr)
	{
		std::cout << "KeywordRecognizer endSideSpeechEngine_ is null"
				  << "func : " << __FUNCTION__ << "line : " << __LINE__ << std::endl;
		aisdk_keyword_recognizer_complete_init_keyword_recognizer(delegate,invocation);
		return true;
	}

	recognizer->endSideSpeechEngine_->initContinuousKeywordRecognitionModule();
	recognizer->endSideSpeechEngine_->setRecognizingCallback([recognizer](ai_engine::lm::speech::RecognitionResult result)
															 {
		if (recognizer == nullptr)
		{
			return;
		}

		onEmitResultSignal(recognizer, result); });
	recognizer->endSideSpeechEngine_->setRecognizedCallback([&recognizer](ai_engine::lm::speech::RecognitionResult result)
															{ std::cout << "keyword recognition result:" << std::string(result.text) << std::endl; });

	aisdk_keyword_recognizer_complete_init_keyword_recognizer(delegate,invocation);
	return true;
}

bool KeywordRecognizer::onDestroyKeywordRecognizer(AisdkKeywordRecognizer *delegate,
												   GDBusMethodInvocation *invocation, gpointer userData)
{
	std::cout << "destroy KeywordRecognizer." << std::endl;

	KeywordRecognizer *recognizer = reinterpret_cast<KeywordRecognizer *>(userData);
	if (recognizer == nullptr)
	{
		aisdk_keyword_recognizer_complete_destroy_keyword_recognizer(delegate,invocation);
		return true;
	}
	
	if (recognizer->microphone_ != nullptr)
	{
		delete recognizer->microphone_;
		recognizer->microphone_ = nullptr;
	}

	if (recognizer->endSideSpeechEngine_ != nullptr)
	{
		recognizer->endSideSpeechEngine_->destroyContinuousKeywordRecognitionModule();
		recognizer->endSideSpeechEngine_ = nullptr;
	}

	aisdk_keyword_recognizer_complete_destroy_keyword_recognizer(delegate,invocation);
	return true;
}

bool KeywordRecognizer::onStartKeywordRecognizer(AisdkKeywordRecognizer *delegate,
												 GDBusMethodInvocation *invocation, gpointer userData)
{
	std::cout << "start KeywordRecognizer." << std::endl;
	KeywordRecognizer *recognizer = reinterpret_cast<KeywordRecognizer *>(userData);
	if (recognizer == nullptr)
	{
		aisdk_keyword_recognizer_complete_start_keyword_recognizer(delegate,invocation);
		return true;
	}
	if (recognizer->endSideSpeechEngine_ == nullptr)
	{
		std::cout << "KeywordRecognizer endSideSpeechEngine_ is null"
				  << "func : " << __FUNCTION__ << "line : " << __LINE__ << std::endl;
		aisdk_keyword_recognizer_complete_start_keyword_recognizer(delegate,invocation);
		return true;
	}

	samples.clear();
	start = true;
	recognizer->microphone_->startMicrophone();

	recognizer->endSideSpeechEngine_->startContinuousKeywordRecognition("\{\"rate\": 16000}");

	std::thread t([recognizer]()
				  {
		while (start)
		{
			std::vector<char> buf;
			{
				std::lock_guard<std::mutex> lock(samplesMutex_);
				buf = std::move(samples);
				recognizer->endSideSpeechEngine_->writeContinuousKeywordRecognitionAudioData(buf);
			}
			// 需要每隔1s处理一次音频数据
			sleep(1);
		} });
	t.detach();

	aisdk_keyword_recognizer_complete_start_keyword_recognizer(delegate,invocation);
	return true;
}

bool KeywordRecognizer::onStopKeywordRecognizer(AisdkKeywordRecognizer *delegate,
												GDBusMethodInvocation *invocation, gpointer userData)
{
	std::cout << "stop KeywordRecognizer." << std::endl;

	samples.clear();
	start = false;

	KeywordRecognizer *recognizer = reinterpret_cast<KeywordRecognizer *>(userData);
	if (recognizer == nullptr)
	{
		aisdk_keyword_recognizer_complete_stop_keyword_recognizer(delegate,invocation);
		return true;
	}
	if (recognizer->endSideSpeechEngine_ == nullptr)
	{
		std::cout << "KeywordRecognizer endSideSpeechEngine_ is null"
				  << "func : " << __FUNCTION__ << "line : " << __LINE__ << std::endl;
		aisdk_keyword_recognizer_complete_stop_keyword_recognizer(delegate,invocation);
		return true;
	}

	recognizer->microphone_->stopMicrophone();
	recognizer->endSideSpeechEngine_->stopContinuousKeywordRecognition();

	aisdk_keyword_recognizer_complete_stop_keyword_recognizer(delegate,invocation);
	return true;
}

void KeywordRecognizer::onEmitResultSignal(KeywordRecognizer *recognizer, ai_engine::lm::speech::RecognitionResult result)
{
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
	g_variant_builder_add(&builder, "s", result.text.c_str());
	g_variant_builder_add(&builder, "i", result.text.length());
	g_variant_builder_add(&builder, "i", result.errorCode);

	g_dbus_connection_emit_signal(&recognizer->connection_,
								  "org.lingmo.aisdk.keywordrecognizer",
								  "/org/lingmo/aisdk/keywordrecognizer",
								  "org.lingmo.aisdk.keywordrecognizer",
								  "KeywordRecognizerResult",
								  g_variant_builder_end(&builder), nullptr);
}
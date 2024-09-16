/**************************************************************************************************
---------------------------------------------------------------------------------------------------
	Copyright (C) 2015-2021  Jonathan Bagg
	This file is part of QtZeroConf.

	QtZeroConf is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	QtZeroConf is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with QtZeroConf.  If not, see <http://www.gnu.org/licenses/>.
---------------------------------------------------------------------------------------------------
   Project name : QtZeroConf
   File name    : androidnsd.cpp
   Created      : 10 Spetember 2021
   Author(s)    : Michael Zanetti
---------------------------------------------------------------------------------------------------
   NsdManager wrapper for use on Android devices
---------------------------------------------------------------------------------------------------
**************************************************************************************************/
#include <QGuiApplication>
#include <QRegularExpression>
#include "androidnsd_p.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using QAndroidJniEnvironment = QJniEnvironment;
#endif

Q_DECLARE_METATYPE(QHostAddress)

static QMutex s_instancesMutex;
static QList<QZeroConfPrivate*> s_instances;


QZeroConfPrivate::QZeroConfPrivate(QZeroConf *parent)
{
	qRegisterMetaType<QHostAddress>();
	qRegisterMetaType<TxtRecordMap>("TxtRecordMap");

	pub = parent;

	QAndroidJniEnvironment env;

	JNINativeMethod methods[] {
		{ "onServiceResolvedJNI", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/util/Map;)V", (void*)QZeroConfPrivate::onServiceResolvedJNI },
		{ "onServiceRemovedJNI", "(JLjava/lang/String;)V", (void*)QZeroConfPrivate::onServiceRemovedJNI },
		{ "onBrowserStateChangedJNI", "(JZZ)V", (void*)QZeroConfPrivate::onBrowserStateChangedJNI },
		{ "onPublisherStateChangedJNI", "(JZZ)V", (void*)QZeroConfPrivate::onPublisherStateChangedJNI },
		{ "onServiceNameChangedJNI", "(JLjava/lang/String;)V", (void*)QZeroConfPrivate::onServiceNameChangedJNI }
	};

	// There seems to be no straight forward way to match the "thiz" pointer from JNI calls to our pointer of the Java class
	// Passing "this" as ID down to Java so we can access "this" in callbacks.
	// Note: needs to be quint64 as uintptr_t might be 32 or 64 bit depending on the system, while Java expects a jlong which is always 64 bit.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	nsdManager = QAndroidJniObject("qtzeroconf/QZeroConfNsdManager", "(JLandroid/content/Context;)V", reinterpret_cast<quint64>(this), QtAndroid::androidActivity().object());
#else
	nsdManager = QAndroidJniObject("qtzeroconf/QZeroConfNsdManager", "(JLandroid/content/Context;)V", reinterpret_cast<quint64>(this), QNativeInterface::QAndroidApplication::context());
#endif
	if (nsdManager.isValid()) {
		jclass objectClass = env->GetObjectClass(nsdManager.object<jobject>());
		env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
		env->DeleteLocalRef(objectClass);
	}

	QMutexLocker locker(&s_instancesMutex);
	s_instances.append(this);
}

QZeroConfPrivate::~QZeroConfPrivate()
{
	QMutexLocker locker(&s_instancesMutex);
	s_instances.removeAll(this);
}

// In order to not having to pay attention to only use thread safe methods on the java side, we're only running
// Java calls on the Android thread.
// To make sure the Java object is not going out of scope and being garbage collected when the QZeroConf object
// is deleted before the worker thread actually starts, keep a new QAndroidJniObject to nsdManager
// which will increase the ref counter in the JVM.
void QZeroConfPrivate::startServicePublish(const char *name, const char *type, quint16 port)
{
	QAndroidJniObject ref(nsdManager);
	publishName = name;
	publishType = type;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QtAndroid::runOnAndroidThread([=](){
#else
	QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
#endif
		QAndroidJniObject txtMap("java/util/HashMap");
		foreach (const QByteArray &key, txtRecords.keys()) {
			txtMap.callObjectMethod("put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
									QAndroidJniObject::fromString(key).object<jstring>(),
									QAndroidJniObject::fromString(txtRecords.value(key)).object<jstring>());
		}

		ref.callMethod<void>("registerService", "(Ljava/lang/String;Ljava/lang/String;ILjava/util/Map;)V",
							 QAndroidJniObject::fromString(publishName).object<jstring>(),
							 QAndroidJniObject::fromString(publishType).object<jstring>(),
							 port,
							 txtMap.object());
	});
}

void QZeroConfPrivate::stopServicePublish()
{
	QAndroidJniObject ref(nsdManager);
	// If Android is on it's way to suspend when stopServicePublish() is called, we need to call nsd.unregisterService() synchronously
	// to force it to run before the device goes to sleep.  If instead it is scheduled to run in the Android thread, it will not run
	// until the device is woken back up.
	if (qGuiApp->applicationState() == Qt::ApplicationSuspended) {
		ref.callMethod<void>("unregisterService");
	} else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QtAndroid::runOnAndroidThread([ref]() {
#else
		QNativeInterface::QAndroidApplication::runOnAndroidMainThread([ref]() {
#endif
			ref.callMethod<void>("unregisterService");
		});
	}
}

void QZeroConfPrivate::startBrowser(QString type, QAbstractSocket::NetworkLayerProtocol protocol)
{
	Q_UNUSED(protocol)
	QAndroidJniObject ref(nsdManager);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QtAndroid::runOnAndroidThread([ref, type]() {
#else
	QNativeInterface::QAndroidApplication::runOnAndroidMainThread([ref, type]() {
#endif
		ref.callMethod<void>("discoverServices", "(Ljava/lang/String;)V", QAndroidJniObject::fromString(type).object<jstring>());
	});
}

void QZeroConfPrivate::stopBrowser()
{
	QAndroidJniObject ref(nsdManager);
	// If Android is on it's way to suspend when stopBrowser() is called, we need to call nsd.stopServiceDiscovery() synchronously
	// to force it to run before the device goes to sleep.
	if (qGuiApp->applicationState() == Qt::ApplicationSuspended) {
		ref.callMethod<void>("stopServiceDiscovery");
	} else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QtAndroid::runOnAndroidThread([ref]() {
#else
		QNativeInterface::QAndroidApplication::runOnAndroidMainThread([ref]() {
#endif
			ref.callMethod<void>("stopServiceDiscovery");
		});
	}
}

// Callbacks will come in from the android thread. So we're never accessing any of our members directly but instead
// propagate callbacks through Qt::QueuedConnection invokes into the Qt thread. Be sure to check if the instance is still
// alive by checking s_instances while holding the mutex before scheduling the invokation.
void QZeroConfPrivate::onServiceResolvedJNI(JNIEnv */*env*/, jobject /*thiz*/, jlong id, jstring name, jstring type, jstring hostname, jstring address, jint port, jobject txtRecords)
{
	QMap<QByteArray, QByteArray> txtMap;
	QAndroidJniObject txt(txtRecords);
	QAndroidJniObject txtKeys = txt.callObjectMethod("keySet", "()Ljava/util/Set;").callObjectMethod("toArray", "()[Ljava/lang/Object;");

	QAndroidJniEnvironment env;
	for (int i = 0; i < txt.callMethod<jint>("size"); i++) {
		QAndroidJniObject key = QAndroidJniObject(env->GetObjectArrayElement(txtKeys.object<jobjectArray>(), i));
		QAndroidJniObject valueObj = txt.callObjectMethod("get", "(Ljava/lang/Object;)Ljava/lang/Object;", key.object<jstring>());
		if (valueObj.isValid()) {
			jboolean isCopy;
			jbyte* b = env->GetByteArrayElements(valueObj.object<jbyteArray>(), &isCopy);
			QByteArray value((char *)b, env->GetArrayLength(valueObj.object<jbyteArray>()));
			env->ReleaseByteArrayElements(valueObj.object<jbyteArray>(), b, JNI_ABORT);
			txtMap.insert(key.toString().toUtf8(), value);
		} else {
			txtMap.insert(key.toString().toUtf8(), QByteArray());
		}
	}

	QZeroConfPrivate *ref = reinterpret_cast<QZeroConfPrivate*>(id);
	QMutexLocker locker(&s_instancesMutex);
	if (!s_instances.contains(ref)) {
		return;
	}
	QMetaObject::invokeMethod(ref, "onServiceResolved", Qt::QueuedConnection,
							  Q_ARG(QString, QAndroidJniObject(name).toString()),
							  Q_ARG(QString, QAndroidJniObject(type).toString()),
							  Q_ARG(QString, QAndroidJniObject(hostname).toString()),
							  Q_ARG(QHostAddress, QHostAddress(QAndroidJniObject(address).toString())),
							  Q_ARG(int, port),
							  Q_ARG(TxtRecordMap, txtMap)
							  );

}

void QZeroConfPrivate::onServiceRemovedJNI(JNIEnv */*env*/, jobject /*this*/, jlong id, jstring name)
{
	QZeroConfPrivate *ref = reinterpret_cast<QZeroConfPrivate*>(id);
	QMutexLocker locker(&s_instancesMutex);
	if (!s_instances.contains(ref)) {
		return;
	}
	QMetaObject::invokeMethod(ref, "onServiceRemoved", Qt::QueuedConnection, Q_ARG(QString, QAndroidJniObject(name).toString()));
}


void QZeroConfPrivate::onBrowserStateChangedJNI(JNIEnv */*env*/, jobject /*thiz*/, jlong id, jboolean running, jboolean error)
{
	QZeroConfPrivate *ref = reinterpret_cast<QZeroConfPrivate*>(id);
	QMutexLocker locker(&s_instancesMutex);
	if (!s_instances.contains(ref)) {
		return;
	}
	QMetaObject::invokeMethod(ref, "onBrowserStateChanged", Qt::QueuedConnection, Q_ARG(bool, running), Q_ARG(bool, error));
}

void QZeroConfPrivate::onPublisherStateChangedJNI(JNIEnv */*env*/, jobject /*this*/, jlong id, jboolean running, jboolean error)
{
	QZeroConfPrivate *ref = reinterpret_cast<QZeroConfPrivate*>(id);
	QMutexLocker locker(&s_instancesMutex);
	if (!s_instances.contains(ref)) {
		return;
	}
	QMetaObject::invokeMethod(ref, "onPublisherStateChanged", Qt::QueuedConnection, Q_ARG(bool, running), Q_ARG(bool, error));
}

void QZeroConfPrivate::onServiceNameChangedJNI(JNIEnv */*env*/, jobject /*thiz*/, jlong id, jstring newName)
{
	QZeroConfPrivate *ref = reinterpret_cast<QZeroConfPrivate*>(id);
	QMutexLocker locker(&s_instancesMutex);
	if (!s_instances.contains(ref)) {
		return;
	}
	QMetaObject::invokeMethod(ref, "onServiceNameChanged", Qt::QueuedConnection, Q_ARG(QString, QAndroidJniObject(newName).toString()));
}

void QZeroConfPrivate::onServiceResolved(const QString &name, const QString &type, const QString &hostname, const QHostAddress &address, int port, const TxtRecordMap &txtRecords)
{
	QZeroConfService zcs;
	bool newRecord = false;
	if (pub->services.contains(name)) {
		zcs = pub->services.value(name);
	} else {
		zcs = QZeroConfService(new QZeroConfServiceData);
		newRecord = true;
	}

	zcs->m_name = name;
	zcs->m_type = type;
	// A previous implementation (based on avahi) returned service type as "_http._tcp" but Android API return "._http._tcp"
	// Stripping leading dot for backwards compatibility. FIXME: Still not in line with bonjour, which adds a trailing dot.
	zcs->m_type.remove(QRegularExpression("^."));
	zcs->m_host = hostname;
	zcs->m_port = port;
	zcs->m_ip = address;
	zcs->m_txt = txtRecords;

	// Those are not available on Androids NsdManager
	//        zcs->m_domain = domain;
	//        zcs->m_interfaceIndex = interface;

	if (newRecord) {
		pub->services.insert(name, zcs);
		emit pub->serviceAdded(zcs);
	} else {
		emit pub->serviceUpdated(zcs);
	}
}

void QZeroConfPrivate::onServiceRemoved(const QString &name)
{
	if (pub->services.contains(name)) {
		QZeroConfService service = pub->services.take(name);
		emit pub->serviceRemoved(service);
	}
}

void QZeroConfPrivate::onBrowserStateChanged(bool running, bool error)
{
	browserExists = running;
	if (error) {
		emit pub->error(QZeroConf::browserFailed);
	}
}

void QZeroConfPrivate::onPublisherStateChanged(bool running, bool error)
{
	publisherExists = running;
	if (running) {
		emit pub->servicePublished();
	}
	if (error) {
		emit pub->error(QZeroConf::serviceRegistrationFailed);
	}
}

void QZeroConfPrivate::onServiceNameChanged(const QString &newName)
{
	emit pub->serviceNameChanged(newName);
}


QZeroConf::QZeroConf(QObject *parent) : QObject(parent)
{
	pri = new QZeroConfPrivate(this);
	qRegisterMetaType<QZeroConfService>("QZeroConfService");
}

QZeroConf::~QZeroConf()
{
	delete pri;
}

void QZeroConf::startServicePublish(const char *name, const char *type, const char *domain, quint16 port, quint32 interface)
{
	Q_UNUSED(domain) // Not supported on Android API
	Q_UNUSED(interface) // Not supported on Android API
	pri->startServicePublish(name, type, port);
}

void QZeroConf::stopServicePublish(void)
{
	pri->stopServicePublish();
}

bool QZeroConf::publishExists(void)
{
	return pri->publisherExists;
}

void QZeroConf::addServiceTxtRecord(QString nameOnly)
{
	pri->txtRecords.insert(nameOnly.toUtf8(), QByteArray());
}

void QZeroConf::addServiceTxtRecord(QString name, QString value)
{
	pri->txtRecords.insert(name.toUtf8(), value.toUtf8());
}

void QZeroConf::clearServiceTxtRecords()
{
	pri->txtRecords.clear();
}

void QZeroConf::startBrowser(QString type, QAbstractSocket::NetworkLayerProtocol protocol)
{
	pri->startBrowser(type, protocol);
}

void QZeroConf::stopBrowser(void)
{
	pri->stopBrowser();
}

bool QZeroConf::browserExists(void)
{
	return pri->browserExists;
}

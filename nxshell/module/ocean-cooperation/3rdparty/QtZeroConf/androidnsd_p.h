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
   File name    : androidnsd_p.h
   Created      : 10 Spetember 2021
   Author(s)    : Michael Zanetti
---------------------------------------------------------------------------------------------------
   NsdManager wrapper for use on Android devices
---------------------------------------------------------------------------------------------------
**************************************************************************************************/
#include "qzeroconf.h"
#include <QMap>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtAndroid>
#include <QtAndroidExtras>
#include <QAndroidJniObject>
#else
#include <QJniObject>
using QAndroidJniObject = QJniObject;
#endif

class QZeroConfPrivate: QObject
{
	Q_OBJECT
public:
	typedef QMap<QByteArray, QByteArray> TxtRecordMap;

	QZeroConfPrivate(QZeroConf *parent);
	~QZeroConfPrivate();
	void startServicePublish(const char *name, const char *type, quint16 port);
	void stopServicePublish();
	void startBrowser(QString type, QAbstractSocket::NetworkLayerProtocol protocol);
	void stopBrowser();
	static void onServiceResolvedJNI(JNIEnv */*env*/, jobject /*thiz*/, jlong id, jstring name, jstring type, jstring hostname, jstring address, jint port, jobject txtRecords);
	static void onServiceRemovedJNI(JNIEnv */*env*/, jobject /*this*/, jlong id, jstring name);
	static void onBrowserStateChangedJNI(JNIEnv */*env*/, jobject /*thiz*/, jlong id, jboolean running, jboolean error);
	static void onPublisherStateChangedJNI(JNIEnv */*env*/, jobject /*thiz*/, jlong id, jboolean running, jboolean error);
	static void onServiceNameChangedJNI(JNIEnv */*env*/, jobject /*thiz*/, jlong id, jstring newName);

	QZeroConf *pub;
	QAndroidJniObject nsdManager;

	bool browserExists = false;
	bool publisherExists = false;
	QMap<QByteArray, QByteArray> txtRecords;
	QString publishName;
	QString publishType;


private slots:
	void onServiceResolved(const QString &name, const QString &type, const QString &hostname, const QHostAddress &address, int port, const TxtRecordMap &txtRecords);
	void onServiceRemoved(const QString &name);
	void onBrowserStateChanged(bool running, bool error);
	void onPublisherStateChanged(bool running, bool error);
	void onServiceNameChanged(const QString &newName);
};

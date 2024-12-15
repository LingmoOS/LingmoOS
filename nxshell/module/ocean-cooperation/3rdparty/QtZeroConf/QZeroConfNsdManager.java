/**************************************************************************************************
---------------------------------------------------------------------------------------------------
        Copyright (C) 2021  Jonathan Bagg
        This file is part of QtZeroConf.

   Redistribution and use in source and binary forms, with or without modification, are permitted
   provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of
    conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other materials provided
    with the distribution.
  * Neither the name of Jonathan Bagg nor the names of its contributors may be used to
    endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------------------------------
   Project name : QtZeroConf
   File name    : QZeroConfNsdManager.java
   Created      : 10 Spetember 2021
   Author(s)    : Michael Zanetti
---------------------------------------------------------------------------------------------------
   NsdManager wrapper for use on Android devices
---------------------------------------------------------------------------------------------------
**************************************************************************************************/

package qtzeroconf;

import java.util.Map;
import java.util.ArrayList;

import android.util.Log;

import android.content.Context;

import android.net.nsd.NsdServiceInfo;
import android.net.nsd.NsdManager;

public class QZeroConfNsdManager {

	public static native void onServiceResolvedJNI(long id, String name, String type, String hostname, String address, int port, Map<String, byte[]> txtRecords);
	public static native void onServiceRemovedJNI(long id, String name);
	public static native void onBrowserStateChangedJNI(long id, boolean running, boolean error);
	public static native void onPublisherStateChangedJNI(long id, boolean running, boolean error);
	public static native void onServiceNameChangedJNI(long id, String newName);

	private static String TAG = "QZeroConfNsdManager";
	private long id;
	private Context context;
	private NsdManager nsdManager;
	private NsdManager.DiscoveryListener discoveryListener;
	private NsdManager.RegistrationListener registrationListener;
	private String registrationName; // The original service name that was given for registration, it might change on collisions

	// There can only be one resolver at a time per application, we'll need to queue the resolving
	static private ArrayList<NsdServiceInfo> resolverQueue = new ArrayList<NsdServiceInfo>();
	static private NsdServiceInfo pendingResolve = null;

	public QZeroConfNsdManager(long id, Context context) {
		super();
		this.id = id;
		this.context = context;

		nsdManager = (NsdManager)context.getSystemService(Context.NSD_SERVICE);
		discoveryListener = initializeDiscoveryListener();
		registrationListener = initializeRegistrationListener();
    }

	public void registerService(String name, String type, int port, Map<String, String> txtRecords) {
		registrationName = name;

		NsdServiceInfo serviceInfo = new NsdServiceInfo();
		serviceInfo.setServiceName(name);
		serviceInfo.setServiceType(type);
		serviceInfo.setPort(port);
		for (Map.Entry<String, String> entry: txtRecords.entrySet()) {
			serviceInfo.setAttribute(entry.getKey(), entry.getValue());
		}

		try {
			nsdManager.registerService(serviceInfo, NsdManager.PROTOCOL_DNS_SD, registrationListener);
		} catch (IllegalArgumentException e) {
			Log.w(TAG, "Error registering service: " + e.toString());
			onPublisherStateChangedJNI(id, false, true);
		}
	}

	public void unregisterService() {
		nsdManager.unregisterService(registrationListener);
	}

	public void discoverServices(String serviceType) {
		nsdManager.discoverServices(serviceType, NsdManager.PROTOCOL_DNS_SD, discoveryListener);
	}

	public void stopServiceDiscovery() {
		nsdManager.stopServiceDiscovery(discoveryListener);
	}

	private NsdManager.DiscoveryListener initializeDiscoveryListener() {
		return new NsdManager.DiscoveryListener() {

			@Override
			public void onDiscoveryStarted(String regType) {
				QZeroConfNsdManager.onBrowserStateChangedJNI(id, true, false);
			}

			@Override
			public void onServiceFound(NsdServiceInfo service) {
				enqueueResolver(service);
			}

			@Override
			public void onServiceLost(NsdServiceInfo serviceInfo) {
				QZeroConfNsdManager.onServiceRemovedJNI(id, serviceInfo.getServiceName());
			}

			@Override
			public void onDiscoveryStopped(String serviceType) {
				QZeroConfNsdManager.onBrowserStateChangedJNI(id, false, false);
			}

			@Override
			public void onStartDiscoveryFailed(String serviceType, int errorCode) {
				QZeroConfNsdManager.onBrowserStateChangedJNI(id, false, true);
			}

			@Override
			public void onStopDiscoveryFailed(String serviceType, int errorCode) {
				QZeroConfNsdManager.onBrowserStateChangedJNI(id, false, true);
			}
		};
	}

	private NsdManager.ResolveListener initializeResolveListener() {
		return new NsdManager.ResolveListener() {

			@Override
			public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
				Log.w(TAG, "Resolving failed for: " + serviceInfo.getServiceName() + " " + serviceInfo.getServiceType() + ": " + errorCode);
				if (errorCode == NsdManager.FAILURE_ALREADY_ACTIVE) {
					enqueueResolver(pendingResolve);
				}
				pendingResolve = null;
				processResolverQueue();
			}

			@Override
			public void onServiceResolved(NsdServiceInfo serviceInfo) {
				QZeroConfNsdManager.onServiceResolvedJNI(id,
					serviceInfo.getServiceName(),
					serviceInfo.getServiceType(),
					serviceInfo.getHost().getHostName(),
					serviceInfo.getHost().getHostAddress(),
					serviceInfo.getPort(),
					serviceInfo.getAttributes()
				);
				pendingResolve = null;
				processResolverQueue();
			}
		};
	}

	public NsdManager.RegistrationListener initializeRegistrationListener() {
		return new NsdManager.RegistrationListener() {

			@Override
			public void onServiceRegistered(NsdServiceInfo serviceInfo) {
				QZeroConfNsdManager.onPublisherStateChangedJNI(id, true, false);
				if (!serviceInfo.getServiceName().equals(registrationName)) {
					onServiceNameChangedJNI(id, serviceInfo.getServiceName());
				}
			}

			@Override
			public void onRegistrationFailed(NsdServiceInfo serviceInfo, int errorCode) {
				QZeroConfNsdManager.onPublisherStateChangedJNI(id, false, true);
			}

			@Override
			public void onServiceUnregistered(NsdServiceInfo arg0) {
				QZeroConfNsdManager.onPublisherStateChangedJNI(id, false, false);
			}

			@Override
			public void onUnregistrationFailed(NsdServiceInfo serviceInfo, int errorCode) {
				QZeroConfNsdManager.onPublisherStateChangedJNI(id, false, true);
			}
		};
	}

	private void enqueueResolver(NsdServiceInfo serviceInfo) {
		resolverQueue.add(serviceInfo);
		processResolverQueue();
	}

	private void processResolverQueue() {
		if (resolverQueue.isEmpty()) {
			return;
		}
		if (pendingResolve != null) {
			return;
		}
		pendingResolve = resolverQueue.get(0);
		resolverQueue.remove(0);
		nsdManager.resolveService(pendingResolve, initializeResolveListener());
	}
}

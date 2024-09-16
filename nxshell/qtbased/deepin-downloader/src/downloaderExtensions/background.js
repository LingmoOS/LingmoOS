// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

var QWebChannelMessageTypes = {
    signal: 1,
    propertyUpdate: 2,
    init: 3,
    idle: 4,
    debug: 5,
    invokeMethod: 6,
    connectToSignal: 7,
    disconnectFromSignal: 8,
    setProperty: 9,
    response: 10,
};

var QWebChannel = function(transport, initCallback)
{
    if (typeof transport !== "object" || typeof transport.send !== "function") {
        console.error("The QWebChannel expects a transport object with a send function and onmessage callback property." +
                      " Given is: transport: " + typeof(transport) + ", transport.send: " + typeof(transport.send));
        return;
    }

    var channel = this;
    this.transport = transport;

    this.send = function(data)
    {
        if (typeof(data) !== "string") {
            data = JSON.stringify(data);
        }
        channel.transport.send(data);
    }

    this.transport.onmessage = function(message)
    {
        var data = message.data;
        if (typeof data === "string") {
            data = JSON.parse(data);
        }
        switch (data.type) {
            case QWebChannelMessageTypes.signal:
                channel.handleSignal(data);
                break;
            case QWebChannelMessageTypes.response:
                channel.handleResponse(data);
                break;
            case QWebChannelMessageTypes.propertyUpdate:
                channel.handlePropertyUpdate(data);
                break;
            default:
                console.error("invalid message received:", message.data);
                break;
        }
    }

    this.execCallbacks = {};
    this.execId = 0;
    this.exec = function(data, callback)
    {
        if (!callback) {
            // if no callback is given, send directly
            channel.send(data);
            return;
        }
        if (channel.execId === Number.MAX_VALUE) {
            // wrap
            channel.execId = Number.MIN_VALUE;
        }
        if (data.hasOwnProperty("id")) {
            console.error("Cannot exec message with property id: " + JSON.stringify(data));
            return;
        }
        data.id = channel.execId++;
        channel.execCallbacks[data.id] = callback;
        channel.send(data);
    };

    this.objects = {};

    this.handleSignal = function(message)
    {
        var object = channel.objects[message.object];
        if (object) {
            object.signalEmitted(message.signal, message.args);
        } else {
            console.warn("Unhandled signal: " + message.object + "::" + message.signal);
        }
    }

    this.handleResponse = function(message)
    {
        if (!message.hasOwnProperty("id")) {
            console.error("Invalid response message received: ", JSON.stringify(message));
            return;
        }
        channel.execCallbacks[message.id](message.data);
        delete channel.execCallbacks[message.id];
    }

    this.handlePropertyUpdate = function(message)
    {
        for (var i in message.data) {
            var data = message.data[i];
            var object = channel.objects[data.object];
            if (object) {
                object.propertyUpdate(data.signals, data.properties);
            } else {
                console.warn("Unhandled property update: " + data.object + "::" + data.signal);
            }
        }
        channel.exec({type: QWebChannelMessageTypes.idle});
    }

    this.debug = function(message)
    {
        channel.send({type: QWebChannelMessageTypes.debug, data: message});
    };

    channel.exec({type: QWebChannelMessageTypes.init}, function(data) {
        for (var objectName in data) {
            var object = new QObject(objectName, data[objectName], channel);
        }
        // now unwrap properties, which might reference other registered objects
        for (var objectName in channel.objects) {
            channel.objects[objectName].unwrapProperties();
        }
        if (initCallback) {
            initCallback(channel);
        }
        channel.exec({type: QWebChannelMessageTypes.idle});
    });
};

function QObject(name, data, webChannel)
{
    this.__id__ = name;
    webChannel.objects[name] = this;

    // List of callbacks that get invoked upon signal emission
    this.__objectSignals__ = {};

    // Cache of all properties, updated when a notify signal is emitted
    this.__propertyCache__ = {};

    var object = this;

    // ----------------------------------------------------------------------

    this.unwrapQObject = function(response)
    {
        if (response instanceof Array) {
            // support list of objects
            var ret = new Array(response.length);
            for (var i = 0; i < response.length; ++i) {
                ret[i] = object.unwrapQObject(response[i]);
            }
            return ret;
        }
        if (!response
            || !response["__QObject*__"]
            || response.id === undefined) {
            return response;
        }

        var objectId = response.id;
        if (webChannel.objects[objectId])
            return webChannel.objects[objectId];

        if (!response.data) {
            console.error("Cannot unwrap unknown QObject " + objectId + " without data.");
            return;
        }

        var qObject = new QObject( objectId, response.data, webChannel );
        qObject.destroyed.connect(function() {
            if (webChannel.objects[objectId] === qObject) {
                delete webChannel.objects[objectId];
                // reset the now deleted QObject to an empty {} object
                // just assigning {} though would not have the desired effect, but the
                // below also ensures all external references will see the empty map
                // NOTE: this detour is necessary to workaround QTBUG-40021
                var propertyNames = [];
                for (var propertyName in qObject) {
                    propertyNames.push(propertyName);
                }
                for (var idx in propertyNames) {
                    delete qObject[propertyNames[idx]];
                }
            }
        });
        // here we are already initialized, and thus must directly unwrap the properties
        qObject.unwrapProperties();
        return qObject;
    }

    this.unwrapProperties = function()
    {
        for (var propertyIdx in object.__propertyCache__) {
            object.__propertyCache__[propertyIdx] = object.unwrapQObject(object.__propertyCache__[propertyIdx]);
        }
    }

    function addSignal(signalData, isPropertyNotifySignal)
    {
        var signalName = signalData[0];
        var signalIndex = signalData[1];
        object[signalName] = {
            connect: function(callback) {
                if (typeof(callback) !== "function") {
                    console.error("Bad callback given to connect to signal " + signalName);
                    return;
                }

                object.__objectSignals__[signalIndex] = object.__objectSignals__[signalIndex] || [];
                object.__objectSignals__[signalIndex].push(callback);

                if (!isPropertyNotifySignal && signalName !== "destroyed") {
                    // only required for "pure" signals, handled separately for properties in propertyUpdate
                    // also note that we always get notified about the destroyed signal
                    webChannel.exec({
                        type: QWebChannelMessageTypes.connectToSignal,
                        object: object.__id__,
                        signal: signalIndex
                    });
                }
            },
            disconnect: function(callback) {
                if (typeof(callback) !== "function") {
                    console.error("Bad callback given to disconnect from signal " + signalName);
                    return;
                }
                object.__objectSignals__[signalIndex] = object.__objectSignals__[signalIndex] || [];
                var idx = object.__objectSignals__[signalIndex].indexOf(callback);
                if (idx === -1) {
                    console.error("Cannot find connection of signal " + signalName + " to " + callback.name);
                    return;
                }
                object.__objectSignals__[signalIndex].splice(idx, 1);
                if (!isPropertyNotifySignal && object.__objectSignals__[signalIndex].length === 0) {
                    // only required for "pure" signals, handled separately for properties in propertyUpdate
                    webChannel.exec({
                        type: QWebChannelMessageTypes.disconnectFromSignal,
                        object: object.__id__,
                        signal: signalIndex
                    });
                }
            }
        };
    }

    /**
     * Invokes all callbacks for the given signalname. Also works for property notify callbacks.
     */
    function invokeSignalCallbacks(signalName, signalArgs)
    {
        var connections = object.__objectSignals__[signalName];
        if (connections) {
            connections.forEach(function(callback) {
                callback.apply(callback, signalArgs);
            });
        }
    }

    this.propertyUpdate = function(signals, propertyMap)
    {
        // update property cache
        for (var propertyIndex in propertyMap) {
            var propertyValue = propertyMap[propertyIndex];
            object.__propertyCache__[propertyIndex] = propertyValue;
        }

        for (var signalName in signals) {
            // Invoke all callbacks, as signalEmitted() does not. This ensures the
            // property cache is updated before the callbacks are invoked.
            invokeSignalCallbacks(signalName, signals[signalName]);
        }
    }

    this.signalEmitted = function(signalName, signalArgs)
    {
        invokeSignalCallbacks(signalName, this.unwrapQObject(signalArgs));
    }

    function addMethod(methodData)
    {
        var methodName = methodData[0];
        var methodIdx = methodData[1];
        object[methodName] = function() {
            var args = [];
            var callback;
            for (var i = 0; i < arguments.length; ++i) {
                var argument = arguments[i];
                if (typeof argument === "function")
                    callback = argument;
                else if (argument instanceof QObject && webChannel.objects[argument.__id__] !== undefined)
                    args.push({
                        "id": argument.__id__
                    });
                else
                    args.push(argument);
            }

            webChannel.exec({
                "type": QWebChannelMessageTypes.invokeMethod,
                "object": object.__id__,
                "method": methodIdx,
                "args": args
            }, function(response) {
                if (response !== undefined) {
                    var result = object.unwrapQObject(response);
                    if (callback) {
                        (callback)(result);
                    }
                }
            });
        };
    }

    function bindGetterSetter(propertyInfo)
    {
        var propertyIndex = propertyInfo[0];
        var propertyName = propertyInfo[1];
        var notifySignalData = propertyInfo[2];
        // initialize property cache with current value
        // NOTE: if this is an object, it is not directly unwrapped as it might
        // reference other QObject that we do not know yet
        object.__propertyCache__[propertyIndex] = propertyInfo[3];

        if (notifySignalData) {
            if (notifySignalData[0] === 1) {
                // signal name is optimized away, reconstruct the actual name
                notifySignalData[0] = propertyName + "Changed";
            }
            addSignal(notifySignalData, true);
        }

        Object.defineProperty(object, propertyName, {
            configurable: true,
            get: function () {
                var propertyValue = object.__propertyCache__[propertyIndex];
                if (propertyValue === undefined) {
                    // This shouldn't happen
                    console.warn("Undefined value in property cache for property \"" + propertyName + "\" in object " + object.__id__);
                }

                return propertyValue;
            },
            set: function(value) {
                if (value === undefined) {
                    console.warn("Property setter for " + propertyName + " called with undefined value!");
                    return;
                }
                object.__propertyCache__[propertyIndex] = value;
                var valueToSend = value;
                if (valueToSend instanceof QObject && webChannel.objects[valueToSend.__id__] !== undefined)
                    valueToSend = { "id": valueToSend.__id__ };
                webChannel.exec({
                    "type": QWebChannelMessageTypes.setProperty,
                    "object": object.__id__,
                    "property": propertyIndex,
                    "value": valueToSend
                });
            }
        });
    }



    data.methods.forEach(addMethod);

    data.properties.forEach(bindGetterSetter);

    data.signals.forEach(function(signal) { addSignal(signal, false); });

    for (var name in data.enums) {
        object[name] = data.enums[name];
    }
}

//required for use with nodejs
if (typeof module === 'object') {
    module.exports = {
        QWebChannel: QWebChannel
    };
}

// ----------------------------------------------------------------------

var socket;
var webChanel;
var downloadItem;
var isSelfCreate = false;
var isTakeOver = false;
var isOpen = false;
var isContainCookie = false;

function runDownloader() {
    var port = chrome.runtime.connectNative("browser.downloader.autostart");
    // port.onMessage.addListener(function (msg) {
    //   console.log("Received" + msg);
    // });
    // port.onDisconnect.addListener(function () {
    //   console.log("Disconnected");
    // });
    // port.postMessage({ type: "command", command: "deepin-app-store" });
}

function main() {
    //window.open("downloader:")
    socket  = new WebSocket("ws://localhost:12345");
    socket.onopen = function() {
        onSocketOpen();
    }
    socket.onerror = function() {
        console.log("websocket error")
        runDownloader();
        setTimeout(()=>{
            socket  = new WebSocket("ws://localhost:12345");
            socket.onopen = function() {
                onSocketOpen();
            }
            socket.onerror = function() {
                console.log("websocket error")
            }
            socket.onclose = function() {
                console.log("websocket close")
            }
            }, 2000);
    }
    socket.onclose = function() {
        console.log("websocket close")
    }
    
    var title = chrome.i18n.getMessage("downlaodby");
    addContextMenu("downloader", title);
}

main();

chrome.downloads.onCreated.addListener(function(item) {
    //console.log("chrome.downloads.onCreated")
    //console.log(item)

    if(item.state != "in_progress") {  //判断状态不是刚创建的任务，返回
        //console.log("不是刚创建的任务")
        return;
    }
    if(item.incognito) { //下载器不接管
        console.log("隐私模式")
        return;
    }
    if(!isTakeOver) { //下载器不接管
        console.log("下载器不接管")
        return;
    }
    
    if(item.filename != "" && item.mime == "text/html") {  //判断文件名不为空，即右键另存为的html，返回
        console.log("文件名不为空")
        return;
    }
    if(item.finalUrl.indexOf("chrome://") != -1 ) {  //是chrome内置网页，返回
        console.log("是chrome内置网页")
        return;
    }
    if(item.finalUrl.indexOf("file://") != -1) {  //是file://类型，返回
        console.log("是file://类型")
        return;
    }
    if(isContainCookie && (item.finalUrl.indexOf("ftp://") == -1)) {  //包含cookie，返回
        isContainCookie = false
        console.log("包含cookie")
        return
    }
    
    if(!isSelfCreate) {
        console.log("button Created")
        chrome.downloads.cancel(item.id),
        chrome.downloads.erase({
            id: item.id
        })
        setTimeout(()=>{
            downloadItem = item;
            if(item.finalUrl.indexOf("ftp://")!= -1 ){
                isSelfCreate = false
                sendUrlToDownloader(downloadItem);
            } else {
                console.log("item.finalUrl:" << item.finalUrl)
                if(item.finalUrl.indexOf("javascript:void(0);")== -1 ){
                    loadXMLDoc(item.finalUrl)
                }
            }
        }, 0);
        
    } else {
        isSelfCreate = false
        console.log("self Created")
    }
})

chrome.webRequest.onHeadersReceived.addListener(function(t) {
    return onHeadersReceived(t)
}, {
    urls: ["<all_urls>"]
}, ["blocking", "responseHeaders"])

chrome.webRequest.onBeforeSendHeaders.addListener(function(details) {
    console.log("chrome.webRequest.onBeforeSendHeaders")
    console.log(details)
    if(details.method == "HEAD") {
        console.log(details)
        removeBrowserHeaders(details.requestHeaders, /Cookie/i);
        return { requestHeaders: details.requestHeaders };
    }
    
    details.requestHeaders.some(function(header) {
        if( header.name == 'Cookie' ) {
            isContainCookie = true
            //console.log("isContainCookie = true")
            return;
        }
        isContainCookie = false
        //console.log("isContainCookie = false")
    })
    if(details.requestHeaders.length == 0) {
        isContainCookie = false
    }
    
}, {
    urls: ["<all_urls>"]
}, ["blocking", "requestHeaders", "extraHeaders"])

chrome.contextMenus.onClicked.addListener(onContextMenuClicked)

chrome.downloads.onDeterminingFilename.addListener(function(item, suggest) {
    // suggest({
    //      filename: item.filename,
    //      conflict_action: 'overwrite',
    //      conflictAction: 'overwrite'
    //     });
});

// window.onunload = function() {
//     console.log("windows.onunload")
//     webChanel.objects.core.receiveText("close");
// }

function removeBrowserHeaders(headers, regex) {
    for (var i = 0, header; (header = headers[i]); i++) {
      if (header.name.match(regex)) {
        headers.splice(i, 1);
        return;
        }
    }
}

function loadXMLDoc(url)
{
    xmlhttp=new XMLHttpRequest();
    if (xmlhttp!=null) {
        xmlhttp.onreadystatechange=onStateChange;
        isSelfCreate = true
        xmlhttp.open("HEAD",url,true);
        xmlhttp.send(null);
    } else {
        console.log("Your browser does not support XMLHTTP.");
    }
}

function onStateChange()
{
    isSelfCreate = false
    if (xmlhttp.readyState==4){// 4 = "loaded"
        if (xmlhttp.status==200){// 200 = OK
            console.log("xmlhttp.status==200")
            sendUrlToDownloader(downloadItem);
        } else {
            console.log("Purse url failed");
            downloadData(downloadItem.finalUrl)
        }
    }
}

function sendUrlToDownloader(item) {
    webChanel.objects.core.receiveText(item.finalUrl);
}

function onSocketOpen() {
    console.log("websocket Open")
    new QWebChannel(socket, function(channel) {
        console.log("QWebChannel new")
        webChanel = channel;
        setTimeout(()=>{
            webChanel.objects.core.receiveText("init");
            }, 200);
        
        webChanel.objects.core.sendText.connect(function(message) {
            console.log("message :" + message)
            
            if(message == "0"){
                isTakeOver = false;
            } else {
                isTakeOver = true;
            }
        })
    })
}

function downloadData(url) {
    console.log("redownload: " + url)

    chrome.downloads.setShelfEnabled(true);
    isSelfCreate = true
    chrome.downloads.download({
        url : url,
    });
}

function addContextMenu (id, title) {
    chrome.contextMenus.create({
        id: id,
        title: title,
        contexts: ['link']
    })
}


function onContextMenuClicked(info, tab) {
    console.log("onContextMenuClicked")
    if(info.linkUrl.indexOf("javascript:void(0);")!= -1 ){
        return;
    }
    if(isTakeOver) {
        webChanel.objects.core.receiveText(info.linkUrl);
        return
    }
    chrome.notifications.create(
        Math.random()+'',
        {       
          type: 'basic',
          iconUrl: 'icon128.png',
          title: chrome.i18n.getMessage("notifyTitle"),
          message: "",
          contextMessage: chrome.i18n.getMessage("notifyMessage"),
          eventTime: Date.now() + 200,
          //buttons: [{title:'setting'}]
        },
        (id)=>{
          console.log(id);
        }
    )
}

function onHeadersReceived(details) {
    if(isSelfCreate) {
        return
    }
    //console.log("onHeadersReceived : " + details.url)
    do {
        var code = details.statusCode;
        if (code >= 300 && code < 400 && 304 !== code)
            break;
        if (0 === details.statusLine.indexOf("HTTP/1.1 204 Intercepted by the Xunlei Advanced Integration"))
            break;
        var type = details.type;
        if (!isSupportRequestType(type))
            break;
        var url = details.url
        var o = requestItems[details.requestId];
        o ? delete requestItems[details.requestId] : (o = new urlInfo).tabId = details.tabId;
        for (var r = 0; r < details.responseHeaders.length; ++r) {
            var name = details.responseHeaders[r].name.toLowerCase()
              , value = details.responseHeaders[r].value;
            switch (name) {
            case "referer":
                o.headers.referer = value;
                break;
            case "set-cookie":
                0 === o.headers.cookie.length ? o.headers.cookie = value : o.headers.cookie = o.headers.cookie + "; " + value;
                break;
            case "access-control-allow-origin":
                originHender = "Origin: " + value;
                break;
            case "host":
                o.headers.host = value;
                break;
            case "content-disposition":
                o.headers["content-disposition"] = value;
                break;
            case "content-length":
                o.headers["content-length"] = value;
                break;
            case "content-type":
                o.headers["content-type"] = value
            }
        }


        var c = getFileNameExt(o.fileName);
        if (0 === c.length) {
            var u = getUrlFileName(o.url);
            c = getFileNameExt(u)
        }

        if(isSupportMediaExt(c)) {
            console.log("SupportMediaExt : " + c)
            chrome.downloads.setShelfEnabled(true);
            isSelfCreate = false
            chrome.downloads.download({
                url: u
            });
            return;
        }

        var h = o.headers["content-type"];
        if (isSupportContentType(h)) {
            console.log("SupportMediaExt : " + c)
            chrome.downloads.setShelfEnabled(true);
            isSelfCreate = false
            chrome.downloads.download({
                url: u
            });
            return;
        }

    } while (0);return {}
}


var MIME_TYPE_ARRAY = ["application/vnd.lotus-1-2-3", "image/x-3ds", "video/3gpp", "video/3gpp", "video/3gpp", "video/3gpp", "application/x-t602", "audio/x-mod", "application/x-7z-compressed",
                       "application/x-archive", "audio/mp4", "application/x-abiword", "application/x-abiword", "application/x-abiword", "audio/ac3", "application/x-ace", "text/x-adasrc", "text/x-adasrc",
                       "application/x-font-afm", "image/x-applix-graphics", "application/illustrator", "audio/x-aiff", "audio/x-aiff", "audio/x-aiff", "application/x-perl", "application/x-alz", "audio/amr",
                       "application/x-navi-animation", "video/x-anim", "application/annodex", "audio/x-ape", "application/x-arj", "image/x-sony-arw", "application/x-applix-spreadsheet", "text/plain",
                       "video/x-ms-asf", "application/x-asp", "text/x-ssa", "audio/x-ms-asx", "application/atom+xml", "audio/basic", "video/x-msvideo", "application/x-applix-word", "audio/amr-wb",
                       "application/x-awk", "audio/annodex", "video/annodex", "application/x-trash", "application/x-bcpio", "application/x-font-bdf", "text/x-bibtex", "application/x-blender",
                       "application/x-blender", "image/bmp", "application/x-bzip", "application/x-bzip", "text/x-csrc", "text/x-c++src", "application/vnd.ms-cab-compressed", "application/x-cb7",
                       "application/x-cbr", "application/x-cbt", "application/x-cbz", "text/x-c++src", "application/x-netcdf", "application/vnd.corel-draw", "application/x-x509-ca-cert", "application/x-x509-ca-cert",
                       "image/cgm", "application/x-chm", "application/x-kchart", "application/x-java", "text/x-tex", "text/x-cmake", "application/x-cpio", "application/x-cpio-compressed", "text/x-c++src",
                       "image/x-canon-cr2", "application/x-x509-ca-cert", "image/x-canon-crw", "text/x-csharp", "application/x-csh", "text/css", "text/css", "text/csv", "application/x-cue", "image/x-win-bitmap",
                       "text/x-c++src", "text/x-dsrc", "application/x-dar", "application/x-dbf", "application/x-dc-rom", "text/x-dcl", "application/dicom", "image/x-kodak-dcr", "image/x-dds", "application/x-deb",
                       "application/x-x509-ca-cert", "application/x-desktop", "application/x-dia-diagram", "text/x-patch", "video/x-msvideo", "image/vnd.djvu", "image/vnd.djvu", "image/x-adobe-dng",
                       "application/msword", "application/docbook+xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.document", "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
                       "text/vnd.graphviz", "text/x-dsl", "application/xml-dtd", "text/x-tex", "video/dv", "application/x-dvi", "application/x-bzdvi", "application/x-gzdvi", "image/vnd.dwg", "image/vnd.dxf",
                       "text/x-eiffel", "application/x-egon", "text/x-eiffel", "text/x-emacs-lisp", "image/x-emf", "application/vnd.emusic-emusic_package", "application/xml-external-parsed-entity", "image/x-eps",
                       "image/x-bzeps", "image/x-gzeps", "image/x-eps", "image/x-bzeps", "image/x-gzeps", "image/x-eps", "image/x-bzeps", "image/x-gzeps", "application/epub+zip", "text/x-erlang",
                       "application/ecmascript", "application/x-e-theme", "text/x-setext", "application/x-ms-dos-executable", "image/x-exr", "application/andrew-inset", "text/x-fortran", "text/x-fortran",
                       "text/x-fortran", "application/x-fictionbook+xml", "image/x-xfig", "image/fits", "application/x-fluid", "audio/x-flac", "video/x-flic", "video/x-flic", "video/x-flv", "application/x-kivio",
                       "text/x-xslfo", "text/x-fortran", "image/fax-g3", "application/x-gameboy-rom", "application/x-gba-rom", "text/directory", "application/x-gedcom", "application/x-gedcom",
                       "application/x-genesis-rom", "application/x-tex-gf", "application/x-sms-rom", "image/gif", "application/x-glade", "application/x-gettext-translation", "application/x-gnucash",
                       "application/gnunet-directory", "application/x-gnucash", "application/x-gnumeric", "application/x-gnuplot", "application/x-gnuplot", "application/pgp-encrypted", "application/x-gnuplot",
                       "application/x-graphite", "application/x-font-type1", "audio/x-gsm", "application/x-tar", "text/vnd.graphviz", "text/x-google-video-pointer", "application/x-gzip", "text/x-chdr", "text/x-c++hdr",
                       "application/x-hdf", "text/x-c++hdr", "text/x-c++hdr", "application/vnd.hp-hpgl", "text/x-c++hdr", "text/x-haskell", "application/x-hwp", "application/x-hwt",
                       "text/x-c++hdr", "application/x-ica", "image/x-tga", "image/x-icns", "image/vnd.microsoft.icon", "text/calendar", "text/x-idl", "image/ief", "image/x-iff", "image/x-ilbm", "text/x-imelody",
                       "text/x-imelody", "text/x-tex", "text/x-iptables", "application/x-cd-image", "application/x-cd-image", "audio/x-it", "image/jp2", "text/vnd.sun.j2me.app-descriptor", "application/x-java-archive",
                       "text/x-java", "image/x-jng", "application/x-java-jnlp-file", "image/jp2", "image/jp2", "image/jp2", "image/jpeg", "application/x-jbuilder-project", "image/jp2", "application/javascript",
                       "application/jsonp", "image/x-kodak-k25", "audio/midi", "application/x-karbon", "image/x-kodak-kdc", "application/x-desktop", "application/x-kexiproject-sqlite3",
                       "application/x-kexi-connectiondata", "application/x-kexiproject-shortcut", "application/x-kformula", "application/x-killustrator", "application/smil", "application/vnd.google-earth.kml+xml",
                       "application/vnd.google-earth.kmz", "application/x-kontour", "application/x-kpovmodeler", "application/x-kpresenter", "application/x-kpresenter", "application/x-krita", "application/x-kspread",
                       "application/x-kugar", "application/x-kword", "application/x-kword", "application/x-shared-library-la", "text/x-tex", "text/x-ldif", "application/x-lha", "text/x-literate-haskell",
                       "application/x-lhz", "text/x-log", "text/x-tex", "text/x-lua", "image/x-lwo", "image/x-lwo", "image/x-lws", "text/x-lilypond", "application/x-lyx", "application/x-lzip", "application/x-lha",
                       "application/x-lzma", "application/x-lzop", "text/x-matlab", "audio/x-mod", "video/mpeg", "audio/x-mpegurl", "audio/x-mpegurl", "application/x-m4", "audio/mp4", "audio/x-m4b", "video/mp4",
                       "application/x-markaby", "application/x-troff-man", "application/mbox", "application/x-genesis-rom", "application/vnd.ms-access", "image/vnd.ms-modi", "text/x-troff-me", "audio/x-mod",
                       "application/metalink+xml", "application/x-magicpoint", "audio/midi", "audio/midi", "application/x-mif", "audio/x-minipsf", "audio/x-matroska", "video/x-matroska", "text/x-ocaml", "text/x-ocaml",
                       "text/x-troff-mm", "application/x-smaf", "text/mathml", "video/x-mng", "application/x-gettext-translation", "audio/x-mo3", "text/x-moc", "audio/x-mod", "text/x-mof", "video/quicktime",
                       "video/quicktime", "video/x-sgi-movie", "audio/x-musepack", "video/mpeg", "audio/mpeg", "video/mp4", "audio/x-musepack", "video/mpeg", "video/mpeg", "video/mpeg", "audio/mpeg", "audio/x-musepack",
                       "text/x-mrml", "text/x-mrml", "image/x-minolta-mrw", "text/x-troff-ms", "application/x-msi", "image/x-msod", "application/x-msx-rom", "audio/x-mod", "text/x-mup", "application/mxf",
                       "application/x-n64-rom", "application/mathematica", "application/x-netcdf", "application/x-nintendo-ds-rom", "image/x-nikon-nef", "application/x-nes-rom", "text/x-nfo", "text/x-mup",
                       "application/x-netshow-channel", "video/x-nsv", "application/x-object", "application/x-tgif", "text/x-ocl", "application/oda", "application/vnd.oasis.opendocument.database",
                       "application/vnd.oasis.opendocument.chart", "application/vnd.oasis.opendocument.formula", "application/vnd.oasis.opendocument.graphics", "application/vnd.oasis.opendocument.image",
                       "application/vnd.oasis.opendocument.text-master", "application/vnd.oasis.opendocument.presentation", "application/vnd.oasis.opendocument.spreadsheet", "application/vnd.oasis.opendocument.text",
                       "audio/ogg", "video/x-theora+ogg", "video/x-ogm+ogg", "video/ogg", "application/ogg", "application/x-trash", "application/x-oleo", "text/x-opml+xml", "image/openraster", "image/x-olympus-orf",
                       "application/vnd.oasis.opendocument.chart-template", "application/x-font-otf", "application/vnd.oasis.opendocument.graphics-template", "application/vnd.oasis.opendocument.text-web",
                       "application/vnd.oasis.opendocument.presentation-template", "application/vnd.oasis.opendocument.spreadsheet-template", "application/vnd.oasis.opendocument.text-template", "application/rdf+xml",
                       "application/vnd.openofficeorg.extension", "text/x-pascal", "application/pkcs10", "application/x-pkcs12", "application/x-pkcs7-certificates", "application/pkcs7-signature",
                       "application/x-java-pack200", "application/x-pak", "application/x-par2", "text/x-pascal", "text/x-patch", "image/x-portable-bitmap", "image/x-photo-cd", "application/x-cisco-vpn-settings",
                       "application/x-font-pcf", "application/x-font-pcf", "application/vnd.hp-pcl", "image/x-pcx", "chemical/x-pdb", "application/x-aportisdoc", "application/pdf", "application/x-bzpdf",
                       "application/x-gzpdf", "image/x-pentax-pef", "application/x-x509-ca-cert", "application/x-perl", "application/x-font-type1", "application/x-font-type1", "application/x-pkcs12",
                       "image/x-portable-graymap", "application/x-chess-pgn", "application/pgp-encrypted", "application/x-php", "application/x-php", "application/x-php", "image/x-pict", "image/x-pict",
                       "image/x-pict", "application/python-pickle", "application/x-tex-pk", "application/pkix-pkipath", "application/pgp-keys", "application/x-perl", "audio/x-iriver-pla", "application/x-planperfect",
                       "audio/x-scpls", "application/x-perl", "image/png", "image/x-portable-anymap", "image/x-macpaint", "text/x-gettext-translation", "application/x-spss-por", "text/x-gettext-translation-template",
                       "image/x-portable-pixmap", "application/vnd.ms-powerpoint", "application/vnd.ms-powerpoint", "application/vnd.openxmlformats-officedocument.presentationml.presentation",
                       "application/vnd.openxmlformats-officedocument.presentationml.presentation", "application/vnd.ms-powerpoint", "application/x-palm-database", "application/postscript", "application/x-bzpostscript",
                       "application/x-gzpostscript", "image/vnd.adobe.photoshop", "audio/x-psf", "application/x-gz-font-linux-psf", "audio/x-psflib", "audio/prs.sid", "application/x-pocket-word", "application/x-pw",
                       "text/x-python", "application/x-python-bytecode", "application/x-python-bytecode", "image/x-quicktime", "video/quicktime", "image/x-quicktime", "application/x-quicktime-media-link",
                       "video/quicktime", "audio/vnd.rn-realaudio", "image/x-fuji-raf", "application/ram", "application/x-rar", "image/x-cmu-raster", "image/x-panasonic-raw", "audio/vnd.rn-realaudio",
                       "application/x-ruby", "application/rdf+xml", "application/rdf+xml", "text/x-ms-regedit", "application/x-reject", "image/x-rgb", "image/rle", "application/vnd.rn-realmedia",
                       "application/vnd.rn-realmedia", "application/vnd.rn-realmedia", "application/vnd.rn-realmedia", "application/vnd.rn-realmedia", "application/vnd.rn-realmedia", "text/troff", 
                       "image/vnd.rn-realpix", "application/x-rpm", "application/rss+xml", "text/vnd.rn-realtext", "application/rtf", "text/richtext", "video/vnd.rn-realvideo", "video/vnd.rn-realvideo", 
                       "audio/x-s3m", "application/x-amipro", "application/x-sami", "application/x-spss-sav", "text/x-scheme", "application/vnd.stardivision.draw", "application/vnd.stardivision.calc", 
                       "application/vnd.stardivision.impress", "application/sdp", "application/vnd.stardivision.chart", "application/vnd.stardivision.writer", "application/x-go-sgf", "image/x-sgi", 
                       "application/vnd.stardivision.writer", "text/sgml", "text/sgml", "application/x-shellscript", "application/x-shar", "application/x-shorten", "application/x-siag", "audio/prs.sid", 
                       "application/x-trash", "application/vnd.symbian.install", "x-epoc/x-sisx-app", "application/x-stuffit", "application/sieve", "image/x-skencil", "image/x-skencil", "application/pgp-keys",
                       "text/spreadsheet", "application/x-smaf", "application/x-snes-rom", "application/vnd.stardivision.mail", "application/vnd.stardivision.math", "application/x-sami", "application/smil", 
                       "application/smil", "application/x-sms-rom", "audio/basic", "application/x-sharedlib", "application/x-pkcs7-certificates", "application/x-font-speedo", "text/x-rpm-spec", 
                       "application/x-shockwave-flash", "audio/x-speex", "text/x-sql", "image/x-sony-sr2", "application/x-wais-source", "image/x-sony-srf", "application/x-subrip", "text/x-ssa", 
                       "application/vnd.sun.xml.calc.template", "application/vnd.sun.xml.draw.template", "application/vnd.sun.xml.impress.template", "audio/x-stm", "application/vnd.sun.xml.writer.template", 
                       "text/x-tex", "text/x-subviewer", "image/x-sun-raster", "application/x-sv4cpio", "application/x-sv4crc", "image/svg+xml", "image/svg+xml-compressed", "application/x-shockwave-flash", 
                       "application/vnd.sun.xml.calc", "application/vnd.sun.xml.draw", "application/vnd.sun.xml.writer.global", "application/vnd.sun.xml.impress", "application/vnd.sun.xml.math", 
                       "application/vnd.sun.xml.writer", "text/spreadsheet", "text/troff", "text/x-txt2tags", "application/x-tar", "application/x-bzip-compressed-tar", "application/x-bzip-compressed-tar",
                       "application/x-compressed-tar", "application/x-lzma-compressed-tar", "application/x-tzo", "application/x-xz-compressed-tar", "application/x-tarz", "application/x-bzip-compressed-tar",
                       "application/x-bzip-compressed-tar", "text/x-tcl", "text/x-tex", "text/x-texinfo", "text/x-texinfo", "image/x-tga", "application/x-compressed-tar", "application/x-theme", 
                       "application/x-windows-themepack", "image/tiff", "image/tiff", "text/x-tcl", "application/x-lzma-compressed-tar", "application/vnd.ms-tnef", "application/vnd.ms-tnef", 
                       "application/x-cdrdao-toc", "application/x-bittorrent", "image/x-tga", "text/troff", "application/x-linguist", "text/tab-separated-values", "audio/x-tta", "application/x-font-ttf", 
                       "application/x-font-ttf", "application/x-font-ttx", "text/plain", "application/x-xz-compressed-tar", "application/x-tzo", "application/x-ufraw", "application/x-designer", 
                       "text/x-uil", "audio/x-mod", "audio/x-mod", "text/x-uri", "text/x-uri", "application/x-ustar", "text/x-vala", "text/x-vala", "text/directory", "text/calendar", "text/directory",
                       "image/x-tga", "text/x-vhdl", "text/x-vhdl", "video/vivo", "video/vivo", "audio/x-mpegurl", "video/mpeg", "audio/x-voc", "application/vnd.stardivision.writer", "image/x-tga", 
                       "audio/x-wav", "audio/x-ms-asx", "application/x-quattropro", "application/x-quattropro", "application/x-quattropro", "image/vnd.wap.wbmp", "application/vnd.ms-works", 
                       "application/vnd.ms-works", "video/webm", "application/vnd.lotus-1-2-3", "application/vnd.lotus-1-2-3", "application/vnd.lotus-1-2-3", "application/vnd.ms-works", "audio/x-ms-wma", 
                       "image/x-wmf", "text/vnd.wap.wml", "text/vnd.wap.wmlscript", "video/x-ms-wmv", "audio/x-ms-asx", "application/vnd.wordperfect", "application/vnd.wordperfect", "application/vnd.wordperfect", 
                       "application/vnd.wordperfect", "application/vnd.wordperfect", "application/x-wpg", "application/vnd.ms-wpl", "application/vnd.wordperfect", "application/vnd.ms-works", 
                       "application/x-mswrite", "model/vrml", "audio/x-wavpack", "audio/x-wavpack-correction", "audio/x-wavpack", "audio/x-ms-asx", "image/x-sigma-x3f", "application/x-gnucash", 
                       "application/x-xbel", "image/x-xbitmap", "image/x-xcf", "image/x-compressed-xcf", "image/x-compressed-xcf", "application/xhtml+xml", "audio/x-xi", 
                       "application/vnd.ms-excel", "application/vnd.ms-excel", "application/vnd.ms-excel", "application/x-xliff", "application/x-xliff", "application/vnd.ms-excel", "application/vnd.ms-excel", 
                       "application/vnd.ms-excel", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", 
                       "application/vnd.ms-excel", "application/vnd.ms-excel", "audio/x-xm", "audio/x-xmf", "text/x-xmi", "image/x-xpixmap", "application/vnd.ms-xpsdocument",
                        "text/x-xslfo", "application/xspf+xml", "application/vnd.mozilla.xul+xml", "image/x-xwindowdump", "chemical/x-pdb", "application/x-xz", 
                       "application/w2p", "application/x-compress", "application/x-abiword", "application/zip", "application/x-zoo"]


var SUPPORT_MEDIA_EXT_ARRAY = [".asf", ".avi", ".exe", ".iso", ".mp3", ".mpeg", ".mpg", ".mpga", ".ra", ".rar", ".rm", ".rmvb", ".tar", ".wma", ".wmp", ".wmv", ".mov", ".zip", ".3gp", ".chm", ".mdf", 
                               ".torrent", ".jar", ".msi", ".arj", ".bin", ".dll", ".psd", ".hqx", ".sit", ".lzh", ".gz", ".tgz", ".xlsx", ".xls", ".doc", ".docx", ".ppt", ".pptx", ".flv", ".swf", 
                               ".mkv", ".tp", ".ts", ".flac", ".ape", ".wav", ".aac", ".txt", ".crx", ".dat", ".7z", ".ttf", ".bat", ".xv", ".xvx", ".pdf", ".mp4", ".apk", ".ipa", ".epub", ".mobi", ".deb", 
                               ".sisx", ".cab", ".pxl"]

var SUPPORT_REQUEST_TYPE_ARRAY = ["main_frame", "sub_frame", "object", "xmlhttprequest", "other", "media"]
var requestItems = {}

function urlInfo() {
    this.headers = {},
    this.headers["user-agent"] = "",
    this.headers.referer = "",
    this.headers.cookie = "",
    this.headers["content-type"] = "",
    this.headers["content-disposition"] = "",
    this.headers.host = "",
    this.headers["content-length"] = 0,
    this.headers["access-control-allow-origin"] = "",
    this.url = "",
    this.fileName = "",
    this.ext = "",
    this.postData = "",
    this.tabId = void 0
}



function getUrlFileName(e) {
    var t = e.replace(/\?.*$/, "").replace(/.*\//, "");
    return decodeURIComponent(t)
}

function getFileNameExt(e) {
    var t = "";
    if (e && e.length > 0) {
        var i = e.lastIndexOf(".");
        -1 !== i && (t = (t = e.substr(i)).toLowerCase())
    }
    return t
}

function isSupportMediaExt(e) {
    for (var t in e = e.toLowerCase(),
    SUPPORT_MEDIA_EXT_ARRAY)
        if (e.toLowerCase() === SUPPORT_MEDIA_EXT_ARRAY[t])
            return !0;
    return !1
}

function isSupportContentType(e) {
    for (var t in e = e.toLowerCase(),
    MIME_TYPE_ARRAY)
        if (e.toLowerCase().indexOf(MIME_TYPE_ARRAY[t]) != -1)
            return !0;
    return !1
}

function isSupportRequestType(e) {
    for (var t in e = e.toLowerCase(),
    SUPPORT_REQUEST_TYPE_ARRAY)
        if (e === SUPPORT_REQUEST_TYPE_ARRAY[t])
            return !0;
    return !1
}


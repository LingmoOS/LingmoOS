#!/usr/bin/env python

# List of libraries to link against / preload.
# If linker flags are given then the library is linked againts, 
# and if the flags is None then the library will be dlopen()ed.

# Real build dependencies, you need to update these if they change in debian/control
real_build_deps = set(['cmake (>= 2.6.0)', 
                       'debhelper (>= 7)', 
                       'libqt4-dev (>= 4.5.0)', 
                       'libmeegotouch-dev', 
                       'libxtst-dev', 
                       'libxext-dev', 
                       'libxi-dev', 
                       'doxygen', 
                       'python (>= 2.6)',
                       'python (<< 2.7)'])

# Real runtime dependencies for applauncherd-launcher, you may need to update these as well
real_runtime_deps = set(['${shlibs:Depends}', 
                         '${misc:Depends}', 
                         'applauncherd-single-instance'])

# Library statuses
D = 0   # dlopen
L = 1   # link

# Library modifiers for dlopen (D-status only!), do not use modifiers below for L-status (link) libraries
# Add extra-symbol on first position of library name to control dlopen mode. See dlopen(3) for details.
# 'N' - default, dlopen mode is RTLD_NOW | RTLD_GLOBAL, ex "N/usr/lib/libsomelibrary.so" is equvalent to "/usr/lib/libsomelibrary.so": RTLD_NOW | RTLD_GLOBAL will be used for dlopen
# 'L' - dlopen mode is RTLD_LAZY | RTLD_GLOBAL, ex "L/usr/lib/libsomelibrary.so": RTLD_LAZY | RTLD_GLOBAL will be used for dlopen
# 'D' - dlopen mode is RTLD_DEEPBIND | RTLD_GLOBAL, ex "D/usr/lib/libsomelibrary.so": RTLD_DEEPBIND | RTLD_GLOBAL will be used for dlopen
# '#' - library will NOT be dlopened, ex "#/usr/lib/libsomelibrary.so" - will not be dlopened


libraries_nokia = [
    # Library                                                              Linker flags, Binary package, Dev package
    #(L, "/usr/lib/libcontactsvoicemail.so.0",                              "-lcontactsvoicemail", "libcontactsvoicemail0", "libcontactswidgets-dev"),
    #(L, "/usr/lib/libmaemomeegotouchcalendarinterface.so.1",               "-lmaemomeegotouchcalendarinterface", "maemo-meegotouch-interfaces", "maemo-meegotouch-interfaces-dev"),
    #(L, "/usr/lib/libqtcontacts_extensions_tracker.so.4",                  "-lqtcontacts_extensions_tracker", "libqtcontacts-extensions-tracker0", "libqtcontacts-extensions-tracker-dev"),
    #(L, "/usr/lib/libcontactsutils.so.0",                                  "-lcontactsutils", "libcontactswidgets0", "libcontactswidgets-dev"),
    (L, "/usr/lib/libcontactswidgets.so.0",                                "-lcontactswidgets", "libcontactswidgets0", "libcontactswidgets-dev"),
    (D, "/usr/lib/libpvr2d_r125.so",                                       "-lpvr2d_r125", "opengles-sgx-img-common", "opengles-sgx-img-common-dev"),
    (D, "/usr/lib/libpvrPVR2D_DRI2WSEGL_r125.so",                          "-lpvrPVR2D_DRI2WSEGL_r125", "opengles-sgx-img-common", "opengles-sgx-img-common-dev"),
    (L, "/usr/lib/libdatepicker2.so",                                      "-ldatepicker2", "libdatepicker2", "libdatepicker2"),
    #(L, "/usr/lib/libvideo-widget-meegotouch.so.1",                        "-lvideo-widget-meegotouch", "libvideo-widget-meegotouch", "libvideo-widget-meegotouch-dev"),
    #(L, "/usr/lib/libvideo-renderer-gst-playbin2.so.1",                    "-lvideo-renderer-gst-playbin2", "libvideo-renderer-gst-playbin2", "libvideo-renderer-gst-playbin2-dev"),
    #(L, "/usr/lib/libAccountPlugin.so.1",                                  "-lAccountPlugin", "libaccountplugin0", "libaccountplugin-dev"),
    #(L, "/usr/lib/libmaemomeegotouchcontactsinterface.so.1",               "-lmaemomeegotouchcontactsinterface", "maemo-meegotouch-interfaces", "maemo-meegotouch-interfaces-dev"),
    #(L, "/usr/lib/libgeoengine-mapwidget.so.1",                            "-lgeoengine-mapwidget", "geoengine-mapwidget", "geoengine-mapwidget-dev"),
    #(L, "/usr/lib/libmcontentwidgets.so.0",                                "-lmcontentwidgets", "libmcontentwidgets0", "libmcontentwidgets-dev"),
    #(L, "/usr/lib/libQtVersit.so.1",                                       "-lQtVersit", "libqtm-versit", "libqtm-versit-dev"),
    #(L, "/usr/lib/libMOS.so.1",                                            "-lMOS", "libmos", "libmos-dev"),
    #(L, "/usr/lib/libgallerycore.so.1",                                    "-lgallerycore", "libgallerycore", "libgallerycore-dev"),
    (L, "/usr/lib/liblocationpicker.so",                                   "-llocationpicker", "liblocationpicker", "liblocationpicker"),
    #(L, "/usr/lib/libAccountSetup.so.1",                                   "-lAccountSetup", "libaccountsetup0", "libaccountsetup-dev"),
    (L, "/usr/lib/liblocationextras.so",                                   "-llocationextras", "liblocationextras", "liblocationextras"),
    #(L, "/usr/lib/libNAccountsUI.so.1",                                    "-lNAccountsUI", "libnaccounts-ui0", "libnaccounts-ui-dev"),
    #(L, "/usr/lib/libmdatauri.so.0",                                       "-lmdatauri", "libmdatauri0", "libmdatauri-dev"),
    #(L, "/usr/lib/libgeoengine.so.1",                                      "-lgeoengine", "geoengine", "geoengine-dev"),
    (L, "/usr/lib/libSignOnUI.so",                                         "-lSignOnUI", "libsignon-ui0", "libsignon-ui0"),
    #(L, "/usr/lib/libgq-gconf.so.0",                                       "-lgq-gconf", "libgq-gconf0", "libgq-gconf-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqsvg.so",                    "/usr/lib/qt4/plugins/imageformats/libqsvg.so", "libqt4-svg", "libqt4-svg"),
    #(L, "/usr/lib/libgstinterfaces-0.10.so.0",                             "-lgstinterfaces-0.10", "libgstreamer-plugins-base0.10-0", "libgstreamer-plugins-base0.10-dev"),
    #(L, "/usr/lib/libgstbase-0.10.so.0",                                   "-lgstbase-0.10", "libgstreamer0.10-0", "libgstreamer0.10-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqico.so",                    "/usr/lib/qt4/plugins/imageformats/libqico.so", "libqtgui4", "libqtgui4"),
    #(L, "/usr/lib/libquill.so.1",                                          "-lquill", "libquill1", "libquill-dev"),
    #(L, "/usr/lib/libidn.so.11",                                           "-lidn", "libidn11", "libidn11-dev"),
    (L, "/usr/lib/libmeegocontrol.so",                                     "-lmeegocontrol", "libmeegocontrol0", "libmeegocontrol0"),
    #(L, "/usr/lib/libcares.so.2",                                          "-lcares", "libc-ares2", "libc-ares-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqtiff.so",                   "/usr/lib/qt4/plugins/imageformats/libqtiff.so", "libqtgui4", "libqtgui4-dev"),
    #(L, "/usr/lib/libgstpbutils-0.10.so.0",                                "-lgstpbutils-0.10", "libgstreamer-plugins-base0.10-0", "libgstreamer-plugins-base0.10-dev"),
    (D, "/usr/lib/libambase.so",                                           "-lambase", "libamjpeg1", "libamjpeg1"),
    #(L, "/usr/lib/libcellular-qt.so.1",                                    "-lcellular-qt", "libcellular-qt1", "libcellular-qt-dev"),
    #(L, "/usr/lib/libmoi.so.0",                                            "-lmoi", "libmoi0", "libmoi-dev"),
    #(L, "/usr/lib/libcurl.so.4",                                           "-lcurl", "libcurl3", "libcurl4-openssl-dev"),
    #(L, "/usr/lib/libcommhistory.so.0",                                    "-lcommhistory", "libcommhistory0", "libcommhistory-dev"),
    (L, "/usr/lib/libmessagingif0.so",                                     "-lmessagingif0", "libmessagingif0", "libmessagingif0"),
    #(L, "/usr/lib/libgstvideo-0.10.so.0",                                  "-lgstvideo-0.10", "libgstreamer-plugins-base0.10-0", "libgstreamer-plugins-base0.10-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqgif.so",                    "/usr/lib/qt4/plugins/imageformats/libqgif.so", "libqtgui4", "libqtgui4"),
    #(L, "/usr/lib/libtiff.so.4",                                           "-ltiff", "libtiff4", "libtiff4-dev"),
    (L, "/usr/lib/libresource.so",                                         "-lresource", "libresource0", "libresource0"),
    #(L, "/usr/lib/libgstreamer-0.10.so.0",                                 "-lgstreamer-0.10", "libgstreamer0.10-0", "libgstreamer0.10-dev"),
    #(L, "/usr/lib/libQtLocation.so.1",                                     "-lQtLocation", "libqtm-location", "libqtm-location-dev"),
    #(L, "/usr/lib/libexempi.so.3",                                         "-lexempi", "libexempi3", "libexempi-dev"),
    #(L, "/usr/lib/libquillimagefilter.so.1",                               "-lquillimagefilter", "libquillimagefilter1", "libquillimagefilter-dev"),
    #(L, "/usr/lib/libexif.so.12",                                          "-lexif", "libexif12", "libexif-dev"),
    #(L, "/usr/lib/libresourceqt.so.1",                                     "-lresourceqt", "libresourceqt1", "libresourceqt-dev"),
    #(L, "/usr/lib/libquillmetadata.so.1",                                  "-lquillmetadata", "libquillmetadata1", "libquillmetadata-dev"),
    #(L, "/usr/lib/libprofile.so.0",                                        "-lprofile", "libprofile0", "libprofile-dev"),
    (L, "/usr/lib/libsaveas.so",                                           "-lsaveas", "libsaveas", "libsaveas"),
    #(L, "/usr/lib/libsignoncrypto-qt.so.1",                                "-lsignoncrypto-qt", "libsignoncrypto-qt", "libsignoncrypto-qt-dev"),
    (L, "/usr/lib/libsignon-qt.so",                                        "-lsignon-qt", "libsignon-qt0", "libsignon-qt0"),
    #(L, "/usr/lib/libmaemomeegotouchshareuiinterface.so.1",                "-lmaemomeegotouchshareuiinterface", "maemo-meegotouch-interfaces", "maemo-meegotouch-interfaces-dev"),
    #(L, "/usr/lib/libQtContacts.so.1",                                     "-lQtContacts", "libqtm-contacts", "libqtm-contacts-dev"),
    (L, "/usr/lib/libduicontrolpanel.so",                                  "-lduicontrolpanel", "libduicontrolpanel", "libduicontrolpanel"),
    #(L, "/usr/lib/libaccounts-qt.so.1",                                    "-laccounts-qt", "libaccounts-qt1", "libaccounts-qt-dev"),
    #(L, "/usr/lib/libdbus-qeventloop.so.1",                                "-ldbus-qeventloop", "libdbus-qeventloop1", "libdbus-qeventloop-dev"),
    #(L, "/usr/lib/libaccounts-glib.so.0",                                  "-laccounts-glib", "libaccounts-glib0", "libaccounts-glib-dev"),
    #(L, "/usr/lib/libsmartsearch.so",                                      "-lsmartsearch", "libsmartsearch0", "libsmartsearch0"),
    #(L, "/usr/lib/libhal.so.1",                                            "-lhal", "libhal1", "libhal-dev"),
    #(L, "/usr/lib/libcontentmanager.so.0",                                 "-lcontentmanager", "libcontentmanager0", "libcontentmanager-dev"),
    #(L, "/usr/lib/librelevance.so.0",                                      "-lrelevance", "librelevance0", "librelevance-dev"),
    #(L, "/usr/lib/libdsme.so.0.2.0",                                       "-ldsme", "libdsme0.2.0", "libdsme0.2.0-dev"),
    #(L, "/usr/lib/libtimed.so.0",                                          "-ltimed", "libtimed0", "libtimed-dev"),
    #(L, "/usr/lib/libqmsystem2.so.1",                                      "-lqmsystem2", "libqmsystem2", "libqmsystem2-dev"),
    (L, "/lib/libnss_files.so.2",                                          "-lnss_files", "libc6", "libc6"),
    (L, "/usr/lib/libsensordatatypes.so",                                  "-lsensordatatypes", "sensord", "sensord"),
    (L, "/usr/lib/libsensorclient.so",                                     "-lsensorclient", "sensord", "sensord"),
    #(L, "/usr/lib/libdsme_dbus_if.so.0.2.0",                               "-ldsme_dbus_if", "libdsme0.2.0", "libdsme0.2.0-dev"),
    #(L, "/usr/lib/libqmlog.so.0",                                          "-lqmlog", "libqmlog0", "libqmlog-dev"),
    #(L, "/lib/libuuid.so.1",                                               "-luuid", "libuuid1", "uuid-dev"),
    (L, "/usr/lib/libiphb.so",                                             "-liphb", "libiphb0", "libiphb0"),
    #(L, "/usr/lib/libthumbnailer.so.0",                                    "-lthumbnailer", "libthumbnailer0", "libthumbnailer-dev"),
    #(L, "/usr/lib/libsysinfo.so.0",                                        "-lsysinfo", "libsysinfo0", "libsysinfo-dev"),
    #(L, "/usr/lib/libbmeipc.so.0",                                         "-lbmeipc", "libbmeipc0", "libbmeipc-dev"),
    #(L, "/usr/lib/libjpeg.so.62",                                          "-ljpeg", "libjpeg62", "libjpeg62-dev"),
    #(L, "/usr/lib/libQtXmlPatterns.so.4",                                  "-lQtXmlPatterns", "libqt4-xmlpatterns", "libqt4-dev"),
    #(L, "/usr/lib/libQtScript.so.4",                                       "-lQtScript", "libqt4-script", "libqt4-dev"),
    #(L, "/usr/lib/libssl.so.0.9.8",                                        "-lssl", "libssl0.9.8", "libssl-dev"),
    (L, "/usr/lib/libQtDeclarative.so.4",                                  "-lQtDeclarative", "libqt4-declarative", "libqt4-dev"),
    #(L, "/usr/lib/libxml2.so.2",                                           "-lxml2", "libxml2", "libxml2-dev"),
    #(L, "/lib/libcal.so.1",                                                "-lcal", "libcal1", "libcal-dev"),
    #(L, "/usr/lib/libcontentaction.so.0",                                  "-lcontentaction", "libcontentaction0", "libcontentaction-dev"),
    #(L, "/usr/lib/libcrypto.so.0.9.8",                                     "-lcrypto", "libssl0.9.8", "libssl-dev"),
    #(L, "/usr/lib/libbb5.so.0",                                            "-lbb5", "libbb5-0", "libbb5-dev"),
    (L, "/lib/libresolv.so.2",                                             "-lresolv", "libc6", "libc6"),
    #(L, "/usr/lib/libgio-2.0.so.0",                                        "-lgio-2.0", "libglib2.0-0", "libglib2.0-dev"),
    #(L, "/usr/lib/libQtSparql.so.0",                                       "-lQtSparql", "libqtsparql0", "libqtsparql-dev"),
    #(L, "/usr/lib/libsqlite3.so.0",                                        "-lsqlite3", "libsqlite3-0", "libsqlite3-dev"),
    #(L, "/usr/lib/libQtSql.so.4",                                          "-lQtSql", "libqt4-sql", "libqt4-dev"),
    #(L, "/usr/lib/libmeegotouchsettings.so.0",                             "-lmeegotouchsettings", "libmeegotouchsettings0", "libmeegotouch-dev"),
    #(L, "/usr/lib/libmeegotouchextensions.so.0",                           "-lmeegotouchextensions", "libmeegotouchextensions0", "libmeegotouch-dev"),
    (D, "/usr/lib/qt4/plugins/inputmethods/libminputcontext.so",           "-lminputcontext", "meego-im-context", "meego-im-context"),
    (D, "/usr/lib/qt4/plugins/inputmethods/libqimsw-multi.so",             "-lqimsw-multi", "libqtgui4", "libqtgui4"),
    #(L, "/lib/libudev.so.0",                                               "-ludev", "libudev0", "libudev-dev"),
    #(L, "/usr/lib/libmeegotouchviews.so.0",                                "-lmeegotouchviews", "libmeegotouchviews0", "libmeegotouch-dev"),
    (D, "/usr/lib/qt4/plugins/graphicssystems/libqmeegographicssystem.so", "/usr/lib/qt4/plugins/graphicssystems/libqmeegographicssystem.so", "libqt4-meegographicssystem", "libqt4-meegographicssystem"),
    (L, "/usr/lib/libXrandr.so.2",                                         "-lXrandr", "libxrandr2", "libxrandr-dev"),
    (D, "/usr/lib/contextkit/subscriber-plugins/kbslider-1.so",            "/usr/lib/contextkit/subscriber-plugins/kbslider-1.so", "context-keyboard", "context-keyboard"),
    (L, "/usr/lib/libXcomposite.so.1",                                     "-lXcomposite", "libxcomposite1", "libxcomposite-dev"),
    (D, "/usr/lib/libGLESv2_r125.so",                                      "-lGLESv2_r125", "libgles2-sgx-img", "libgles2-sgx-img"),
    #(L, "/usr/lib/libgconf-2.so.4",                                        "-lgconf-2", "libgconf2-6", "libgconf2-dev"),
    #(L, "/usr/lib/libQtCore.so.4",                                         "-lQtCore", "libqtcore4", "libqt4-dev"),
    #(L, "/usr/lib/libdbus-glib-1.so.2",                                    "-ldbus-glib-1", "libdbus-glib-1-2", "libdbus-glib-1-dev"),
    #(L, "/usr/lib/libstdc++.so.6",                                         "-lstdc++", "libstdc++6", "libstdc++6-4.4-dev"),
    #(L, "/usr/lib/libXi.so.6",                                             "-lXi", "libxi6", "libxi-dev"),
    #(L, "/lib/libpcre.so.3",                                               "-lpcre", "libpcre3", "libpcre3-dev"),
    #(L, "/usr/lib/libQtSvg.so.4",                                          "-lQtSvg", "libqt4-svg", "libqt4-dev"),
    #(L, "/usr/lib/libicui18n.so.44",                                       "-licui18n", "libicu44", "libicu-dev"),
    (D, "/usr/lib/libIMGegl_r125.so",                                      "-lIMGegl_r125", "opengles-sgx-img-common", "opengles-sgx-img-common"),
    #(L, "/usr/lib/libdbus-1.so.3",                                         "-ldbus-1", "libdbus-1-3", "libdbus-1-dev"),
    #(L, "/usr/lib/libmeegotouchcore.so.0",                                 "-lmeegotouchcore", "libmeegotouchcore0", "libmeegotouch-dev"),
    #(L, "/usr/lib/libX11.so.6",                                            "-lX11", "libx11-6", "libx11-dev"),
    #(L, "/usr/lib/libexpat.so.1",                                          "-lexpat", "libexpat1", "libexpat1-dev"),
    #(L, "/usr/lib/libfontconfig.so.1",                                     "-lfontconfig", "libfontconfig1", "libfontconfig1-dev"),
    #(L, "/usr/lib/libSM.so.6",                                             "-lSM", "libsm6", "libsm-dev"),
    #(L, "/usr/lib/libICE.so.6",                                            "-lICE", "libice6", "libice-dev"),
    (D, "/usr/lib/libEGL_r125.so",                                         "-lEGL_r125", "opengles-sgx-img-common", "opengles-sgx-img-common"),
    #(L, "/usr/lib/libXdamage.so.1",                                        "-lXdamage", "libxdamage1", "libxdamage-dev"),
    #(L, "/usr/lib/libcontextsubscriber.so.0",                              "-lcontextsubscriber", "libcontextsubscriber0", "libcontextsubscriber-dev"),
    (L, "/lib/libdl.so.2",                                                 "-ldl", "libc6", "libc6"),
    #(L, "/usr/lib/libXext.so.6",                                           "-lXext", "libxext6", "libxext-dev"),
    (L, "/lib/libpthread.so.0",                                            "-lpthread", "libc6", "libc6"),
    #(L, "/usr/lib/libXau.so.6",                                            "-lXau", "libxau6", "libxau-dev"),
    #(L, "/usr/lib/libgobject-2.0.so.0",                                    "-lgobject-2.0", "libglib2.0-0", "libglib2.0-dev"),
    #(L, "/usr/lib/libcdb.so.1",                                            "-lcdb", "libcdb1", "libcdb-dev"),
    #(L, "/usr/lib/libQtNetwork.so.4",                                      "-lQtNetwork", "libqt4-network", "libqt4-dev"),
    (D, "/usr/lib/gconv/UTF-16.so",                                        "/usr/lib/gconv/UTF-16.so", "libc6", "libc6"),
    #(L, "/usr/lib/libicudata.so.44",                                       "-licudata", "libicu44", "libicu-dev"),
    #(L, "/usr/lib/libQtMeeGoGraphicsSystemHelper.so.4",                    "-lQtMeeGoGraphicsSystemHelper", "libqt4-meegographicssystemhelper", "libqt4-meegographicssystemhelper-dev"),
    #(L, "/usr/lib/libQtGui.so.4",                                          "-lQtGui", "libqtgui4", "libqt4-dev"),
    #(L, "/usr/lib/libQtOpenGL.so.4",                                       "-lQtOpenGL", "libqt4-opengl", "libqt4-dev"),
    #(L, "/usr/lib/libQtXml.so.4",                                          "-lQtXml", "libqt4-xml", "libqt4-dev"),
    #(L, "/usr/lib/libQtDBus.so.4",                                         "-lQtDBus", "libqt4-dbus", "libqt4-dev"),
    #(L, "/usr/lib/libXfixes.so.3",                                         "-lXfixes", "libxfixes3", "libxfixes-dev"),
    #(L, "/usr/lib/libz.so.1",                                              "-lz", "zlib1g", "zlib1g-dev"),
    #(L, "/usr/lib/libxcb.so.1",                                            "-lxcb", "libxcb1", "libxcb1-dev"),
    (L, "/lib/libm.so.6",                                                  "-lm", "libc6", "libc6"),
    #(L, "/usr/lib/libgmodule-2.0.so.0",                                    "-lgmodule-2.0", "libglib2.0-0", "libglib2.0-dev"),
    (D, "/usr/lib/libsrv_um_r125.so",                                      "-lsrv_um_r125", "opengles-sgx-img-common", "opengles-sgx-img-common"),
    #(L, "/usr/lib/libpng12.so.0",                                          "-lpng12", "libpng12-0", "libpng12-dev"),
    #(L, "/usr/lib/libfreetype.so.6",                                       "-lfreetype", "libfreetype6", "libfreetype6-dev"),
    #(L, "/usr/lib/libgthread-2.0.so.0",                                    "-lgthread-2.0", "libglib2.0-0", "libglib2.0-dev"),
    #(L, "/lib/libglib-2.0.so.0",                                           "-lglib-2.0", "libglib2.0-0", "libglib2.0-dev"),
    #(L, "/usr/lib/libXrender.so.1",                                        "-lXrender", "libxrender1", "libxrender-dev"),
    #(L, "/usr/lib/libicuuc.so.44",                                         "-licuuc", "libicu44", "libicu-dev"),
    (L, "/lib/librt.so.1",                                                 "-lrt", "libc6", "libc6"),
    (D, "/usr/lib/libmlocale.so",                                          "-lmlocale", "libmlocale0", "libmlocale-dev"),
]

# Choose library list based on build environment
libraries = libraries_nokia
try:
    if "MeeGo" in file("/etc/meego-release").read() or "Mer" in file("/etc/mer-release").read():
        import libraries_meego
        libraries=libraries_meego.libraries
except: pass

import sys
import fileinput
import re
import string
import subprocess
from collections import defaultdict

pulled_in_by = defaultdict(list)

def find_roots(initial_libs):
    global pulled_in_by
    dep_reg = re.compile(r"^.*=>\s*(\S*)\s.*$")
    for lib in initial_libs:
        if pulled_in_by[lib] == []:

            # potential root, mark all its dependant libraries
            cout, cerr = subprocess.Popen(["ldd", lib], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
            for line in cout.split("\n"):
                m = dep_reg.match(line)
                if m:
                    l = m.group(1)
                    if l != "":
                        pulled_in_by[l].append(lib)

    # construct list of dependency tree roots
    roots = []
    for lib in initial_libs:
        if not pulled_in_by[lib]: 
            roots.append(lib)
    return roots

def build_deps_from_debian_control(control_file):
    res = set()
    dep_reg = re.compile(r"^Build-Depends:(.*)$")
    for line in fileinput.input(control_file):
        m = dep_reg.match(line)
        if m:
            for dep in m.group(1).split(","):
                res.add(dep.strip())
            fileinput.close()
            return res

def applauncherd_launcher_deps_from_debian_control(control_file):
    res = set()
    package_reg = re.compile(r"^Package: applauncherd-launcher$")
    dep_reg = re.compile(r"^Depends:(.*)$")

    line_match = False
    for line in fileinput.input(control_file):
        if not line_match:
            # skip until applauncherd-launcher package found
            line_match = package_reg.match(line)
        else:
            # then find the depends line
            m = dep_reg.match(line)
            if m:
                for dep in m.group(1).split(","):
                    res.add(dep.strip())
                fileinput.close()
                return res

if __name__ == "__main__":
    initial_libs = []
    linker_flags = {}
    bin_package = {}
    dev_package = {}
    status = {}

    # collect information from the library list
    for st, lib, lf, bp, dp in libraries:
        status[lib] = st
        initial_libs.append(lib)
        linker_flags[lib] = lf
        bin_package[lib] = bp
        dev_package[lib] = dp

    if len(sys.argv) == 2 and sys.argv[1] == "--preload-h-libraries":
        f = open("preload-h-libraries.h", "w")
        f.write("// List of libraries produced by library-helper.py. DO NOT EDIT\n")
        for lib in initial_libs:
            if status[lib] == D:
                f.write('"%s",\n' % lib)
        f.close()
    elif len(sys.argv) == 2 and sys.argv[1] == "--linker-flags":
        # produce minimized linker line
        f = open("additional-linked-libraries.ld", "w")
        for lib in initial_libs:
            if status[lib] == L:
                f.write('%s ' % linker_flags[lib])
        f.close()
    elif len(sys.argv) == 3 and sys.argv[1] == "--sanity-check":
        # roots of the library dependency tree
        roots = find_roots(initial_libs)

        for lib in initial_libs:
            if status[lib] == D and lib not in roots:
                print "Redundant library in dlopen list: %s" % lib
                print "   already pulled in as dependency of %s" % string.join(pulled_in_by[lib], ", ")
            elif status[lib] == L and lib not in roots:
                print "Redundant library in linked libraries: %s" % lib
                print "   already pulled in as dependency of %s" % string.join(pulled_in_by[lib], ", ")

        all_dev_packages = set()
        runtime_packages = set()
        linker_lib_packages = set()
        for status, lib, linkerflags, binpackage, devpackage in libraries:
            all_dev_packages.add(devpackage)
            if status == L and lib in roots:
                linker_lib_packages.add(devpackage)
                runtime_packages.add(binpackage)
        build_deps = build_deps_from_debian_control(sys.argv[2])
        runtime_deps = applauncherd_launcher_deps_from_debian_control(sys.argv[2])

        if not build_deps.issuperset(linker_lib_packages):
            print "Missing build dependencies: %s" % string.join(linker_lib_packages.difference(build_deps), ", ")
        if build_deps.difference(real_build_deps.union(linker_lib_packages)):
            print "Redundant build dependencies: %s" % string.join(build_deps.difference(real_build_deps.union(linker_lib_packages)), ", ")
        if not build_deps.issuperset(real_build_deps):
            print "Missing real build dependencies: %s" % string.join(real_build_deps.difference(build_deps), ", ")

        if not runtime_deps.issuperset(runtime_packages):
            print "Missing runtime dependencies: %s" % string.join(runtime_packages.difference(runtime_deps), ", ")
        if runtime_deps.difference(real_runtime_deps.union(runtime_packages)):
            print "Redundant runtime dependencies: %s" % string.join(runtime_deps.difference(real_runtime_deps.union(runtime_packages)), ", ")
        if not runtime_deps.issuperset(real_runtime_deps):
            print "Missing real runtime dependencies: %s" % string.join(real_runtime_deps.difference(runtime_deps), ", ")

    else:
        print """Usage: library-helper.py option
           Options: 
              --preload-h-libraries 
                   Produce a list of libraries in preload-h-libraries.h
              --linker-flags
                   Produdce a linker line fragment in additional-linked-libraries.ld
              --sanity-check control-file
                   Given the debian control file, check if there are missing packages
                   or redundant packages in Build-Depends or applauncherd-launcher Depends.
                   Also check the linked libraries and dlopened libraries for redundancies.
"""

        sys.exit(1)


# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/lingmo/project/LingmoOS/shell/bc/Notepad/addons/backtracebrowser/btbrowserwidget.ui'
#
# Created by: PyQt5 UI code generator 5.15.9
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_BtBrowserWidget(object):
    def setupUi(self, BtBrowserWidget):
        BtBrowserWidget.setObjectName("BtBrowserWidget")
        self.gridlayout = QtWidgets.QGridLayout(BtBrowserWidget)
        self.gridlayout.setContentsMargins(0, 0, 0, 0)
        self.gridlayout.setObjectName("gridlayout")
        self.lstBacktrace = QtWidgets.QTreeWidget(BtBrowserWidget)
        self.lstBacktrace.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.lstBacktrace.setRootIsDecorated(False)
        self.lstBacktrace.setItemsExpandable(False)
        self.lstBacktrace.setAllColumnsShowFocus(True)
        self.lstBacktrace.setColumnCount(4)
        self.lstBacktrace.setObjectName("lstBacktrace")
        self.gridlayout.addWidget(self.lstBacktrace, 0, 0, 1, 1)
        self.hboxlayout = QtWidgets.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")
        self.lblStatus = QtWidgets.QLabel(BtBrowserWidget)
        self.lblStatus.setText("")
        self.lblStatus.setObjectName("lblStatus")
        self.hboxlayout.addWidget(self.lblStatus)
        spacerItem = QtWidgets.QSpacerItem(31, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.hboxlayout.addItem(spacerItem)
        self.btnConfigure = QtWidgets.QPushButton(BtBrowserWidget)
        self.btnConfigure.setObjectName("btnConfigure")
        self.hboxlayout.addWidget(self.btnConfigure)
        self.btnClipboard = QtWidgets.QPushButton(BtBrowserWidget)
        self.btnClipboard.setObjectName("btnClipboard")
        self.hboxlayout.addWidget(self.btnClipboard)
        self.btnBacktrace = QtWidgets.QPushButton(BtBrowserWidget)
        self.btnBacktrace.setObjectName("btnBacktrace")
        self.hboxlayout.addWidget(self.btnBacktrace)
        self.gridlayout.addLayout(self.hboxlayout, 1, 0, 1, 1)

        self.retranslateUi(BtBrowserWidget)
        QtCore.QMetaObject.connectSlotsByName(BtBrowserWidget)

    def retranslateUi(self, BtBrowserWidget):
        _translate = QtCore.QCoreApplication.translate
        self.lstBacktrace.headerItem().setText(0, _translate("BtBrowserWidget", "#"))
        self.lstBacktrace.headerItem().setText(1, _translate("BtBrowserWidget", "File"))
        self.lstBacktrace.headerItem().setText(2, _translate("BtBrowserWidget", "Line"))
        self.lstBacktrace.headerItem().setText(3, _translate("BtBrowserWidget", "Function"))
        self.btnConfigure.setText(_translate("BtBrowserWidget", "Configure Paths..."))
        self.btnClipboard.setText(_translate("BtBrowserWidget", "Use Clipboard"))
        self.btnBacktrace.setText(_translate("BtBrowserWidget", "Load File..."))

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/lingmo/project/nx_pkg/LingmoOS/utils/kate/addons/gdbplugin/debugconfig.ui'
#
# Created by: PyQt5 UI code generator 5.15.11
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_DebugConfigWidget(object):
    def setupUi(self, DebugConfigWidget):
        DebugConfigWidget.setObjectName("DebugConfigWidget")
        DebugConfigWidget.resize(671, 770)
        self.verticalLayout_3 = QtWidgets.QVBoxLayout(DebugConfigWidget)
        self.verticalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.tabWidget = QtWidgets.QTabWidget(DebugConfigWidget)
        self.tabWidget.setObjectName("tabWidget")
        self.tab_1 = QtWidgets.QWidget()
        self.tab_1.setObjectName("tab_1")
        self.verticalLayout_4 = QtWidgets.QVBoxLayout(self.tab_1)
        self.verticalLayout_4.setObjectName("verticalLayout_4")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.label = QtWidgets.QLabel(self.tab_1)
        self.label.setObjectName("label")
        self.horizontalLayout_2.addWidget(self.label)
        self.edtConfigPath = KUrlRequester(self.tab_1)
        self.edtConfigPath.setText("")
        self.edtConfigPath.setObjectName("edtConfigPath")
        self.horizontalLayout_2.addWidget(self.edtConfigPath)
        self.verticalLayout_4.addLayout(self.horizontalLayout_2)
        self.userConfig = QtWidgets.QPlainTextEdit(self.tab_1)
        self.userConfig.setObjectName("userConfig")
        self.verticalLayout_4.addWidget(self.userConfig)
        self.userConfigError = QtWidgets.QLabel(self.tab_1)
        self.userConfigError.setText("")
        self.userConfigError.setObjectName("userConfigError")
        self.verticalLayout_4.addWidget(self.userConfigError)
        self.tabWidget.addTab(self.tab_1, "")
        self.tab_2 = QtWidgets.QWidget()
        self.tab_2.setObjectName("tab_2")
        self.verticalLayout_9 = QtWidgets.QVBoxLayout(self.tab_2)
        self.verticalLayout_9.setObjectName("verticalLayout_9")
        self.defaultConfig = QtWidgets.QPlainTextEdit(self.tab_2)
        self.defaultConfig.setReadOnly(True)
        self.defaultConfig.setObjectName("defaultConfig")
        self.verticalLayout_9.addWidget(self.defaultConfig)
        self.tabWidget.addTab(self.tab_2, "")
        self.verticalLayout_3.addWidget(self.tabWidget)

        self.retranslateUi(DebugConfigWidget)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(DebugConfigWidget)

    def retranslateUi(self, DebugConfigWidget):
        _translate = QtCore.QCoreApplication.translate
        self.label.setText(_translate("DebugConfigWidget", "Settings File:"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_1), _translate("DebugConfigWidget", "User Debug Adapter Settings"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), _translate("DebugConfigWidget", "Default Debug Adapter Settings"))
from kurlrequester import KUrlRequester

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/lingmo/project/LingmoOS/shell/bc/Notepad/addons/kate-ctags/kate_ctags.ui'
#
# Created by: PyQt5 UI code generator 5.15.9
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_kateCtags(object):
    def setupUi(self, kateCtags):
        kateCtags.setObjectName("kateCtags")
        kateCtags.resize(391, 291)
        self.verticalLayout = QtWidgets.QVBoxLayout(kateCtags)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.tabWidget = QtWidgets.QTabWidget(kateCtags)
        self.tabWidget.setObjectName("tabWidget")
        self.widget = QtWidgets.QWidget()
        self.widget.setObjectName("widget")
        self.gridLayout = QtWidgets.QGridLayout(self.widget)
        self.gridLayout.setContentsMargins(0, 0, 0, 0)
        self.gridLayout.setObjectName("gridLayout")
        self.inputEdit = QtWidgets.QLineEdit(self.widget)
        self.inputEdit.setProperty("showClearButton", True)
        self.inputEdit.setObjectName("inputEdit")
        self.gridLayout.addWidget(self.inputEdit, 0, 0, 1, 1)
        self.updateButton = QtWidgets.QPushButton(self.widget)
        self.updateButton.setObjectName("updateButton")
        self.gridLayout.addWidget(self.updateButton, 0, 1, 1, 1)
        self.tagTreeWidget = QtWidgets.QTreeWidget(self.widget)
        self.tagTreeWidget.setRootIsDecorated(False)
        self.tagTreeWidget.setObjectName("tagTreeWidget")
        self.gridLayout.addWidget(self.tagTreeWidget, 1, 0, 1, 2)
        self.tabWidget.addTab(self.widget, "")
        self.targets = QtWidgets.QWidget()
        self.targets.setObjectName("targets")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.targets)
        self.gridLayout_2.setContentsMargins(0, 0, 0, 0)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.targetList = QtWidgets.QListWidget(self.targets)
        self.targetList.setObjectName("targetList")
        self.gridLayout_2.addWidget(self.targetList, 0, 0, 4, 2)
        self.addButton = QtWidgets.QPushButton(self.targets)
        self.addButton.setObjectName("addButton")
        self.gridLayout_2.addWidget(self.addButton, 0, 2, 1, 1)
        self.delButton = QtWidgets.QPushButton(self.targets)
        self.delButton.setObjectName("delButton")
        self.gridLayout_2.addWidget(self.delButton, 1, 2, 1, 1)
        spacerItem = QtWidgets.QSpacerItem(13, 16, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_2.addItem(spacerItem, 3, 2, 1, 1)
        self.updateButton2 = QtWidgets.QPushButton(self.targets)
        self.updateButton2.setObjectName("updateButton2")
        self.gridLayout_2.addWidget(self.updateButton2, 2, 2, 1, 1)
        self.gridLayout_2.setColumnStretch(0, 1)
        self.tabWidget.addTab(self.targets, "")
        self.database = QtWidgets.QWidget()
        self.database.setObjectName("database")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.database)
        self.gridLayout_3.setContentsMargins(-1, 0, 0, 0)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.fileLabel = QtWidgets.QLabel(self.database)
        self.fileLabel.setLayoutDirection(QtCore.Qt.RightToLeft)
        self.fileLabel.setObjectName("fileLabel")
        self.gridLayout_3.addWidget(self.fileLabel, 0, 0, 1, 1)
        self.cmdLabel = QtWidgets.QLabel(self.database)
        self.cmdLabel.setLayoutDirection(QtCore.Qt.RightToLeft)
        self.cmdLabel.setObjectName("cmdLabel")
        self.gridLayout_3.addWidget(self.cmdLabel, 3, 0, 1, 1)
        self.cmdEdit = QtWidgets.QLineEdit(self.database)
        self.cmdEdit.setFrame(True)
        self.cmdEdit.setProperty("showClearButton", True)
        self.cmdEdit.setObjectName("cmdEdit")
        self.gridLayout_3.addWidget(self.cmdEdit, 3, 1, 1, 1)
        spacerItem1 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_3.addItem(spacerItem1, 4, 0, 1, 1)
        self.tagsFile = KUrlRequester(self.database)
        self.tagsFile.setObjectName("tagsFile")
        self.gridLayout_3.addWidget(self.tagsFile, 0, 1, 1, 2)
        self.resetCMD = QtWidgets.QToolButton(self.database)
        self.resetCMD.setAutoRaise(True)
        self.resetCMD.setObjectName("resetCMD")
        self.gridLayout_3.addWidget(self.resetCMD, 3, 2, 1, 1)
        self.tabWidget.addTab(self.database, "")
        self.verticalLayout.addWidget(self.tabWidget)

        self.retranslateUi(kateCtags)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(kateCtags)

    def retranslateUi(self, kateCtags):
        _translate = QtCore.QCoreApplication.translate
        self.updateButton.setText(_translate("kateCtags", "Update Index"))
        self.tagTreeWidget.headerItem().setText(0, _translate("kateCtags", "Tag"))
        self.tagTreeWidget.headerItem().setText(1, _translate("kateCtags", "Type"))
        self.tagTreeWidget.headerItem().setText(2, _translate("kateCtags", "File"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.widget), _translate("kateCtags", "Lookup"))
        self.addButton.setText(_translate("kateCtags", "Add"))
        self.delButton.setText(_translate("kateCtags", "Remove"))
        self.updateButton2.setText(_translate("kateCtags", "Update Index"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.targets), _translate("kateCtags", "Index Targets"))
        self.fileLabel.setText(_translate("kateCtags", "CTags database file"))
        self.cmdLabel.setText(_translate("kateCtags", "CTags command"))
        self.resetCMD.setToolTip(_translate("kateCtags", "Revert to the default command"))
        self.resetCMD.setText(_translate("kateCtags", "..."))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.database), _translate("kateCtags", "Database"))
from kurlrequester import KUrlRequester

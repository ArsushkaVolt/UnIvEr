# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'part1_design.ui'
##
## Created by: Qt User Interface Compiler version 6.11.2
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QAction, QBrush, QColor, QConicalGradient,
    QCursor, QFont, QFontDatabase, QGradient,
    QIcon, QImage, QKeySequence, QLinearGradient,
    QPainter, QPalette, QPixmap, QRadialGradient,
    QTransform)
from PySide6.QtWidgets import (QApplication, QGridLayout, QHBoxLayout, QLCDNumber,
    QMainWindow, QMenu, QMenuBar, QPushButton,
    QSizePolicy, QStatusBar, QVBoxLayout, QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(359, 402)
        self.actionNewGame = QAction(MainWindow)
        self.actionNewGame.setObjectName(u"actionNewGame")
        self.actionExit = QAction(MainWindow)
        self.actionExit.setObjectName(u"actionExit")
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.verticalLayout = QVBoxLayout(self.centralwidget)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.lcdMines = QLCDNumber(self.centralwidget)
        self.lcdMines.setObjectName(u"lcdMines")

        self.horizontalLayout.addWidget(self.lcdMines)

        self.btnReset = QPushButton(self.centralwidget)
        self.btnReset.setObjectName(u"btnReset")

        self.horizontalLayout.addWidget(self.btnReset)

        self.lcdTimer = QLCDNumber(self.centralwidget)
        self.lcdTimer.setObjectName(u"lcdTimer")

        self.horizontalLayout.addWidget(self.lcdTimer)


        self.verticalLayout.addLayout(self.horizontalLayout)

        self.gridContainer = QWidget(self.centralwidget)
        self.gridContainer.setObjectName(u"gridContainer")
        self.gridLayout = QGridLayout(self.gridContainer)
        self.gridLayout.setObjectName(u"gridLayout")

        self.verticalLayout.addWidget(self.gridContainer)

        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(MainWindow)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 359, 21))
        self.menu = QMenu(self.menubar)
        self.menu.setObjectName(u"menu")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(MainWindow)
        self.statusbar.setObjectName(u"statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.menubar.addAction(self.menu.menuAction())
        self.menu.addSeparator()
        self.menu.addAction(self.actionNewGame)
        self.menu.addAction(self.actionExit)

        self.retranslateUi(MainWindow)

        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"MainWindow", None))
        self.actionNewGame.setText(QCoreApplication.translate("MainWindow", u"\u041d\u043e\u0432\u0430\u044f \u0438\u0433\u0440\u0430", None))
        self.actionExit.setText(QCoreApplication.translate("MainWindow", u"\u0412\u044b\u0445\u043e\u0434", None))
        self.btnReset.setText(QCoreApplication.translate("MainWindow", u"\U0001f642", None))
        self.menu.setTitle(QCoreApplication.translate("MainWindow", u"\u0421\u0430\u043f\u0451\u0440", None))
    # retranslateUi


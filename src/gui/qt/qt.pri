SOURCES += \
    $$PWD/mainwindow.cpp \
	$$PWD/terminalwidget.cpp \
	$$PWD/palette.cpp \

HEADERS  += \
	$$PWD/mainwindow.h \
	$$PWD/terminalwidget.h \
	$$PWD/palette.h \

FORMS += \
	$$PWD/mainwindow.ui \

blackberry {
	HEADERS += \
		$$PWD/bbvirtualkeyboardhandler.h \
		$$PWD/bbvirtualkeyboardwidget.h \

	SOURCES += \
		$$PWD/bbvirtualkeyboardhandler.cpp \
		$$PWD/bbvirtualkeyboardwidget.cpp \
}

RESOURCES += \
    $$PWD/qui-qt.qrc

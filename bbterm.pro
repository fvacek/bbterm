#-------------------------------------------------
#
# Project created by QtCreator 2014-04-23T13:46:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}
else {
	DEFINES += Q_DECL_OVERRIDE=
}

TARGET = bbterm
TEMPLATE = app

!qnx {
LIBS += \
  -lutil \
}

include(src/src.pri)


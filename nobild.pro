TEMPLATE	= app
CONFIG		+= qt release
QT		+= core network

greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
}

HEADERS		+= nobild.h
SOURCES		+= nobild.cpp

TARGET		= nobild

isEmpty(PREFIX) {
    PREFIX=/usr/local
}

target.path	= $${PREFIX}/sbin
INSTALLS	+= target

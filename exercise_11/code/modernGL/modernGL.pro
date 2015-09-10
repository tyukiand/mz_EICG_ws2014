TEMPLATE = app
TARGET = modernGL
QT += core gui opengl
CONFIG += console #debug
CONFIG += c++11

macx: CONFIG += c++11
macx: QMAKE_MAC_SDK = macosx10.9 

unix:QMAKE_CXXFLAGS += -std=c++0x
macx:QMAKE_CXXFLAGS += -stdlib=libc++

RESOURCES += shaders.qrc
HEADERS += *.h

SOURCES += *.cpp


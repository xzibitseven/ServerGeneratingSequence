TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -pthread -latomic

SOURCES += \
        main.cpp \
    server.cpp \
    sequencecommand.cpp \
    command.cpp \
    exportseqcommand.cpp

HEADERS += \
    server.hpp \
    sequencecommand.hpp \
    command.hpp \
    exportseqcommand.hpp \
    common.hpp \
    storage.hpp

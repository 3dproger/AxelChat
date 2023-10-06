QT += core websockets network gui

CONFIG += c++11
TEMPLATE = lib
DEFINES += AXELCHAT_LIBRARY

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
HEADERS += \
    chat_services/apikeys.h \
    chat_services/chatservice.h \
    chat_services/chatservicestypes.h \
    chat_services/trovo.h \
    chat_services/vkplaylive.h \
    chat_services/goodgame.h \
    chat_services/twitch.h \
    chat_services/youtube.h \
    models/author.h \
    models/message.h \
    models/messagesmodel.h \
    applicationinfo.h \
    axelchatlib_api.h \
    ChatManager.h \
    utils.h

SOURCES += \
    chat_services/trovo.cpp \
    chat_services/vkplaylive.cpp \
    chat_services/goodgame.cpp \
    chat_services/twitch.cpp \
    chat_services/youtube.cpp \
    models/author.cpp \
    models/message.cpp \
    models/messagesmodel.cpp \
    ChatManager.cpp \
    main.cpp

RESOURCES +=

TRANSLATIONS +=

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

win32: {
    #Подключаем SSL для Windows. Соответствующий модуль должен быть установлён!!!

    OPENSSL_LIBS='-L/opt/ssl/lib -lssl -lcrypto' ./configure -openssl-linked

    contains(QT_ARCH, i386) {
        #Для Windows x32
        INCLUDEPATH += $$(QTDIR)/../../Tools/OpenSSL/Win_x86/include
    } else {
        #Для Windows x64
        INCLUDEPATH += $$(QTDIR)/../../Tools/OpenSSL/Win_x64/include
    }


    #Сборка файлов релизной версии

    CONFIG(debug, debug|release) {
        #debug
    } else {
        #release
        contains(QT_ARCH, i386) {
            #Для Windows x32
            DESTDIR = $$_PRO_FILE_PWD_/../release_win32
        } else {
            #Для Windows x64
            DESTDIR = $$_PRO_FILE_PWD_/../release_win64
        }

        QMAKE_POST_LINK += $$(QTDIR)/bin/windeployqt --release --qmldir $$(QTDIR)/qml $$DESTDIR $$escape_expand(\\n\\t)
    }
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix: target.path = /usr/lib
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

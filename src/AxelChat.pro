QT += widgets gui quick multimedia websockets network svg

CONFIG += c++17

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
    chat_services/chatservice.hpp \
    applicationinfo.hpp \
    authorqmlprovider.h \
    botaction.hpp \
    chatbot.hpp \
    chathandler.hpp \
    clipboardqml.hpp \
    commandseditor.h \
    commandsingleeditor.h \
    models/chatauthor.h \
    models/chatmessage.h \
    models/chatmessagesmodle.hpp \
    githubapi.hpp \
    chat_services/goodgame.h \
    i18n.hpp \
    outputtofile.hpp \
    qmlutils.hpp \
    setting.h \
    tray.h \
    chat_services/twitch.hpp \
    utils.hpp \
    chat_services/youtube.hpp

SOURCES += \
        authorqmlprovider.cpp \
        botaction.cpp \
        chatbot.cpp \
        chathandler.cpp \
        clipboardqml.cpp \
        commandseditor.cpp \
        commandsingleeditor.cpp \
        models/chatauthor.cpp \
        models/chatmessage.cpp \
        models/chatmessagesmodle.cpp \
        githubapi.cpp \
        chat_services/goodgame.cpp \
        i18n.cpp \
        main.cpp \
        outputtofile.cpp \
        qmlutils.cpp \
        tray.cpp \
        chat_services/twitch.cpp \
        chat_services/youtube.cpp

RESOURCES += qml.qrc \
    resources.qrc

contains(QT_ARCH, i386)|contains(QT_ARCH, x86_64) {
    RESOURCES += builtin_commands.qrc
}

TRANSLATIONS += \
    Translation_ru_RU.ts

CONFIG += lrelease
CONFIG += embed_translations

RC_FILE = rc.rc

FORMS += \
    commandseditor.ui \
    commandsingleeditor.ui

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

ANDROID_EXTRA_LIBS += \
    $$_PRO_FILE_PWD_/../app_files/arm/libcrypto_1_1.so \
    $$_PRO_FILE_PWD_/../app_files/arm/libssl_1_1.so

win32: {
    #Подключаем SSL для Windows. Соответствующий модуль должен быть установлён!!!

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

        #QMAKE_POST_LINK += $$(QTDIR)/bin/windeployqt --release --qmldir $$(QTDIR)/qml $$DESTDIR $$escape_expand(\\n\\t) # In Qt 5.15 with --release not working
        QMAKE_POST_LINK += $$(QTDIR)/bin/windeployqt --qmldir $$(QTDIR)/qml $$DESTDIR $$escape_expand(\\n\\t)
    }
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

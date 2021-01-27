QT += widgets gui quick multimedia

CONFIG += c++11
CONFIG += qtquickcompiler

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
    applicationinfo.hpp \
    botaction.hpp \
    cef.hpp \
    chatbot.hpp \
    chathandler.hpp \
    chatmessage.hpp \
    clipboardqml.hpp \
    commandseditor.h \
    commandsingleeditor.h \
    githubapi.hpp \
    i18n.hpp \
    outputtofile.hpp \
    qmlutils.hpp \
    settingspageabout.h \
    settingspagecommands.h \
    settingspagecommon.h \
    settingspageoutputtofile.h \
    settingspageyoutube.h \
    settingswindow.h \
    types.hpp \
    youtube.hpp

SOURCES += \
        botaction.cpp \
        cef.cpp \
        chatbot.cpp \
        chathandler.cpp \
        chatmessage.cpp \
        clipboardqml.cpp \
        commandseditor.cpp \
        commandsingleeditor.cpp \
        githubapi.cpp \
        i18n.cpp \
        main.cpp \
        outputtofile.cpp \
        qmlutils.cpp \
        settingspageabout.cpp \
        settingspagecommands.cpp \
        settingspagecommon.cpp \
        settingspageoutputtofile.cpp \
        settingspageyoutube.cpp \
        settingswindow.cpp \
        youtube.cpp

RESOURCES += qml.qrc \
    my_components/InteractiveWait/InteractiveWait.qrc \
    resources.qrc \
    translations.qrc

TRANSLATIONS += \
    ru_RU.ts

RC_FILE = icon.rc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

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

        QMAKE_POST_LINK += $$(QTDIR)/bin/windeployqt --release --qmldir $$(QTDIR)/qml $$DESTDIR $$escape_expand(\\n\\t)
    }
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# CEF
win32: LIBS += -L$$PWD/../cef/Release/ -llibcef

INCLUDEPATH += $$PWD/../cef
DEPENDPATH += $$PWD/../cef

win32: LIBS += -L$$PWD/../cef/libcef_dll_wrapper/Release/ -llibcef_dll_wrapper

QMAKE_CXXFLAGS_RELEASE += /MT
QMAKE_CXXFLAGS_DEBUG += /MTd

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../cef/libcef_dll_wrapper/Release/libcef_dll_wrapper.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../cef/libcef_dll_wrapper/Release/libcef_dll_wrapper.a

FORMS += \
    commandseditor.ui \
    commandsingleeditor.ui \
    settingspageabout.ui \
    settingspagecommands.ui \
    settingspagecommon.ui \
    settingspageoutputtofile.ui \
    settingspageyoutube.ui \
    settingswindow.ui

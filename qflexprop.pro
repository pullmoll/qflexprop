QT      += core gui widgets serialport
CONFIG  += c++11
VERSION_MAJOR = 0
VERSION_MINOR = 1
VERSION_PATCH = 0
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}

DEFINES += VERSION_MAJOR=$${VERSION_MAJOR}
DEFINES += VERSION_MINOR=$${VERSION_MINOR}
DEFINES += VERSION_PATCH=$${VERSION_PATCH}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/propconst.cpp \
    $$PWD/idstrings.cpp \
    $$PWD/propload.cpp \
    $$PWD/serterm.cpp \
    $$PWD/qflexprop.cpp \
    $$PWD/util.cpp \
    $$PWD/widgets/propedit.cpp \
    $$PWD/dialogs/flexspindlg.cpp \
    $$PWD/dialogs/serialportdlg.cpp \
    $$PWD/term/vt220.cpp \
    $$PWD/term/vtattr.cpp \
    $$PWD/term/vtglyph.cpp \
    $$PWD/term/vtglyphidx.cpp \
    $$PWD/term/vtglyphs.cpp \
    $$PWD/term/vtline.cpp \
    $$PWD/term/vtscrollarea.cpp \
    dialogs/aboutdlg.cpp \
    dialogs/settingsdlg.cpp \
    dialogs/textbrowserdlg.cpp \
    loadelf.cpp

HEADERS += \
    $$PWD/propconst.h \
    $$PWD/idstrings.h \
    $$PWD/serterm.h \
    $$PWD/qflexprop.h \
    $$PWD/propload.h \
    $$PWD/proptypes.h \
    $$PWD/util.h \
    $$PWD/widgets/propedit.h \
    $$PWD/dialogs/flexspindlg.h \
    $$PWD/dialogs/serialportdlg.h \
    $$PWD/term/vt220.h \
    $$PWD/term/vtattr.h \
    $$PWD/term/vtchar.h \
    $$PWD/term/vtglyph.h \
    $$PWD/term/vtglyphidx.h \
    $$PWD/term/vtglyphs.h \
    $$PWD/term/vtline.h \
    $$PWD/term/vtscrollarea.h \
    dialogs/aboutdlg.h \
    dialogs/settingsdlg.h \
    dialogs/textbrowserdlg.h \
    loadelf.h

FORMS += \
    $$PWD/qflexprop.ui \
    $$PWD/dialogs/flexspindlg.ui \
    $$PWD/dialogs/serialportdlg.ui \
    $$PWD/serterm.ui \
    dialogs/aboutdlg.ui \
    dialogs/settingsdlg.ui \
    dialogs/textbrowserdlg.ui

TRANSLATIONS += \
    $$PWD/qflexprop_de_DE.ts

INCLUDEPATH += \
    $$PWD/term \
    $$PWD/dialogs \
    $$PWD/widgets

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qflexprop.qrc

QT += websockets
QT += core
QT += network
QT += widgets
QT += printsupport
QT += charts
QT += gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += -fpermissive
LIBS += -lpthread

#Boost includepath
INCLUDEPATH += ../boost_1_76_0/
#Dicomlib includepath
INCLUDEPATH += 3rd-party/dicomlib/include \
#Eigen includepath
INCLUDEPATH += /home/ivni/Documents/Imp/eigen/eigen-3.3.9/
#Artemis includepath
INCLUDEPATH += 3rd-party/artemis/include \
               3rd-party/artemis/include/dicom \
               3rd-party/artemis/include/jpg \
               3rd-party/artemis/include/bmp \
               3rd-party/artemis/include/png \
               3rd-party/artemis/include/krl

#Higiia includepath
INCLUDEPATH += 3rd-party/hermes/util/include/
INCLUDEPATH += controller/include/
INCLUDEPATH += gui/include/
INCLUDEPATH += gui/ui/
INCLUDEPATH += util/include/


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#Higiia sources
SOURCES += \
    util/src/Util.cpp \
    util/src/WindowingInterval.cpp \
    main.cpp \
    gui/src/FormDiagnosis.cpp \
    gui/src/Login.cpp \
    gui/src/UnreportedStudies.cpp \
    gui/src/QueryParameters.cpp \
    gui/src/OberonViewer.cpp \
    gui/src/QPushButtonAlter.cpp \
    gui/src/Analytics.cpp \
    controller/src/ScopeCaption.cpp \
    controller/src/ResultTable.cpp \
    controller/src/MedicalImageTable.cpp \
    controller/src/SirenSqlQuery.cpp

#Higiia headers
HEADERS += \
    util/include/Util.h \
    util/include/WindowingInterval.h \    
    gui/include/FormDiagnosis.h \
    gui/include/Login.h \
    gui/include/UnreportedStudies.h \
    gui/include/QueryParameters.h \
    gui/include/OberonViewer.h \
    gui/include/QPushButtonAlter.h \
    gui/include/Analytics.h \
    controller/include/ScopeCaption.h \
    controller/include/ResultTable.h \
    controller/include/MedicalImageTable.h \
    controller/include/SirenSqlQuery.h

#Higiia forms
FORMS += \
    gui/ui/FormDiagnosis.ui \
    gui/ui/Login.ui \
    gui/ui/UnreportedStudies.ui \
    gui/ui/QueryParameters.ui \
    gui/ui/OberonViewer.ui \
    gui/ui/Analytics.ui

#Higiia resources
RESOURCES += icons.qrc

#Hermes headers
HEADERS += 3rd-party/hermes/util/include/BasicArrayObject.h

#Artemis headers
HEADERS += 3rd-party/artemis/include/Pixel.h \
           3rd-party/artemis/include/ImageBase.h \
           3rd-party/artemis/include/dicom/DcmLib.h \
           3rd-party/artemis/include/jpg/JpgLib.h \
           3rd-party/artemis/include/bmp/BmpLib.h \
           3rd-party/artemis/include/png/PngLib.h \
           3rd-party/artemis/include/krl/KrlFileHandler.h \
           3rd-party/artemis/include/krl/KrlLib.h

#Artemis sources
SOURCES += 3rd-party/artemis/src/Pixel.cpp \
           3rd-party/artemis/src/ImageBase.cpp \
           3rd-party/artemis/src/dicom/DcmLib.cpp \
           3rd-party/artemis/src/jpg/JpgLib.cpp \
           3rd-party/artemis/src/bmp/BmpLib.cpp \
           3rd-party/artemis/src/png/PngLib.cpp \
           3rd-party/artemis/src/krl/KrlFileHandler.cpp \
           3rd-party/artemis/src/krl/KrlLib.cpp

#Dicomlib headers
SOURCES += \
        3rd-party/dicomlib/src/File.cpp \
        3rd-party/dicomlib/src/UID.cpp \
        3rd-party/dicomlib/src/Exceptions.cpp \
        3rd-party/dicomlib/src/FileMetaInformation.cpp \
        3rd-party/dicomlib/src/TransferSyntax.cpp \
        3rd-party/dicomlib/src/Decoder.cpp \
        3rd-party/dicomlib/src/Encoder.cpp \
        3rd-party/dicomlib/src/Buffer.cpp \
        3rd-party/dicomlib/src/Utility.cpp \
        3rd-party/dicomlib/src/GroupLength.cpp \
        3rd-party/dicomlib/src/DataDictionary.cpp

#Dicomlib sources
HEADERS += \
        3rd-party/dicomlib/include/dicomlib.hpp \
        3rd-party/dicomlib/include/File.hpp \
        3rd-party/dicomlib/include/UIDs.hpp \
        3rd-party/dicomlib/include/UID.hpp \
        3rd-party/dicomlib/include/Utility.hpp \
        3rd-party/dicomlib/include/Dumper.hpp \
        3rd-party/dicomlib/include/Exceptions.hpp \
        3rd-party/dicomlib/include/FileMetaInformation.hpp \
        3rd-party/dicomlib/include/TransferSyntax.hpp \
        3rd-party/dicomlib/include/Decoder.hpp \
        3rd-party/dicomlib/include/Encoder.hpp \
        3rd-party/dicomlib/include/Buffer.hpp \
        3rd-party/dicomlib/include/Utility.hpp \
        3rd-party/dicomlib/include/GroupLength.hpp \
        3rd-party/dicomlib/include/DataDictionary.hpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



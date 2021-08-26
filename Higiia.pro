QT += websockets
QT += core
QT += network
QT += widgets
QT += printsupport
QT += charts
QT += gui
QT += widgets

CONFIG += c++11
CONFIG += -fpermissive
LIBS += -lpthread

#Boost includepath
INCLUDEPATH += ../<boost>
#Dicomlib includepath
INCLUDEPATH += 3rd-party/<dicomlib>
#Eigen includepath
INCLUDEPATH += 3rd-party/<eigen>
#Artemis includepath
INCLUDEPATH += 3rd-party/artemis/include \
               3rd-party/artemis/include/dicom \
               3rd-party/artemis/include/jpg
#Higiia includepath
INCLUDEPATH += 3rd-party/hermes/include/util/

INCLUDEPATH += controller/include/
INCLUDEPATH += gui/include
INCLUDEPATH += gui/ui


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
    Util.cpp \
    WindowingInterval.cpp \
    main.cpp \
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
    Util.h \
    WindowingInterval.h \
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
    gui/ui/Login.ui \
    gui/ui/UnreportedStudies.ui \
    gui/ui/QueryParameters.ui \
    gui/ui/OberonViewer.ui \
    gui/ui/Analytics.ui

#Higiia resources
RESOURCES += icons.qrc

#Hermes headers
HEADERS += 3rd-party/hermes/include/util/BasicArrayObject.h

#Artemis headers
HEADERS += 3rd-party/artemis/include/Pixel.h \
           3rd-party/artemis/include/ImageBase.h \
           3rd-party/artemis/include/dicom/DcmLib.h \
           3rd-party/artemis/include/jpg/JpgLib.h

#Artemis sources
SOURCES += 3rd-party/artemis/src/Pixel.cpp \
           3rd-party/artemis/src/ImageBase.cpp \
           3rd-party/artemis/src/dicom/DcmLib.cpp \
           3rd-party/artemis/src/jpg/JpgLib.cpp

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




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
INCLUDEPATH += ../../boost_1_76_0
#Dicomlib includepath
INCLUDEPATH += ../../higiia/3rd-party/dicomlib/include
INCLUDEPATH += ../../higiia/3rd-party/dicomlib
#Eigen includepath
INCLUDEPATH += ../../eigen/eigen-3.3.9

#Artemis includepath
INCLUDEPATH += ../../higiia/3rd-party/artemis/include \
               ../../higiia/3rd-party/artemis/include/dicom \
               ../../higiia/3rd-party/artemis/include/jpg \
               ../../higiia/3rd-party/artemis/include/bmp \
               ../../higiia/3rd-party/artemis/include/png \
               ../../higiia/3rd-party/artemis/include/krl

#Higiia includepath

INCLUDEPATH += ../../higiia/3rd-party/hermes/include/
INCLUDEPATH += ../../higiia/3rd-party/hermes/util/include/

INCLUDEPATH += controller/include/
INCLUDEPATH += gui/include
INCLUDEPATH += gui/ui
INCLUDEPATH += util/include


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
    gui/src/Login.cpp \
    gui/src/UnreportedStudies.cpp \
    gui/src/QueryParameters.cpp \
    gui/src/OberonViewer.cpp \
    gui/src/QPushButtonAlter.cpp \
    gui/src/Analytics.cpp \
    gui/src/FormDiagnosis.cpp \
    controller/src/ScopeCaption.cpp \
    controller/src/ResultTable.cpp \
    controller/src/MedicalImageTable.cpp \
    controller/src/SirenSqlQuery.cpp

#Higiia headers
HEADERS += \    
    util/include/Util.h \
    util/include/WindowingInterval.h \
    gui/include/Login.h \
    gui/include/UnreportedStudies.h \
    gui/include/QueryParameters.h \
    gui/include/OberonViewer.h \
    gui/include/QPushButtonAlter.h \
    gui/include/Analytics.h \
    gui/include/FormDiagnosis.h \
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
    gui/ui/Analytics.ui \
    gui/ui/FormDiagnosis.ui

#Higiia resources
RESOURCES += icons.qrc

#Hermes headers
HEADERS += ../../higiia/3rd-party/hermes/util/include/BasicArrayObject.h

#Artemis headers
HEADERS += ../../higiia/3rd-party/artemis/include/Pixel.h \
           ../../higiia/3rd-party/artemis/include/ImageBase.h \
           ../../higiia/3rd-party/artemis/include/dicom/DcmLib.h \
           ../../higiia/3rd-party/artemis/include/jpg/JpgLib.h \
           ../../higiia/3rd-party/artemis/include/bmp/BmpLib.h \
           ../../higiia/3rd-party/artemis/include/png/PngLib.h \
           ../../higiia/3rd-party/artemis/include/krl/KrlLib.h \
           ../../higiia/3rd-party/artemis/include/krl/KrlFileHandler.h

#Artemis sources
SOURCES += ../../higiia/3rd-party/artemis/src/Pixel.cpp \
           ../../higiia/3rd-party/artemis/src/ImageBase.cpp \
           ../../higiia/3rd-party/artemis/src/dicom/DcmLib.cpp \
           ../../higiia/3rd-party/artemis/src/jpg/JpgLib.cpp \
           ../../higiia/3rd-party/artemis/src/bmp/BmpLib.cpp \
           ../../higiia/3rd-party/artemis/src/png/PngLib.cpp \
           ../../higiia/3rd-party/artemis/src/krl/KrlLib.cpp \
           ../../higiia/3rd-party/artemis/src/krl/KrlFileHandler.cpp

#Dicomlib headers
SOURCES += \
        ../../higiia/3rd-party/dicomlib/src/File.cpp \
        ../../higiia/3rd-party/dicomlib/src/UID.cpp \
        ../../higiia/3rd-party/dicomlib/src/Exceptions.cpp \
        ../../higiia/3rd-party/dicomlib/src/FileMetaInformation.cpp \
        ../../higiia/3rd-party/dicomlib/src/TransferSyntax.cpp \
        ../../higiia/3rd-party/dicomlib/src/Decoder.cpp \
        ../../higiia/3rd-party/dicomlib/src/Encoder.cpp \
        ../../higiia/3rd-party/dicomlib/src/Buffer.cpp \
        ../../higiia/3rd-party/dicomlib/src/Utility.cpp \
        ../../higiia/3rd-party/dicomlib/src/GroupLength.cpp \
        ../../higiia/3rd-party/dicomlib/src/DataDictionary.cpp

#Dicomlib sources
HEADERS += \
        ../../higiia/3rd-party/dicomlib/include/dicomlib.hpp \
        ../../higiia/3rd-party/dicomlib/include/File.hpp \
        ../../higiia/3rd-party/dicomlib/include/UIDs.hpp \
        ../../higiia/3rd-party/dicomlib/include/UID.hpp \
        ../../higiia/3rd-party/dicomlib/include/Utility.hpp \
        ../../higiia/3rd-party/dicomlib/include/Dumper.hpp \
        ../../higiia/3rd-party/dicomlib/include/Exceptions.hpp \
        ../../higiia/3rd-party/dicomlib/include/FileMetaInformation.hpp \
        ../../higiia/3rd-party/dicomlib/include/TransferSyntax.hpp \
        ../../higiia/3rd-party/dicomlib/include/Decoder.hpp \
        ../../higiia/3rd-party/dicomlib/include/Encoder.hpp \
        ../../higiia/3rd-party/dicomlib/include/Buffer.hpp \
        ../../higiia/3rd-party/dicomlib/include/Utility.hpp \
        ../../higiia/3rd-party/dicomlib/include/GroupLength.hpp \
        ../../higiia/3rd-party/dicomlib/include/DataDictionary.hpp




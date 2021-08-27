#ifndef UTIL_H
#define UTIL_H

//Qt includes
#include <QImage>
#include <QFileInfo>
#include <QDir>

//Artemis includes
#include <ImageBase.h>
#include <DcmLib.h>
#include <JpgLib.h>
#include <PngLib.h>
#include <BmpLib.h>
#include <KrlLib.h>

//Hermes includes
#include <BasicArrayObject.h>

class Util{

    public:
        enum SEARCH_TYPE{SIMILARITY_SEARCH, DIVERSITY_SEARCH, BRIDGE_SEARCH};

    public:
        Util();

        static int findColumn(QString key, QStringList caption);
        static int findRow(QString key, int column, QStringList tempRSet);
        static int findRowByFVId(size_t oid, int column, QStringList tempRSet);

        static QImage* convertImageToQImage(Image *src);
        static QStringList toStringList(const QList<QByteArray> list);
        static Image* openImage(QString filename);
        static Image* openThumbnail(QString filename);

        static void saveImageAndThumbnailToFS(QString filename, QByteArray imgStream);
        static void saveImageAndThumbnailToFS(QString filename, QByteArray imgStream, QSize thumbSize);
        static void removeImage(QString filename);
        static void print(QStringList list);
};

#endif // UTIL_H

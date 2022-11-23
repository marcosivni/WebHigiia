#ifndef UTIL_H
#define UTIL_H

//Qt includes
#include <QImage>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

//Artemis includes
#include <ImageBase.h>
#include <DcmLib.h>
#include <JpgLib.h>
#include <PngLib.h>
#include <BmpLib.h>
#include <KrlLib.h>

//Hermes includes
#include <BasicArrayObject.h>

const QString WFS_NAME = "fs/";
const QString PACS_BASE_URL = "https://www.dicomlibrary.com?study=";

class Util{

    public:
        enum SEARCH_TYPE{SIMILARITY_SEARCH, DIVERSITY_SEARCH, BRIDGE_SEARCH, CLUSTER_EXPANSION_SEARCH};

    public:
        Util();

        static int findColumn(QString key, QStringList caption);
        static int findRow(QString key, int column, QStringList tempRSet);
        static int findRowByFVId(size_t oid, int column, QStringList tempRSet);

        static QImage* convertImageToQImage(Image *src);
        static QStringList toStringList(const QList<QByteArray> list);
        static Image* openImage(QString filename, QString mask = QString());
        static Image* openThumbnail(QString filename);

        static void saveImageAndThumbnailToFS(QString filename, QByteArray imgStream, QString mask = QString());
        static void saveImageAndThumbnailToFS(QString filename, QByteArray imgStream, QSize thumbSize, QString mask = QString());
        static void removeDirectoryAndContent(QString dirPath = WFS_NAME);
        static void removeImage(QString filename);
        static void print(QStringList list);

        //Mask required methods
        static std::string serialize(std::string file);
        static Image unserialize(std::string dataIn);
        static QString readAndCompressMask(std::string file);
        static Image uncompressMask(QString zipStr);

        static QString mostFrequentValue(QStringList valuesIn);
        static double highestFrequencyPercent(QStringList valuesIn);
};

#endif // UTIL_H

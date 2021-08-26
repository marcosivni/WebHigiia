#include "Util.h"

Util::Util(){
}

int Util::findColumn(QString key, QStringList caption){

    int pos = -1;
    for (int x = 0; ((pos == -1) && (x < caption.size())); x++){
        if (caption.at(x).simplified().toUpper() == key.simplified().toUpper()){
            pos = x;
        }
    }
    return pos;
}

int Util::findRow(QString key, int column, QStringList tempRSet){

    int pos = -1;
    for (int x = 1; ((pos == -1) && (x < tempRSet.size())); x++){
        if (tempRSet.at(x).split(",").at(column).simplified() == key.simplified()){
            pos = x;
        }
    }
    return pos;
}

int Util::findRowByFVId(size_t oid, int column, QStringList tempRSet){

    FeatureVector oj;
    int pos = -1;
    for (int x = 1; ((pos == -1) && (x < tempRSet.size())); x++){
        oj.unserializeFromString(FeatureVector::fromBase64(tempRSet.at(x).split(",").at(column).simplified().toStdString()));
        if (oj.getOID() == oid){
            pos = x;
        }
    }

    return pos;
}

QImage* Util::convertImageToQImage(Image *src){

    QImage *image = new QImage(src->getWidth(), src->getHeight(), QImage::Format_RGB32);

    uint8_t div;
    if ((src->getBitsPerPixel() > 8) && (src->getBitsPerPixel() <= 16))
        div = 16;
    else
        div = 1;

    for(size_t x = 0; x < src->getWidth(); x++){
        for (size_t y = 0; y < src->getHeight(); y++){
            uint16_t pp = (uint16_t) (src->getPixel(x, y).getGrayPixelValue()/div);
            //
            if (    (src->getPixel(x, y).getGrayPixelValue() == pow(2.0, src->getBitsPerPixel()))
                 || (src->getPixel(x, y).getGrayPixelValue() == 4095)){
                pp = 0;
            }
            //
            image->setPixel(x, y, qRgb(pp, pp, pp));
        }
    }

    return image;
}


QStringList Util::toStringList(const QList<QByteArray> list) {

    QStringList strings;
    foreach (const QByteArray &item, list) {
        strings.append(QString::fromLocal8Bit(item)); // Assuming local 8-bit.
    }

    //Remove empty spaces at the end
    while (strings.size() && !strings.last().size()){
        strings.removeLast();
    }

    return strings;
}

Image* Util::openThumbnail(QString filename){

    return openImage(filename  + "_thumb.jpg");
}


Image* Util::openImage(QString filename){

    try{
        //It handles jpg, bmp, png, krl and dicom through Artemis external library
//        if (filename.split(".").last().toUpper() == "KRL"){
//            KRLImage *img = new KRLImage(ext);
//            return img;
//        }

        if (filename.split(".").last().toUpper() == "JPG" || filename.split(".").last().toUpper() == ".JPEG"){
            JPGImage *img = new JPGImage(filename.toStdString());
            return img;
        }

        //Extensionless files are interpreted as DICOM images...
        if (filename.split(".").last().toUpper() == "DCM" || filename.split(".").size() == 1){
            DCMImage *img = new DCMImage(filename.toStdString());
            return img;
        }
    } catch (std::exception e) {
        //File not found or format not supported
        return nullptr;
    }

    return nullptr;
}

void Util::saveImageAndThumbnailToFS(QString filename, QByteArray imgStream){

    QFileInfo info(filename);
    QDir dir(info.dir().path());
    if (!dir.exists()){
        dir.mkpath(".");
    }

    std::ofstream out;
    out.open(filename.toStdString().c_str(),  std::ofstream::out);
    out << imgStream.toStdString();
    out.close();

    Image *img = Util::openImage(filename);
    QImage *qimg = Util::convertImageToQImage(img);
    *qimg = qimg->scaled(120,120);
    qimg->save(filename + "_thumb.jpg");
    delete (img);
    delete (qimg);
}

void Util::saveImageAndThumbnailToFS(QString filename, QByteArray imgStream, QSize thumbSize){

    QFileInfo info(filename);
    QDir dir(info.dir().path());
    if (!dir.exists()){
        dir.mkpath(".");
    }

    std::ofstream out;
    out.open(filename.toStdString().c_str(),  std::ofstream::out);
    out << imgStream.toStdString();
    out.close();

    Image *img = Util::openImage(filename);
    QImage *qimg = Util::convertImageToQImage(img);
    *qimg = qimg->scaled(thumbSize);
    qimg->save(filename + "_thumb.jpg");
    delete (img);
    delete (qimg);
}

void Util::removeImage(QString filename){

    remove(filename.toStdString().c_str());
    remove((filename + "_thumb.jpg").toStdString().c_str());

    QFileInfo info(filename.toStdString().c_str());
    QDir dir(info.dir().path());
    if (dir.exists()){
        dir.rmpath(".");
    }
}

void Util::print(QStringList list){

    for (int x = 0; x < list.size(); x++){
        std::cout << x << " - " << list.at(x).toStdString() << std::endl;
    }
}


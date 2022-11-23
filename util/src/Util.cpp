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
    if ((src->getBitsPerPixel() > 8) && (src->getBitsPerPixel() <= 16)){
        div = 16;
    } else {
        div = 1;
    }

    for(size_t x = 0; x < src->getWidth(); x++){
        for (size_t y = 0; y < src->getHeight(); y++){
            uint16_t pp;
            if (src->isPhotometric()){
                if (div == 1){
                    pp = (uint8_t) (255 - src->getPixel(x, y).getGrayPixelValue());
                } else {
                    pp = (uint16_t) (65535 - src->getPixel(x, y).getGrayPixelValue())/div;
                }
            } else {
                if (div == 1){
                    pp = (uint8_t) src->getPixel(x, y).getGrayPixelValue();
                } else {
                    pp = (uint16_t) src->getPixel(x, y).getGrayPixelValue()/div;
                }
            }
            if ((src->getPixel(x, y).getRedPixelValue() != src->getPixel(x, y).getBluePixelValue()) || (src->getPixel(x, y).getGreenPixelValue() != src->getPixel(x, y).getBluePixelValue())){
                image->setPixel(x, y,
                                qRgb(
                                    src->getPixel(x, y).getRedPixelValue(),
                                    src->getPixel(x, y).getGreenPixelValue(),
                                    src->getPixel(x, y).getBluePixelValue()));
            } else {
                image->setPixel(x, y, qRgb(pp, pp, pp));
            }

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


Image* Util::openImage(QString filename, QString mask){

    filename = WFS_NAME + filename;
    Image *answer = nullptr;

    try{
        //It handles jpg, bmp, png, krl and dicom through Artemis external library
        if (filename.split(".").last().toUpper() == "KRL"){
            KRLImage *img = new KRLImage(filename.toStdString());
            answer = (Image*) img->clone();
            delete (img);
        }

        if (filename.split(".").last().toUpper() == "PNG"){
            PNGImage *img = new PNGImage(filename.toStdString());
            answer = (Image*) img->clone();
            delete (img);
        }

        if (filename.split(".").last().toUpper() == "BMP"){
            BMPImage *img = new BMPImage(filename.toStdString());
            answer = (Image*) img->clone();
            delete (img);
        }


        if (filename.split(".").last().toUpper() == "JPG" || filename.split(".").last().toUpper() == ".JPEG"){
            JPGImage *img = new JPGImage(filename.toStdString());
            answer = (Image*) img->clone();
            delete (img);
        }

        //Extensionless files are interpreted as DICOM images...
        if (filename.split(".").last().toUpper() == "DCM" || filename.split(".").size() == 1){
            DCMImage *img = new DCMImage(filename.toStdString());
            answer = (Image*) img->clone();
            delete (img);
        }
    } catch (std::exception e) {
        //File not found or format not supported
        return answer;
    }

    //
    if (mask.size()){
        Image maskImage = uncompressMask(mask);
        //Masking the original image
        if ((answer->getWidth() == maskImage.getWidth()) && (answer->getHeight() == maskImage.getHeight())){
            for (uint32_t x = 0; x < answer->getWidth(); x++){
                for (uint32_t y = 0; y < answer->getHeight(); y++){
                    if (maskImage.getPixel(x, y).getGrayPixelValue() == 255){
                        //Black for gray-scale monitors
                        //Pixel p(255, 255, 255);
                        //Nice yellow for color monitors
                        Pixel p(247, 245, 58);
                        answer->setPixel(x, y, p);
                    }
                }
            }
        }
    }
    //
    return answer;
}

void Util::saveImageAndThumbnailToFS(QString filename, QByteArray imgStream, QString mask){


    QFileInfo info(WFS_NAME + filename);
    QDir dir(info.dir().path());
    if (!dir.exists()){
        dir.mkpath(".");
    }

    std::ofstream out;
    out.open((WFS_NAME + filename).toStdString().c_str(),  std::ofstream::out);
    out << imgStream.toStdString();
    out.close();


    Image *img = Util::openImage(filename, mask);
    if (!img->windowedPixels()){
        Image *bck = img->windowing(img->getWindowWidth(), img->getWindowCenter());
        delete (img);
        img = bck;
    }
    QImage *qimg = Util::convertImageToQImage(img);
    *qimg = qimg->scaled(120,120);
    qimg->save((WFS_NAME + filename) + "_thumb.jpg");

    delete (img);
    delete (qimg);
}

void Util::saveImageAndThumbnailToFS(QString filename, QByteArray imgStream, QSize thumbSize, QString mask){

    QFileInfo info(WFS_NAME + filename);
    QDir dir(info.dir().path());
    if (!dir.exists()){
        dir.mkpath(".");
    }

    std::ofstream out;
    out.open((WFS_NAME + filename).toStdString().c_str(),  std::ofstream::out);
    out << imgStream.toStdString();
    out.close();

    Image *img = Util::openImage(filename, mask);
    if (!img->windowedPixels()){
        Image *bck = img->windowing(img->getWindowWidth(), img->getWindowCenter());
        delete (img);
        img = bck;
    }
    QImage *qimg = Util::convertImageToQImage(img);
    *qimg = qimg->scaled(thumbSize);
    qimg->save((WFS_NAME + filename) + "_thumb.jpg");

    delete (img);
    delete (qimg);
}

void Util::removeDirectoryAndContent(QString dirPath){

    QDir dir(dirPath);

    if (dir.exists()) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                removeDirectoryAndContent(info.absoluteFilePath());
            } else {
                QFile::remove(info.absoluteFilePath());
            }
        }
        QDir().rmdir(dirPath);
    }
}

void Util::removeImage(QString filename){

    filename = WFS_NAME + filename;

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

std::string Util::serialize(std::string file){

    std::string answer;
    BMPImage *image = new BMPImage(file);
    unsigned char *serialized = new unsigned char[sizeof(uint32_t) + sizeof(uint32_t) +  (sizeof(bool) * image->getWidth()*image->getHeight())];

    uint32_t size = image->getWidth();
    memcpy(serialized, &size, sizeof(uint32_t));
    size = image->getHeight();
    memcpy(serialized + sizeof(uint32_t), &size, sizeof(uint32_t));

    for (uint32_t x = 0; x < image->getHeight(); x++){
        for (uint32_t y = 0; y < image->getWidth(); y++){
            bool aux = 0;
            if (image->getPixel(y,x).getGrayPixelValue() == 255){
                aux = 1;
            }
            memcpy(serialized + (2*sizeof(uint32_t))+(sizeof(bool)*((x*image->getWidth()) + y)), &aux, sizeof(bool));
        }
    }

    for (unsigned long x = 0; x < sizeof(uint32_t) + sizeof(uint32_t) +  (sizeof(bool)*image->getWidth()*image->getHeight()); x++){
        answer += serialized[x];
    }

    delete[] (serialized);
    delete (image);

    return answer;
}

Image Util::unserialize(std::string dataIn){

    uint32_t w, h;
    bool *d;

    memcpy(&w, dataIn.c_str(), sizeof(uint32_t));
    memcpy(&h, dataIn.c_str() + sizeof(uint32_t), sizeof(uint32_t));

    Image bmp;
    bmp.createPixelMatrix(w, h);

    d = new bool[w*h];
    for (uint32_t x = 0; x < bmp.getHeight(); x++){
        for (uint32_t y = 0; y < bmp.getWidth(); y++){
            memcpy(&d[(x*bmp.getWidth()) + y], dataIn.c_str() + sizeof(uint32_t) + sizeof(uint32_t) + (sizeof(bool)*( (x*bmp.getWidth()) + y)), sizeof(bool));
            Pixel p;
            if (d[(x*bmp.getWidth()) + y]){
                p.setGrayPixelValue(255);
            }
            bmp.setPixel(y, x, p);
        }
    }

    delete[] (d);

    return bmp;
}

QString Util::readAndCompressMask(std::string file){

    std::string s = serialize(file);
    QString conversor = FeatureVector::toBase64(s).c_str();
    QByteArray byteArray(conversor.toUtf8());
    byteArray = qCompress(byteArray, 9);
    QString zipStr = QString::fromUtf8(byteArray.toBase64().data(), byteArray.toBase64().size());

    return zipStr;
}


Image Util::uncompressMask(QString zipStr){

    QByteArray byteArray = QByteArray::fromBase64(QByteArray(zipStr.toUtf8()));
    QString result;
    std::string answer;

    byteArray = qUncompress(byteArray);

    result = QString::fromUtf8(byteArray.data(), byteArray.size());
    answer = FeatureVector::fromBase64(result.toStdString());
    return unserialize(answer);
}


QString Util::mostFrequentValue(QStringList valuesIn){

    //Histogram <value, frequency>
    std::vector<QString> values;
    std::vector<uint16_t> frequencies;

    QString prediction = "Empty!";
    double maxFrequency = 0.0;

    //Build a histogram by scanning the result set...
    for (int i = 0; i < valuesIn.size(); i++){
        QString value = valuesIn.at(i);
        bool found = false;
        for (size_t itV = 0; ((!found) && (itV < values.size())); itV++){
            found = (value.toStdString() == values[itV].toStdString());
            if (found){
                frequencies[itV]++;
            }
        }
        if (!found){
            values.push_back(value);
            frequencies.push_back(1);
        }
    }

    for (size_t k = 0; k < values.size(); k++){
        if (maxFrequency < frequencies[k]){
            maxFrequency = frequencies[k];
            prediction = values[k];
        }
    }

    values.clear();
    frequencies.clear();

    return prediction;
}

double Util::highestFrequencyPercent(QStringList valuesIn){

    //Histogram <value, frequency>
    std::vector<QString> values;
    std::vector<uint16_t> frequencies;

    double sumFrequency, maxFrequency, answer;
    sumFrequency = maxFrequency = answer = 0.0;

    //Build a histogram by scanning the result set...
    for (int i = 0; i < valuesIn.size(); i++){
        QString value = valuesIn.at(i);
        bool found = false;
        for (size_t itV = 0; ((!found) && (itV < values.size())); itV++){
            found = (value.toStdString() == values[itV].toStdString());
            if (found){
                frequencies[itV]++;
            }
        }
        if (!found){
            values.push_back(value);
            frequencies.push_back(1);
        }
    }

    for (size_t k = 0; k < values.size(); k++){
        if (maxFrequency < frequencies[k]){
            maxFrequency = frequencies[k];
        }
        sumFrequency += frequencies[k];
    }
    values.clear();
    frequencies.clear();

    if (sumFrequency > 0){
        answer = maxFrequency/sumFrequency;
    }
    return answer;
}


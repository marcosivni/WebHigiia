#include "OberonViewer.h"
#include "ui_OberonViewer.h"

OberonViewer::OberonViewer(bool vTable,
                            QString scopeAttributes,
                            QString tableName,
                            QString queryCenterName,
                            QString similarityAttribute,
                            FeatureVector queryCenter,
                            MedicalImageTable rSet,
                            Util::SEARCH_TYPE searchType,
                            uint16_t k,
                            uint16_t kBridge,
                            QString vTableName,
                            QString metricName,
                            QWebSocket *webSocket,
                            int32_t oqId,
                            int32_t userId, QString link,
                            QWidget *parent)
    : QMainWindow(parent), ui(new Ui::OberonViewer){

    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);

    this->searchType = searchType;
    this->rSet = rSet;
    this->simAttribute = similarityAttribute;
    this->tableName = tableName;
    this->webSocket = webSocket;
    this->oqFileName = queryCenterName;
    this->oq = queryCenter;
    this->vTable = vTable;
    this->scopeAttributes = scopeAttributes;
    this->vTableName = vTableName;
    this->metricName = metricName;
    this->link = link;
    this->oqId = oqId;
    this->userId = QString::number(userId);

    ui->txtKnn->setText(QString::number(k));
    this->kBridge = QString::number(kBridge);

    currentImage = nullptr;
    queryImage = nullptr;

    //Provenance-related stuff
    #if M_PROV
        diagnosisForm = nullptr;
    #endif


    ui->lblServerSetup->setText("Loading, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state01);
    webSocket->sendBinaryMessage(scopeCaption.generateSQLBaseQuery(tableName).toStdString().c_str());
}

void OberonViewer::state01(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state01);

    int posAtt, posValue, posCaption;
    ResultTable tuples(Util::toStringList(message.split('\n')));

    posAtt= tuples.locateColumn("attributeN");
    posValue= tuples.locateColumn("valueN");
    posCaption= tuples.locateColumn("caption");

    //Load caption for user-defined scope...
    for (int x = 0; x < tuples.size(); x++){
        scopeCaption.addEntry(tuples.fetchByColumnId(x, posAtt), tuples.fetchByColumnId(x, posValue), tuples.fetchByColumnId(x, posCaption));
    }

    splitResultSets();
}

void OberonViewer::splitResultSets(){

    //Lock screen widgets...
    ui->tabWidget->hide();
    ui->lytCentral->setEnabled(false);

    //In-memory FS for WASM - File sync finishes at state 04
    if (searchType == Util::SIMILARITY_SEARCH || searchType == Util::CLUSTER_EXPANSION_SEARCH || searchType == Util::DIVERSITY_SEARCH){
        downloadNonInfluencedMasks();
    } else {
        loadInfluencedImages();
    }
}

void OberonViewer::fillThumbnails(){

    int posClass = rSet.locateColumn("Image_Class");
    //<html><head/><body><p align="center"><br/></p></body></html>
    //Sanity-check only
    if (posClass != -1){
        QStringList labels = rSet.fetchColumn(posClass);
        ui->lblClassification->setText("<html><head/><body><p align=\"center\">" +
                                       scopeCaption.matchCaption("Image_Class", Util::mostFrequentValue(labels).simplified()) + " (" +
                                       QString::number(Util::highestFrequencyPercent(labels)*100.0, 'f', 2) + "%)" +
                                       "</body></html>");
    }

    currentImage = Util::openImage(oqFileName, mapNameToMask[oqFileName]);
    adjustSliders();
    fillQueryCenter(currentImage);
    if (!currentImage->windowedPixels()){
        showWindowing();
    }
    queryImage = currentImage;

    if (searchType == Util::SIMILARITY_SEARCH || searchType == Util::CLUSTER_EXPANSION_SEARCH  || searchType == Util::DIVERSITY_SEARCH){
        //Single result set
        loadThumbnailsRight(rSet);
    } else {
        //Two-level and clustered result set
        ui->lytDiversity->setAlignment(Qt::AlignLeft);
        ui->lytMyList->setAlignment(Qt::AlignLeft);
        loadThumbnailsRight(rSet);
        tabs[0] = ui->lstInfluencedImages;
        tabs[1] = ui->lstMyList;
        ui->lstInfluencedImages->setEnabled(false);
        ui->lstMyList->setEnabled(false);
        ui->tabWidget->clear();
    }
}

void OberonViewer::loadInfluencedImages(){

    ui->lblServerSetup->setText("Loading, please wait ...");

    if (!rSet.size()){
        ui->lblServerSetup->setText("Empty result set!");
        return ;
    }

    //Separate influencers and influenced
    FeatureVector oj;
    MedicalImageTable newRSet;
    int colId, colInfluenced;

    tempRInfset.clear();
    newRSet.addCaption(rSet.fetchCaption());
    tempRInfset.addCaption(rSet.fetchCaption());

    colId = rSet.locateImageID();
    colInfluenced = rSet.locateColumn(simAttribute + "$bridged");

    for (int x = 0; x < rSet.size(); x++){
        bool found = false;
        for (int y = 0; ((!found) && (y < newRSet.size())); y++){
            found = (newRSet.fetchByColumnId(y, colId) == rSet.fetchByColumnId(x, colId));
        }
        if (!found){
            newRSet.addTuple(rSet.fetchTupleByRowId(x));
        }
        tempRInfset.addTuple(rSet.fetchTupleByRowId(x));
    }
    rSet.clear();
    rSet = newRSet;

    //Requested data from first influenced entry, then loop...
    counterImgsDownPanel = 0;
    if (counterImgsDownPanel < tempRInfset.size()){

        SirenSQLQuery queryT;
        QString tblName;
        QStringList scope = scopeAttributes.split(",");

        oj.unserializeFromString(FeatureVector::fromBase64(tempRInfset.fetchByColumnId(counterImgsDownPanel, colInfluenced).toStdString()));
        for (int x = 0; x < scope.size(); x++){
            queryT.addProjectionAttribute(scope.at(x).simplified());
        }
        if (vTable){
            tblName = "temp";
        } else {
            tblName = tableName;
        }

        queryT.addProjectionList({ tblName + ".Filename Filename", tblName + ".Id Id"});
        queryT.addTable(tableName + " AS " + tblName);
        queryT.addWhereAttribute(simAttribute + " = " + QString::number(oj.getOID()));

        ui->lblServerSetup->setText("Downloading, please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state02);
        webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
    } else {
        downloadInfluencedMasks();
    }
}

void OberonViewer::state02(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state02);

    FeatureVector oj;
    int posIdTempSet, posIdInfSet, posIdRSet, colInfluenced;
    MedicalImageTable tuples(Util::toStringList(message.split('\n')));

    if (!rInfset.fetchCaption().size()){
        rInfset.addCaption(tuples.fetchCaption());
    }

    posIdTempSet = tempRInfset.locateImageID();
    posIdInfSet = rInfset.locateImageID();
    posIdRSet = rSet.locateImageID();
    colInfluenced = tempRInfset.locateColumn(simAttribute + "$bridged");


    QString key = tempRInfset.fetchByColumnId(counterImgsDownPanel, posIdTempSet);
    uint16_t pos = rSet.locateFirstRow(key, posIdRSet);


    if (tuples.fetchByColumnId(0, posIdInfSet) != rSet.fetchByColumnId(pos, posIdRSet)){
        rInfset.addTuple(tuples.fetchTupleByRowId(0));
        mapInfluencedRows.insert(pos, rInfset.size()-1);
    }

    counterImgsDownPanel++;
    if (counterImgsDownPanel < tempRInfset.size()){

        SirenSQLQuery queryT;
        QString tblName;
        QStringList scope = scopeAttributes.split(",");

        oj.unserializeFromString(FeatureVector::fromBase64(tempRInfset.fetchByColumnId(counterImgsDownPanel, colInfluenced).toStdString()));
        for (int x = 0; x < scope.size(); x++){
            queryT.addProjectionAttribute(scope.at(x).simplified());
        }
        if (vTable){
            tblName = "temp";
        } else {
            tblName = tableName;
        }

        queryT.addProjectionList({ tblName + ".Filename Filename", tblName + ".Id Id"});
        queryT.addTable(tableName + " AS " + tblName);
        queryT.addWhereAttribute(simAttribute + " = " + QString::number(oj.getOID()));

        ui->lblServerSetup->setText("Downloading, please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state02);
        webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
    } else {
        downloadInfluencedMasks();
    }
}


void OberonViewer::downloadInfluencedImages(){

    int posImg, posId;
    //Dismantle temporary second-level result set
    tempRInfset.clear();
    //Recycle global counterImgsDownPanel
    counterImgsDownPanel = 0;

    posImg = rInfset.locateImageFilename();
    posId = rInfset.locateImageID();

    while (counterImgsDownPanel < rInfset.size()
           && QFileInfo::exists(WFS_NAME + rInfset.fetchByColumnId(counterImgsDownPanel, posImg))
           && QFileInfo(WFS_NAME + rInfset.fetchByColumnId(counterImgsDownPanel, posImg)).isFile()) {
        mapOidToNames[rInfset.fetchByColumnId(counterImgsDownPanel, posId).toUInt()] = rInfset.fetchByColumnId(counterImgsDownPanel, posImg).toStdString();
        counterImgsDownPanel++;
    }

    if (counterImgsDownPanel < rInfset.size()){
        QString request = "REQUEST " + rInfset.fetchByColumnId(counterImgsDownPanel, posImg);
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state03);
        ui->lblServerSetup->setText("Downloading " + QString::number(counterImgsDownPanel) + "/" + QString::number(rInfset.size())+ ", please wait ...");
        webSocket->sendBinaryMessage(request.toStdString().c_str());
    } else {
        downloadNonInfluencedMasks();
    }
}

void OberonViewer::state03(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state03);

    int posImg, posId;
    posImg = rInfset.locateImageFilename();
    posId = rInfset.locateImageID();

    QString filename = rInfset.fetchByColumnId(counterImgsDownPanel, posImg);

    if (!filename.toStdString().empty()){
        Util::saveImageAndThumbnailToFS(filename, message, mapNameToMask[filename]);
        mapOidToNames[rInfset.fetchByColumnId(counterImgsDownPanel, posId).toUInt()] = filename.toStdString();
    } else {
        ui->lblServerSetup->setText("Local storage fatal error.");
    }

    counterImgsDownPanel++;
    while (counterImgsDownPanel < rInfset.size()
           && QFileInfo::exists(WFS_NAME + rInfset.fetchByColumnId(counterImgsDownPanel, posImg))
           && QFileInfo(WFS_NAME + rInfset.fetchByColumnId(counterImgsDownPanel, posImg)).isFile()) {
        mapOidToNames[rInfset.fetchByColumnId(counterImgsDownPanel, posId).toUInt()] = rInfset.fetchByColumnId(counterImgsDownPanel, posImg).toStdString();
        counterImgsDownPanel++;
    }

    if (counterImgsDownPanel < rInfset.size()){
        QString request = "REQUEST " + rInfset.fetchByColumnId(counterImgsDownPanel, posImg);
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state03);
        ui->lblServerSetup->setText("Downloading " + QString::number(counterImgsDownPanel) + "/" + QString::number(rInfset.size())+ ", please wait ...");
        webSocket->sendBinaryMessage(request.toStdString().c_str());
    } else {
        downloadNonInfluencedMasks();
    }
}

void OberonViewer::downloadNonInfluencedMasks(){

    QStringList ids;
    for (int x = 0; x < rSet.size(); x++){
        ids.append(rSet.fetchByColumnId(x, rSet.locateImageID()));
    }


    SirenSQLQuery buildMaskStatement;
    buildMaskStatement.addProjectionAttribute("Id, Mask");
    buildMaskStatement.addTable(tableName);

    QString inList = "Id IN (";
    for (int x = 0; x < ids.size(); x++){
        if (x > 0){
            inList.append(", ");
        }
        inList.append(ids[x]);
    }
    inList.append(" )");
    buildMaskStatement.addWhereAttribute(inList);

    ui->lblServerSetup->setText("Loading masks, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state09);
    webSocket->sendBinaryMessage(buildMaskStatement.generateQuery().toStdString().c_str());
}

void OberonViewer::downloadInfluencedMasks(){

    QStringList ids;
    for (int x = 0; x < rInfset.size(); x++){
        ids.append(rInfset.fetchByColumnId(x, rInfset.locateImageID()));
    }


    SirenSQLQuery buildMaskStatement;
    buildMaskStatement.addProjectionAttribute("Id, Mask");
    buildMaskStatement.addTable(tableName);

    QString inList = "Id IN (";
    for (int x = 0; x < ids.size(); x++){
        if (x > 0){
            inList.append(", ");
        }
        inList.append(ids[x]);
    }
    inList.append(" )");
    buildMaskStatement.addWhereAttribute(inList);

    ui->lblServerSetup->setText("Loading masks, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state10);
    webSocket->sendBinaryMessage(buildMaskStatement.generateQuery().toStdString().c_str());
}

void OberonViewer::downloadNonInfluencedImages(){

    int posImg, posId;
    counterImgsRightPanel = 0;

    posImg = rSet.locateImageFilename();
    posId = rSet.locateImageID();

    while (counterImgsRightPanel < rSet.size()
           && QFileInfo::exists(WFS_NAME + rSet.fetchByColumnId(counterImgsRightPanel, posImg))
           && QFileInfo(WFS_NAME + rSet.fetchByColumnId(counterImgsRightPanel, posImg)).isFile()){
        mapOidToNames[rSet.fetchByColumnId(counterImgsRightPanel, posId).toUInt()] = rSet.fetchByColumnId(counterImgsRightPanel, posImg).toStdString();
        counterImgsRightPanel++;
    }

    if (counterImgsRightPanel < rSet.size()){
        QString request = "REQUEST " + rSet.fetchByColumnId(counterImgsRightPanel, posImg);
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state04);
        ui->lblServerSetup->setText("Downloading " + QString::number(counterImgsRightPanel) + "/" + QString::number(rSet.size())+ ", please wait ...");
        webSocket->sendBinaryMessage(request.toStdString().c_str());
    } else {
        downloadOqMask();
    }
}


void OberonViewer::state04(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state04);
    ui->lblServerSetup->setText("Saving, please wait ...");

    int posImg, posId;
    QString filename;
    posImg = rSet.locateImageFilename();
    posId = rSet.locateImageID();

    filename = rSet.fetchByColumnId(counterImgsRightPanel, posImg);
    if (!filename.toStdString().empty()){
        Util::saveImageAndThumbnailToFS(filename, message, mapNameToMask[filename]);
        mapOidToNames[rSet.fetchByColumnId(counterImgsRightPanel, posId).toUInt()] = filename.toStdString();
    } else {
        ui->lblServerSetup->setText("Local storage fatal error.");
    }

    counterImgsRightPanel++;
    while (counterImgsRightPanel < rSet.size()
           && QFileInfo::exists(WFS_NAME + rSet.fetchByColumnId(counterImgsRightPanel, posImg))
           && QFileInfo(WFS_NAME + rSet.fetchByColumnId(counterImgsRightPanel, posImg)).isFile()){
        mapOidToNames[rSet.fetchByColumnId(counterImgsRightPanel, posId).toUInt()] = rSet.fetchByColumnId(counterImgsRightPanel, posImg).toStdString();
        counterImgsRightPanel++;
    }

    if (counterImgsRightPanel < rSet.size()){
        QString request = "REQUEST " + rSet.fetchByColumnId(counterImgsRightPanel, posImg);
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state04);
        ui->lblServerSetup->setText("Downloading " + QString::number(counterImgsRightPanel) + "/" + QString::number(rSet.size())+ ", please wait ...");
        webSocket->sendBinaryMessage(request.toStdString().c_str());
    } else {
        downloadOqMask();
    }
}

void OberonViewer::downloadOqMask(){

    SirenSQLQuery buildMaskStatement;

    buildMaskStatement.addProjectionAttribute("Id, Mask");
    buildMaskStatement.addTable("U_" + tableName);
    buildMaskStatement.addWhereAttribute("Id = " + QString::number(oqId));

    ui->lblServerSetup->setText("Loading masks, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state08);
    webSocket->sendBinaryMessage(buildMaskStatement.generateQuery().toStdString().c_str());
}

void OberonViewer::downloadOq(){

    ui->lblServerSetup->setText("Downloading center, please wait ...");

    if (QFileInfo::exists(WFS_NAME + oqFileName.simplified() )
            && QFileInfo(WFS_NAME + oqFileName.simplified()).isFile()){
        fillThumbnails();
        //Unlocking main screen....
        ui->lytCentral->setEnabled(true);
        ui->lblServerSetup->setText("CBIR is ready!");
    } else {
        QString request = "REQUEST " + oqFileName.simplified();
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state05);
        ui->lblServerSetup->setText("Building, please wait ...");
        webSocket->sendBinaryMessage(request.toStdString().c_str());
    }
}

void OberonViewer::state05(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state05);
    ui->lblServerSetup->setText("Saving file, please wait ...");

    if (!oqFileName.toStdString().empty()){
        Util::saveImageAndThumbnailToFS(oqFileName, message, mapNameToMask[oqFileName]);
    } else {
        ui->lblServerSetup->setText("Local storage fatal error.");
    }

    fillThumbnails();
    //Unlocking main screen....
    ui->lytCentral->setEnabled(true);
    ui->lblServerSetup->setText("CBIR is ready!");
}


QString OberonViewer::toolTipBuilder(MedicalImageTable tempSet, int row){

    QString answer, attName;
    std::string caption;
    int posId, posImg, posBridge, posBridgedId;

    posId = tempSet.locateImageID();
    posImg = tempSet.locateImageFilename();
    posBridge = tempSet.locateColumn(simAttribute + "$bridged");
    posBridgedId = tempSet.locateColumn(simAttribute + "$_id");

    answer += "<html><head/><body>";
    for (int x = 0; ((tempSet.size() > row) && (x < tempSet.dimensionality())); x++){
        if (x != posId && x != posImg && x != posBridge && x != posBridgedId){
            if (answer.size() > QString("<html><head/><body>").size()){
                answer += "<br>";
            }
            attName = tempSet.fetchCaption(x).simplified();
            answer += attName + ": ";
            if (scopeCaption.findCaption(tempSet.fetchCaption(x),
                                         tempSet.fetchByColumnId(row, x),
                                         &caption)){
                answer += QString(caption.c_str());
            } else {
                if (attName.toUpper() == "URL"){
                    answer += "<a href=\"" + QString(tempSet.fetchByColumnId(row, x)) + "\" target=\"_blank\">See series</a>";
                } else {
                    answer += tempSet.fetchByColumnId(row, x);
                }
            }
        }
    }
    answer += "</body></html>";

    return answer;
}

void OberonViewer::loadThumbnailsRight(MedicalImageTable tempSet){

    ui->lblServerSetup->setText("Loading thumbs, please wait ...");

    int posImg, posId;
    int row, column;

    posId = tempSet.locateImageID();
    posImg = tempSet.locateImageFilename();


    //Delete thumbs inside the gridlayout
    for (size_t x = 0; x < thumbsImagesRight.size(); x++){
        QPushButton *tmp = thumbsImagesRight[x];
        ui->gridLayout_5->removeItem(ui->gridLayout_5->itemAt(ui->gridLayout_5->indexOf(tmp)));
        delete (tmp);
    }
    thumbsImagesRight.clear();

    ui->gridLayout_5->update();
    ui->scrollAreaSimilar->update();

    for (int x = 0; x < tempSet.size(); x++){
        //Check if the image was removed (marked as unrelevant)
        bool removed = false;
        for(size_t i = 0; ((!removed) && (i < removedListOfIds.size())); i++){
            removed = (tempSet.fetchByColumnId(x, posId).toUInt() == removedListOfIds[i]);
        }

        if(!removed){
            QPushButtonAlter *btn = new QPushButtonAlter(ui->scrollAreaSimilar);
            connect(btn, SIGNAL(rightClicked()), this, SLOT(unRelevantImageClickedRight()));
            connect(btn, SIGNAL(leftDoubleClicked()), this, SLOT(changeCenterImage()));

            if(searchType == Util::BRIDGE_SEARCH){
                connect(btn, SIGNAL(clicked()), this, SLOT(diversityImageClicked()));
            } else{
                connect(btn, SIGNAL(clicked()), this, SLOT(relevantImageClicked()));
            }

            btn->prepare(tempSet.fetchByColumnId(x, posImg));
            btn->setWhatsThis(tempSet.fetchByColumnId(x, posId));
            btn->setToolTip(toolTipBuilder(tempSet, x));

            row = floor(thumbsImagesRight.size()/2.0);
            column = thumbsImagesRight.size()%2;
            thumbsImagesRight.push_back(btn);
            ui->gridLayout_5->addWidget(btn, row, column);
        }
    }
    ui->lytSimilar->update();
    ui->scrollAreaSimilar->update();
}

void OberonViewer::unRelevantImageClickedRight(){

    QPushButton *tmp = (QPushButton *)sender();
    tmp->setFlat(false || (!tmp->isFlat()));

    uint32_t OID = tmp->whatsThis().toUInt();

    if (!tmp->isFlat()){
            std::vector<QPushButton *> aux;
            for (size_t x = 0; x < thumbsImagesRight.size(); x++){
                if (x != ui->gridLayout_5->indexOf(tmp)){
                    aux.push_back(thumbsImagesRight[x]);
                }
            }
            thumbsImagesRight.clear();
            thumbsImagesRight = aux;
            ui->gridLayout_5->removeItem(ui->gridLayout_5->itemAt(ui->gridLayout_5->indexOf(tmp)));
            removedListOfIds.push_back(OID);

            //Checks if this image is an element of the user-selected list of relevance and removes it
            std::vector<uint32_t> auxList;
            for(size_t i = 0; i < myList.size(); i++){
                if(myList[i] != OID)
                    auxList.push_back(myList[i]);
            }
            myList = auxList;
            loadThumbnailsMyList();
            tabManager(3);
            delete (tmp);
    } else {
        tmp->setWindowTitle("0");
        tmp->setStyleSheet(" QPushButton { background: black; border: none }");
    }
}

void OberonViewer::fillQueryCenter(Image *src, uint32_t scale){

    QImage *img;
    img = Util::convertImageToQImage(src);
    ui->lblCenter->clear();
    ui->lblCenter->setPixmap(QPixmap::fromImage(img->scaled(scale, scale)));

    if(queryImage == nullptr){
        if (!src->windowedPixels()){
            Image *aux = src->windowing(src->getWindowWidth(), src->getWindowCenter());
            delete(img);
            img = Util::convertImageToQImage(aux);
            delete(aux);
        }
        QIcon ico;
        ico.addPixmap(QPixmap::fromImage(img->scaled(200, 200)));
        ui->btnQueryCenter->setIcon(ico);
    }

    ui->lblCenter->setMinimumWidth(600);
    ui->lblCenter->setMinimumHeight(600);

    delete (img);
}

void OberonViewer::changeCenterImage(){

    QPushButton *tmp = (QPushButton *)sender();
    uint16_t x = tmp->whatsThis().toUInt();

    if(currentImage != queryImage){
        delete currentImage;
    }

    std::string imageName = mapOidToNames[x];
    currentImage = Util::openImage(imageName.c_str(), mapNameToMask[imageName.c_str()]);

    //Reset windowing settings
    ui->cbxWindowing->setCurrentIndex(0);
    adjustSliders();

    fillQueryCenter(currentImage);
    if (!currentImage->windowedPixels()){
        showWindowing();
    }
}

void OberonViewer::diversityImageClicked() {

    QPushButton *tmp = (QPushButton *)sender();
    FeatureVector fv;
    int posId, rowId;
    uint32_t oid = tmp->whatsThis().toUInt();

    posId = rSet.locateImageID();
    rowId = rSet.locateFirstRow(QString::number(oid), posId);

    tmp->setFlat(false || (!tmp->isFlat()));
    ui->lblDetails->setText(tmp->toolTip());
    //
    ui->lblDetails->setTextFormat(Qt::RichText);
    ui->lblDetails->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->lblDetails->setOpenExternalLinks(true);
    //

    if (!tmp->isFlat()){
        for(size_t x = 0; x < thumbsImagesRight.size(); x++){
            QPushButton *aux = thumbsImagesRight[x];
            aux->setWindowTitle("0");
            tmp->setStyleSheet(" QPushButton { background: black; border: none }");
            aux->setFlat(true);
        }
        tmp->setStyleSheet(" QPushButton { background: black; border:2px solid #81a10e }");
        tmp->setWindowTitle("1");
        tmp->setFlat(false);

        if (mapInfluencedRows.contains(rowId)){
            loadThumbnailsDown(mapInfluencedRows.values(rowId));
            diversityIndex = rowId;
            tabManager(0);
        } else{
            tabManager(1);
        }
    } else {
        tmp->setWindowTitle("0");
        tmp->setStyleSheet(" QPushButton { background: black; border: none }");
        tabManager(1);
    }
}

void OberonViewer::relevantImageClicked() {

    QPushButton *tmp = (QPushButton *)sender();
    tmp->setFlat(false || (!tmp->isFlat()));

    if (!tmp->isFlat()){
        tmp->setStyleSheet(" QPushButton { background: black; border:2px solid #81a10e }");
        tmp->setWindowTitle("1");
        ui->lblDetails->setText(tmp->toolTip());
        //
        ui->lblDetails->setTextFormat(Qt::RichText);
        ui->lblDetails->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->lblDetails->setOpenExternalLinks(true);
        //
    } else {
        tmp->setWindowTitle("0");
        tmp->setStyleSheet(" QPushButton { background: black; border: none }");
    }
}

void OberonViewer::loadThumbnailsMyList(){

    int pos, pos2;

    for (size_t x = 0; x < thumbsImagesMyList.size(); x++){
        QPushButton *tmp = thumbsImagesMyList[x];
        ui->lytDiversity->removeItem(ui->lytMyList->itemAt(ui->lytMyList->indexOf(tmp)));
        delete (tmp);
    }
    thumbsImagesMyList.clear();
    ui->lytMyList->update();
    ui->scrollAreaMyList->update();

    for (size_t x = 0; x < myList.size() ; x++){
        QPushButtonAlter *btn = new QPushButtonAlter(ui->scrollAreaMyList);
        connect(btn, SIGNAL(rightClicked()), this, SLOT(unRelevantImageClickedMyList()));
        connect(btn, SIGNAL(leftDoubleClicked()), this, SLOT(changeCenterImage()));

        pos = rInfset.locateImageID();
        pos2 = rInfset.locateFirstRow(QString::number(myList[x]), pos);

        btn->prepare(mapOidToNames[myList[x]].c_str());
        btn->setWhatsThis(QString::number(myList[x]));
        btn->setToolTip(toolTipBuilder(rInfset, pos2));

        ui->lytMyList->addWidget(btn,0,x);
        thumbsImagesMyList.push_back(btn);
        ui->lytMyList->setColumnMinimumWidth(x,120);
    }
    ui->lytMyList->update();
    ui->scrollAreaMyList->update();
}

/**
* Set tabs according to the diversity and the mylist of results.
* 0 - Diversity image clicked
* 1 - Diversity image non-clicked
* 2 - Add a new image to my list
* 3 - Remove a image from my list
*/
void OberonViewer::tabManager(uint8_t operation){

    switch(operation){
    case 0: {
        if(!ui->lstInfluencedImages->isEnabled()){
            ui->tabWidget->insertTab(0, tabs[0], QString("List of influenced images"));
            ui->lstInfluencedImages->setEnabled(true);
            ui->tabWidget->show();
        }
        ui->tabWidget->setCurrentIndex(0);
        break;
    }
    case 1: {
        if(myList.size() != 0){
            if(ui->lstInfluencedImages->isEnabled())
                ui->tabWidget->removeTab(0);
        }else{
            ui->tabWidget->clear();
            ui->tabWidget->hide();
            ui->lstMyList->setEnabled(false);
        }
        ui->lstInfluencedImages->setEnabled(false);
        break;
    }
    case 2: {
        if(!ui->lstMyList->isEnabled()){
            ui->tabWidget->insertTab(1, tabs[1], QString("My list of relevant images"));
            ui->lstMyList->setEnabled(true);
            ui->tabWidget->show();
        }
        break;
    }
    case 3: {
        if(myList.size() == 0){
            if(ui->lstInfluencedImages->isEnabled())
                ui->tabWidget->removeTab(1);
            else
                ui->tabWidget->removeTab(0);

            ui->lstMyList->setEnabled(false);
            if(thumbsImagesDown.size() == 0){
                ui->tabWidget->removeTab(0);
                ui->tabWidget->clear();
                ui->tabWidget->hide();
                ui->lstInfluencedImages->setEnabled(false);
            }
        }
        break;
    }
    }
}

void OberonViewer::loadThumbnailsDown(QList<uint16_t> rowIds){

    int posId;

    //Clear images inside lytDiversity
    for (size_t x = 0; x < thumbsImagesDown.size(); x++){
        QPushButton *tmp = thumbsImagesDown[x];
        ui->lytDiversity->removeItem(ui->lytDiversity->itemAt(ui->lytDiversity->indexOf(tmp)));
        delete (tmp);
    }
    thumbsImagesDown.clear();
    ui->lytDiversity->update();
    ui->scrollAreaInfluenced->update();

    posId = rInfset.locateImageID();
    for (int x = 0; x < rowIds.size(); x++){
        //Discard unrelevant images...
        bool removed = false;
        for(size_t y = 0; ((!removed) && (y < removedListOfIds.size())); y++){
            removed = (rInfset.fetchByColumnId(rowIds.at(x), posId).toUInt() == removedListOfIds[y]);
        }

        if(!removed){
            QPushButtonAlter *btn = new QPushButtonAlter(ui->scrollAreaInfluenced);
            connect(btn, SIGNAL(clicked()), this, SLOT(relevantImageClickedAddMyList()));
            connect(btn, SIGNAL(rightClicked()), this, SLOT(unRelevantImageClickedDown()));
            connect(btn, SIGNAL(leftDoubleClicked()), this, SLOT(changeCenterImage()));

            btn->prepare(mapOidToNames[rInfset.fetchByColumnId(rowIds.at(x), posId).toUInt()].c_str());
            btn->setWhatsThis(rInfset.fetchByColumnId(rowIds.at(x), posId));
            btn->setToolTip(toolTipBuilder(rInfset, rowIds.at(x)));


            //Highlights images in the user-selected list
            bool favorite = false;
            for(size_t y = 0; ((!favorite) && (y < myList.size())); y++){
                favorite = (rInfset.fetchByColumnId(rowIds.at(x), posId).toUInt() == myList[y]);
            }
            if(!favorite){
                btn->setFlat(true);
            } else {
                btn->setStyleSheet(" QPushButton { background: black; border:2px solid #81a10e }");
                btn->setFlat(false);
            }

            ui->lytDiversity->addWidget(btn,0,x);
            thumbsImagesDown.push_back(btn);
            ui->lytDiversity->setColumnMinimumWidth(x,120);
        }
    }
    ui->lytDiversity->update();
    ui->scrollAreaInfluenced->update();
}

void OberonViewer::relevantImageClickedAddMyList(){

    QPushButton *tmp = (QPushButton *)sender();
    uint32_t x = tmp->whatsThis().toUInt();
    tmp->setFlat(false || (!tmp->isFlat()));

    if (!tmp->isFlat()){
        myList.push_back(x);
        loadThumbnailsMyList();
        tmp->setStyleSheet(" QPushButton { background: black; border:2px solid #81a10e }");
        tmp->setWindowTitle("1");
        tabManager(2);
        ui->lblDetails->setText(tmp->toolTip());
        //
        ui->lblDetails->setTextFormat(Qt::RichText);
        ui->lblDetails->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->lblDetails->setOpenExternalLinks(true);
        //
    } else {
        tmp->setWindowTitle("0");
        tmp->setStyleSheet(" QPushButton { background: black; border: none }");
        std::vector<uint32_t> aux;
        for(size_t i = 0; i < myList.size(); i++){
            if(myList[i] != x)
                aux.push_back(myList[i]);
        }
        myList = aux;
        loadThumbnailsMyList();
        tabManager(3);
    }
}

void OberonViewer::unRelevantImageClickedDown(){

    QPushButton *tmp = (QPushButton *)sender();
    tmp->setFlat(false || (!tmp->isFlat()));
    uint32_t OID = tmp->whatsThis().toUInt();

    if (!tmp->isFlat()){
        std::vector<QPushButton *> aux;
        for (size_t x = 0; x < thumbsImagesDown.size(); x++){
            if (x != ui->lytDiversity->indexOf(tmp)){
                aux.push_back(thumbsImagesDown[x]);
            }
        }
        thumbsImagesDown.clear();
        thumbsImagesDown = aux;

        ui->lytDiversity->removeItem(ui->lytDiversity->itemAt(ui->lytDiversity->indexOf(tmp)));
        removedListOfIds.push_back(OID);
        std::vector<uint32_t> auxList;
        for(size_t i = 0; i < myList.size(); i++){
            if(myList[i] != OID){
                auxList.push_back(myList[i]);
            }
        }
        myList = auxList;
        loadThumbnailsMyList();
        tabManager(3);

        delete (tmp);
    } else {
        tmp->setWindowTitle("0");
        tmp->setStyleSheet(" QPushButton { background: black; border: none }");
    }
}

void OberonViewer::unRelevantImageClickedMyList(){

    QPushButton *tmp = (QPushButton *)sender();
    tmp->setFlat(false || (!tmp->isFlat()));
    uint32_t OID = tmp->whatsThis().toUInt();

    if (!tmp->isFlat()){
        std::vector<QPushButton *> auxButton;
        for (size_t i = 0; i < thumbsImagesMyList.size(); i++){
            if (i != ui->lytMyList->indexOf(tmp)){
                auxButton.push_back(thumbsImagesMyList[i]);
            }
        }
        thumbsImagesMyList.clear();
        thumbsImagesMyList = auxButton;

        removedListOfIds.push_back(OID);
        std::vector<uint32_t> aux;
        for(size_t i = 0; i < myList.size(); i++){
            if(myList[i] != OID)
                aux.push_back(myList[i]);
        }
        myList = aux;
        loadThumbnailsMyList();
        tabManager(3);

        //Reload thumbnails down panel
        if (mapInfluencedRows.values(diversityIndex).size()){
            loadThumbnailsDown(mapInfluencedRows.values(diversityIndex));
            tabManager(3);
        }else{
            tabManager(1);
        }

        delete(tmp);
    } else {
        tmp->setWindowTitle("0");
        tmp->setStyleSheet(" QPushButton { background: black; border: none }");
    }
}



OberonViewer::~OberonViewer(){

    clearAllThumbnails();
    clearResultSets();

    mapInfluencedRows.clear();
    myList.clear();
    removedListOfIds.clear();

    if(queryImage  != currentImage && queryImage != nullptr){
        delete(queryImage);
        delete (currentImage);
        queryImage = nullptr;
        currentImage = nullptr;
    } else {
        if (queryImage != nullptr){
            delete queryImage;
            queryImage = nullptr;
        }
    }
    mapOidToNames.clear();
    mapNameToMask.clear();

    delete (ui);
}



void OberonViewer::on_btnZoomIn_clicked(){

    double scale = (double) (ui->lblCenter->pixmap()->width() / (400.0));

    if (scale < 5.0) {
        int size = (int) 600*scale;
        fillQueryCenter(currentImage, size);
        if (!currentImage->windowedPixels()){
            showWindowing();
        }
        ui->lblCenter->setMinimumWidth(size);
        ui->lblCenter->setMinimumHeight(size);
    }

    ui->gridLayout_3->removeWidget(ui->lblCenter);
    ui->gridLayout_3->addWidget(ui->lblCenter);
    ui->gridLayout_3->update();
    ui->scrollArea->update();

    if (currentImage->type() == Image::DICOM || currentImage->type() == Image::KRL){
        showWindowing();
    }
}


void OberonViewer::on_btnExit_clicked(){

    #if M_PROV
    {
        if (searchType != Util::CLUSTER_EXPANSION_SEARCH){
            if (diagnosisForm != nullptr){
                delete (diagnosisForm);
            }
            diagnosisForm = new FormDiagnosis(oqFileName, oqId, tableName, "origin: Final Diagnosis", webSocket, userId.toInt(), mapNameToMask[oqFileName]);
            connect(diagnosisForm, &FormDiagnosis::finished, this, &OberonViewer::state12);
            diagnosisForm->showFullScreen();
        } else {
            state12();
        }
    }
    #else
        state12();
    #endif
}


void OberonViewer::on_btnZoomOut_clicked(){

    double scale = (double) (ui->lblCenter->pixmap()->width() / (800.0));

    if (600*scale > 600) {
        uint32_t size = (uint32_t) 600*scale;
        ui->lblCenter->setPixmap(ui->lblCenter->pixmap()->scaled(size, size));
        //Re-size QLabelSize
        ui->lblCenter->setMinimumWidth(size);
        ui->lblCenter->setMinimumHeight(size);
    } else {
        ui->lblCenter->setPixmap(ui->lblCenter->pixmap()->scaled(600, 600));
        //Re-size QLabelSize
        ui->lblCenter->setMinimumWidth(600);
        ui->lblCenter->setMinimumHeight(600);
    }

    //Updates the QLabel on the QScrollArea
    ui->gridLayout_3->removeWidget(ui->lblCenter);
    ui->gridLayout_3->addWidget(ui->lblCenter);
    ui->gridLayout_3->update();
    ui->scrollArea->update();
}


void OberonViewer::on_btnPDF_clicked(){

    QString fileName = "dicom2pdf.pdf";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QPainter painter(&printer);
    painter.drawText(0, 0, "WebHigiia - Exported DICOM image.");
    painter.drawPixmap(0, 400, 8000, 8000, ui->lblCenter->pixmap()->scaled(8000, 8000));
    painter.end();

    QFile pdfFile(fileName);
    pdfFile.open(QIODevice::ReadOnly);
    QByteArray content = pdfFile.readAll();

    QFileDialog::saveFileContent(content, fileName);
}

void OberonViewer::adjustSliders(){

    if (currentImage->type() == Image::DICOM || currentImage->type() == Image::KRL){

        ui->sliderCenter->setEnabled(true);
        ui->sliderWidth->setEnabled(true);

        if (!ui->cbxWindowing->currentIndex()){
            ui->sliderCenter->setMaximum(currentImage->getWindowCenter() + 2000);
            ui->sliderCenter->setMinimum(currentImage->getWindowCenter() - 2000);
            ui->sliderCenter->setValue(currentImage->getWindowCenter());
            ui->sliderCenter->setPageStep(30);
            ui->sliderCenter->setSingleStep(30);

            ui->sliderWidth->setMaximum(currentImage->getWindowWidth() + 2000);
            ui->sliderWidth->setMinimum(currentImage->getWindowWidth() - 2000);
            ui->sliderWidth->setValue(currentImage->getWindowWidth());
            ui->sliderWidth->setPageStep(30);
            ui->sliderWidth->setSingleStep(30);
        } else {
            ui->sliderCenter->setMaximum(WindowingInterval::getCenter((uint8_t) ui->cbxWindowing->currentIndex()) +
                                         WindowingInterval::getMargin((uint8_t) ui->cbxWindowing->currentIndex()));
            ui->sliderCenter->setMinimum(WindowingInterval::getCenter((uint8_t) ui->cbxWindowing->currentIndex()) -
                                         WindowingInterval::getMargin((uint8_t) ui->cbxWindowing->currentIndex()));
            ui->sliderCenter->setValue(WindowingInterval::getCenter((uint8_t) ui->cbxWindowing->currentIndex()));
            ui->sliderCenter->setPageStep(WindowingInterval::getStep((uint8_t) ui->cbxWindowing->currentIndex()));
            ui->sliderCenter->setSingleStep(WindowingInterval::getStep((uint8_t) ui->cbxWindowing->currentIndex()));

            ui->sliderWidth->setMaximum(WindowingInterval::getWidth((uint8_t) ui->cbxWindowing->currentIndex()) +
                                         WindowingInterval::getMargin((uint8_t) ui->cbxWindowing->currentIndex()));
            ui->sliderWidth->setMinimum(WindowingInterval::getWidth((uint8_t) ui->cbxWindowing->currentIndex()) -
                                         WindowingInterval::getMargin((uint8_t) ui->cbxWindowing->currentIndex()));
            ui->sliderWidth->setValue(WindowingInterval::getWidth((uint8_t) ui->cbxWindowing->currentIndex()));
            ui->sliderWidth->setPageStep(WindowingInterval::getStep((uint8_t) ui->cbxWindowing->currentIndex()));
            ui->sliderWidth->setSingleStep(WindowingInterval::getStep((uint8_t) ui->cbxWindowing->currentIndex()));
        }

    } else {
        ui->sliderCenter->setEnabled(false);
        ui->sliderWidth->setEnabled(false);
    }
}

void OberonViewer::showWindowing(){

    Image *i = currentImage->windowing(ui->sliderWidth->value(), ui->sliderCenter->value());
    QImage *img;

    img = Util::convertImageToQImage(i);
    ui->lblCenter->setPixmap(QPixmap::fromImage(img->scaled(ui->lblCenter->pixmap()->width(), ui->lblCenter->pixmap()->height())));

    delete (i);
    delete (img);
}

void OberonViewer::clearAllThumbnails(){

    for (size_t x = 0; x < thumbsImagesDown.size(); x++){
        QPushButton *tmp = thumbsImagesDown[x];
        ui->lytDiversity->removeItem(ui->lytDiversity->itemAt(ui->lytDiversity->indexOf(tmp)));
        delete (tmp);
    }
    thumbsImagesDown.clear();

    for (size_t x = 0; x < thumbsImagesRight.size(); x++){
        QPushButton *tmp = thumbsImagesRight[x];
        ui->gridLayout_5->removeItem(ui->gridLayout_5->itemAt(ui->gridLayout_5->indexOf(tmp)));
        delete (tmp);
    }
    thumbsImagesRight.clear();

    for (size_t x = 0; x < thumbsImagesMyList.size(); x++){
        QPushButton *tmp = thumbsImagesMyList[x];
        ui->lytDiversity->removeItem(ui->lytMyList->itemAt(ui->lytMyList->indexOf(tmp)));
        delete (tmp);
    }
    thumbsImagesMyList.clear();
}

void OberonViewer::clearResultSets(){

    rSet.clear();
    tempRInfset.clear();
    rInfset.clear();
}

void OberonViewer::on_sliderCenter_sliderReleased(){

    on_sliderWidth_sliderReleased();
}


void OberonViewer::on_sliderWidth_sliderReleased(){

    showWindowing();
    ui->gridLayout_3->removeWidget(ui->lblCenter);
    ui->gridLayout_3->addWidget(ui->lblCenter);
    ui->gridLayout_3->update();
    ui->scrollArea->update();
}


void OberonViewer::on_btnQueryCenter_clicked(){

    if(currentImage != queryImage){
        delete currentImage;
    }

    currentImage = queryImage;
    adjustSliders();
    fillQueryCenter(currentImage);
    if (!currentImage->windowedPixels()){
        showWindowing();
    }
}



void OberonViewer::on_btnRestart_clicked(){

    ui->lblServerSetup->setText("Restarting, please wait ...");

    //Lock screen widgets...
    ui->tabWidget->hide();
    ui->lytCentral->setEnabled(false);

    //Clear current variables, except result sets ...
    clearAllThumbnails();
    myList.clear();
    removedListOfIds.clear();

    if(queryImage  != currentImage && queryImage != nullptr){
        delete(queryImage);
        delete (currentImage);
        queryImage = nullptr;
        currentImage = nullptr;
    }else{
        if (queryImage != nullptr){
            delete queryImage;
            queryImage = nullptr;
        }
    }

    fillThumbnails();

    //Unlock screen
    ui->lytCentral->setEnabled(true);
    ui->lblServerSetup->setText("CBIR is ready!");
}


void OberonViewer::on_btnFeedback_clicked(){

    if (ui->txtKnn->text().isEmpty()){
        ui->lblServerSetup->setText("Invalid search parameters!");
        return;
    }

    #if M_PROV
    {
        if (diagnosisForm != nullptr){
            delete (diagnosisForm);
        }
        if (searchType == Util::CLUSTER_EXPANSION_SEARCH){
            diagnosisForm = new FormDiagnosis(oqFileName, oqId, tableName, "origin: Before Cluster Feedback Cycle", webSocket, userId.toInt(), mapNameToMask[oqFileName]);
        } else {
            diagnosisForm = new FormDiagnosis(oqFileName, oqId, tableName, "origin: Before Feedback Cycle", webSocket, userId.toInt(), mapNameToMask[oqFileName]);
        }
        connect(diagnosisForm, &FormDiagnosis::finished, this, &OberonViewer::state11);
        diagnosisForm->showFullScreen();
    }
    #else
        state11();
    #endif
}


void OberonViewer::state06(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state06);

    MedicalImageTable tempSet(Util::toStringList(message.split('\n')));
    FeatureVector newOq, oi;
    float sumRelevants, sumNotRelevants, scaleFactor;
    QString queryObjectValue, condition, tblName;
    SirenSQLQuery queryT;


    if (!tempSet.size()){
        //Unlock screen
        ui->lytCentral->setEnabled(true);
        ui->lblServerSetup->setText("Empty result set!");
        return;
    }


    int pos = tempSet.locateColumn(simAttribute);
    sumRelevants = sumNotRelevants = 0;
    for (int y = 0; y < tempSet.size(); y++){
        if (tempSet.fetchByColumnId(y, ((pos+1)%2)) == "0"){
            sumRelevants += 1.0;
        } else {
            sumNotRelevants += 1.0;
        }
    }
    scaleFactor = 1.0;
    if (sumRelevants > 0.0){
        scaleFactor += 1.0;
    }
    if (sumNotRelevants > 0.0){
        scaleFactor += 0.5;
    }

    //Rocchio-Method alpha = 1, beta = 1/2
    newOq.setOID(0);
    for (size_t x = 0; x < oq.size(); x++){
        newOq.add(oq[x]);
        for (int y = 0; y < tempSet.size(); y++){
            oi.unserializeFromString(FeatureVector::fromBase64(tempSet.fetchByColumnId(y, pos).toStdString()));
            if (tempSet.fetchByColumnId(y, ((pos+1)%2)) == "0"){
                //relevantes
                newOq[x] += (oi[x] / sumRelevants);
            } else {
                //nao relevantes
                newOq[x] -= (oi[x] / (2.0*sumNotRelevants));
            }
        }
        newOq[x] /= scaleFactor;
    }

    //Builds new query center
    queryObjectValue = " { ";
    for (size_t x = 0; x < newOq.size(); x++){
        if (x > 0)
            queryObjectValue += ", ";
        queryObjectValue += QString::number(*newOq.get(x));
    }
    queryObjectValue += " }";

    if (vTable){
        tblName = "temp";
    } else {
        tblName = tableName;
    }

    //Builds query
    QStringList scope = scopeAttributes.split(",");
    for (int x = 0; x < scope.size(); x++){
        queryT.addProjectionAttribute(scope.at(x).simplified());
    }
    queryT.addProjectionList({tblName + ".Filename Filename", tblName + ".Id Id"});
    if (vTable){
        queryT.addTable("( " + vTableName + " ) AS temp ");
    } else {
        queryT.addTable(tblName);
    }
    condition += tblName +"."+ simAttribute;
    if (searchType == Util::SIMILARITY_SEARCH || searchType == Util::CLUSTER_EXPANSION_SEARCH ){
        condition += " NEAR ";
    } else {
        if (searchType == Util::DIVERSITY_SEARCH){
            condition += " DIVERSITY NEAR ";
        } else {
            condition += " DIVERSIFIED NEAR ";
        }
    }
    condition += queryObjectValue;
    condition += " BY "+ metricName + " STOP AFTER " + ui->txtKnn->text();
    if (searchType == Util::BRIDGE_SEARCH){
        condition += " BRIDGE " + kBridge;
    }
    queryT.addWhereAttribute(condition);
    queryT.addOrderByAttribute(tblName + ".Id");

    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state07);
    webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
}

void OberonViewer::state07(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state07);

    //Clear current variables, including result sets ...
    ui->lblServerSetup->setText("Rebuilding, please wait ...");
    ui->lblDetails->clear();
    clearAllThumbnails();
    clearResultSets();
    mapInfluencedRows.clear();
    myList.clear();
    removedListOfIds.clear();
    mapOidToNames.clear();


    if(queryImage  != currentImage && queryImage != nullptr){
        delete(queryImage);
        delete (currentImage);
        queryImage = nullptr;
        currentImage = nullptr;
    } else {
        if (queryImage != nullptr){
            delete queryImage;
            queryImage = nullptr;
        }
    }

    rSet.clear();
    rInfset.clear();
    tempRInfset.clear();

    rSet = MedicalImageTable(Util::toStringList(message.split('\n')));
    splitResultSets();
}

void OberonViewer::state08(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state08);

    ResultTable masks(Util::toStringList(message.split('\n')));

    if (masks.size()){
        mapNameToMask[oqFileName] = masks.fetchByColumnId(0, masks.locateColumn("Mask"));
    }

    downloadOq();
}

void OberonViewer::state09(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state09);

    int posId, posMask;
    ResultTable masks(Util::toStringList(message.split('\n')));

    if (masks.size()){
        posId = masks.locateColumn("Id");
        posMask = masks.locateColumn("Mask");

        //Load and map image masks
        for (int x = 0; x < masks.size(); x++){
            QString filename = rSet.fetchByColumnId(rSet.locateFirstRow(masks.fetchByColumnId(x, posId), rSet.locateImageID()), rSet.locateImageFilename());
            mapNameToMask[filename] = masks.fetchByColumnId(x, posMask);
        }
    }
    downloadNonInfluencedImages();
}

void OberonViewer::state10(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state10);

    int posId, posMask;
    ResultTable masks(Util::toStringList(message.split('\n')));

    if (masks.size()){
        posId = masks.locateColumn("Id");
        posMask = masks.locateColumn("Mask");

        //Load and map image masks
        for (int x = 0; x < masks.size(); x++){
            QString filename = rInfset.fetchByColumnId(rInfset.locateFirstRow(masks.fetchByColumnId(x, posId), rInfset.locateImageID()), rInfset.locateImageFilename());
            mapNameToMask[filename] = masks.fetchByColumnId(x, posMask);
        }
    }
    downloadInfluencedImages();
}

void OberonViewer::state11(){

    #if M_PROV
    {
        disconnect(diagnosisForm, &FormDiagnosis::finished, this, &OberonViewer::state11);
        if (diagnosisForm != nullptr){
            delete(diagnosisForm);
            diagnosisForm = nullptr;
        }
    }
    #endif

    std::vector<uint32_t> relevants;
    QString tblName, condition, sqlQuery;
    SirenSQLQuery buildRelevants, buildNonRelevants;

    for (int x = 0; x < ui->gridLayout_5->count(); x++){
        QLayoutItem * const item = ui->gridLayout_5->itemAt(x);
        if (item->widget()->windowTitle().toStdString() == "1"){
            relevants.push_back(item->widget()->whatsThis().toUInt());
        }
    }

    if (relevants.empty() && removedListOfIds.empty()){
        ui->lblServerSetup->setText("Invalid search parameters!");
        return;
    }

    ui->lblServerSetup->setText("Cycling, please wait...");

    //Lock screen widgets...
    ui->tabWidget->hide();
    ui->lytCentral->setEnabled(false);

    if (vTable){
        tblName = "temp";
    } else {
        tblName = tableName;
    }
    if (!relevants.empty()){
        buildRelevants.addProjectionList( {"rf$PPV$" + tableName + "_"+ simAttribute + "."+ simAttribute, "'0' "});
        if (vTable){
            buildRelevants.addTable("( " + vTableName + " ) AS " + tblName);
        } else {
            buildRelevants.addTable(tableName);
        }
        buildRelevants.addJoin("PPV$" + tableName + "_"+ simAttribute + " AS rf$PPV$" + tableName + "_"+ simAttribute, "rf$PPV$" + tableName + "_"+ simAttribute + "." + simAttribute + "_id = " + tblName + "." + simAttribute);
        condition = tblName + ".Id IN ( ";
        for (size_t x = 0; x < relevants.size(); x++){
            if (x > 0){
                condition += ", ";
            }
            condition += QString::number(relevants[x]);
        }
        condition += ") ";
        buildRelevants.addWhereAttribute(condition);
        sqlQuery = buildRelevants.generateQuery();
    }

    if (!removedListOfIds.empty() && !relevants.empty()){
        sqlQuery += "UNION ";
    }

    if (!removedListOfIds.empty()){
        buildNonRelevants.addProjectionList( {"rf$PPV$" + tableName + "_"+ simAttribute + "."+ simAttribute, "'1' "});
        if (vTable){
            buildNonRelevants.addTable("( " + vTableName + " ) AS " + tblName);
        } else {
            buildNonRelevants.addTable(tableName);
        }
        buildNonRelevants.addJoin("PPV$" + tableName + "_"+ simAttribute + " AS rf$PPV$" + tableName + "_"+ simAttribute, "rf$PPV$" + tableName + "_"+ simAttribute + "." + simAttribute + "_id = " + tblName + "." + simAttribute);
        condition = tblName + ".Id IN ( ";
        for (size_t x = 0; x < removedListOfIds.size(); x++){
            if (x > 0){
                condition += ", ";
            }
            condition += QString::number(removedListOfIds[x]);
        }
        condition += ")";
        buildNonRelevants.addWhereAttribute(condition);
        sqlQuery = buildNonRelevants.generateQuery();
    }
    relevants.clear();

    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &OberonViewer::state06);
    webSocket->sendBinaryMessage(sqlQuery.toStdString().c_str());
}

void OberonViewer::state12(){

    #if M_PROV
    {
        if (searchType != Util::CLUSTER_EXPANSION_SEARCH){
            disconnect(diagnosisForm, &FormDiagnosis::finished, this, &OberonViewer::state12);
            if (diagnosisForm != nullptr){
                delete(diagnosisForm);
                diagnosisForm = nullptr;
            }
        }
    }
    #endif

    emit finished();
    this->close();
}

void OberonViewer::on_cbxWindowing_currentIndexChanged(int index){

    adjustSliders();
    showWindowing();
}

void OberonViewer::on_btnPACS_clicked(){

    QDesktopServices::openUrl(QUrl(PACS_BASE_URL + link, QUrl::TolerantMode));
}


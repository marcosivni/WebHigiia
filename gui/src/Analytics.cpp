#include "Analytics.h"
#include "ui_Analytics.h"

Analytics::Analytics(QString tableName,
                     QString queryCenterName,
                     FeatureVector oq,
                     QString simAttribute,
                     QString metricName,
                     MedicalImageTable rSet,
                     bool vTable,
                     QString vTableName,
                     QWebSocket *webSocket,
                     int32_t oqId,
                     int32_t userId,
                     QString link,
                     QString numberK,
                     QString clusterSize,
                     QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Analytics){

    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);

    QDesktopWidget *desktop = QApplication::desktop();

    uint16_t screenWidth, width;
    uint16_t screenHeight, height;
    uint16_t x, y;
    QSize windowSize;

    screenWidth = desktop->width();
    screenHeight = desktop->height();

    windowSize = size();
    width = windowSize.width();
    height = windowSize.height();

    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;

    move ( x, y );

    #if M_PROV
        diagnosisForm = nullptr;
    #endif

    this->tableName = tableName;
    this->webSocket = webSocket;
    this->rSet = rSet;
    this->oqFileName = queryCenterName;
    this->simAttribute = simAttribute;
    this->oq = oq;
    this->metricName = metricName;
    this->vTable = vTable;
    this->vTableName = vTableName;
    this->oqId = oqId;
    this->userId = userId;
    this->link = link;
    this->numberK = numberK;
    this->clusterSize = clusterSize;
    searchForm = nullptr;

    seriesByTarget = nullptr;
    chart = nullptr;

    //Lock widgets
    ui->gbxOq->hide();
    ui->gbxOi->hide();
    ui->panelButtons->hide();
    ui->panelMining->hide();

    ui->lblStatus->setText("Loading scope values, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state01);
    webSocket->sendBinaryMessage(scopeCaption.generateSQLBaseQuery(tableName).toStdString().c_str());
}

void Analytics::lockWidgets(){

    ui->btnClose->setEnabled(false);
    ui->btnOi->setEnabled(false);
    ui->btnOq->setEnabled(false);
    ui->btnPACS->setEnabled(false);
    ui->btnScopeOi->setEnabled(false);
    ui->btnScopeOq->setEnabled(false);
    ui->btnSearchOi->setEnabled(false);
    ui->btnStatsOq->setEnabled(false);
    ui->btnStatsOi->setEnabled(false);
    ui->btnStatsOq->setEnabled(false);
    ui->btnUpdate->setEnabled(false);
}

void Analytics::unlockWidgets(){

    ui->btnClose->setEnabled(true);
    ui->btnOi->setEnabled(true);
    ui->btnOq->setEnabled(true);
    ui->btnPACS->setEnabled(true);
    ui->btnScopeOi->setEnabled(true);
    ui->btnScopeOq->setEnabled(true);
    ui->btnSearchOi->setEnabled(true);
    ui->btnStatsOq->setEnabled(true);
    ui->btnStatsOi->setEnabled(true);
    ui->btnStatsOq->setEnabled(true);
    ui->btnUpdate->setEnabled(true);
}

Analytics::~Analytics(){

    if (searchForm != nullptr){
        delete (searchForm);
    }
    searchForm = nullptr;

    if (seriesByTarget != nullptr) {
        for (int i = 0; i < nSeries; i++) {
            delete (seriesByTarget[i]);
        }
        delete[] seriesByTarget;
        seriesByTarget = nullptr;
    }
    if (chart != nullptr){
        delete (chart);
        chart = nullptr;
    }

    rSet.clear();
    rInfset.clear();

    mapInfluencedRows.clear();
    mapPointToRowId.clear();
    dataset2D.clear();

    mapNameToMask.clear();

    delete ui;
}

void Analytics::state01(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state01);

    int posAtt, posValue, posCaption;
    ResultTable scopeV(Util::toStringList(message.split('\n')));

    posAtt = scopeV.locateColumn("attributeN");
    posValue= scopeV.locateColumn("valueN");
    posCaption= scopeV.locateColumn("caption");

    //Load caption for user-defined scope...
    for (int x = 0; x < scopeV.size(); x++){
        scopeCaption.addEntry(scopeV.fetchByColumnId(x, posAtt), scopeV.fetchByColumnId(x, posValue), scopeV.fetchByColumnId(x, posCaption));
    }

    loadScope();
}

void Analytics::loadScope(){

    ui->lblStatus->setText("Loading scope, please wait ...");

    SirenSQLQuery buildScope;
    buildScope.addProjectionAttribute("attributeName");
    buildScope.addTable("Scope");
    buildScope.addWhereAttribute("tableName = '" + tableName + "'");

    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state02);
    webSocket->sendBinaryMessage(buildScope.generateQuery().toStdString().c_str());
}

void Analytics::state02(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state02);
    ResultTable scope(Util::toStringList(message.split('\n')));

    scopeAtts.clear();
    scopeAtts.append("Image_Class");
    if (scope.size()){
        for (int x = 0; x < scope.size(); x++){
            if (scope.fetchByColumnId(x, 0).simplified().toUpper() != "IMAGE_CLASS"){
                if (scope.fetchByColumnId(x, 0).simplified().toUpper() != "URL"){
                    ui->cbxTarget->addItem(scope.fetchByColumnId(x, 0));
                }
                scopeAtts.append(scope.fetchByColumnId(x, 0));
            }
        }
    }
    loadInfluencedImages();
}

void Analytics::loadInfluencedImages(){

    ui->lblStatus->setText("Loading result set, please wait ...");
    if (!rSet.size()){
        ui->lblStatus->setText("Empty result set!");
        return ;
    }

    //Separate influencers and influenced
    FeatureVector oj;
    SirenSQLQuery queryT;
    int colId, colInfluenced;

    tempRInfset.clear();
    newRSet.addCaption(rSet.fetchCaption());
    tempRInfset.addCaption(rSet.fetchCaption());
    colId = rSet.locateImageID();
    colInfluenced = rSet.locateColumn(simAttribute + "$bridged");

    iteratorRSet = 0;
    if (iteratorRSet < rSet.size()){
        queryT.addProjectionAttribute("temp." + simAttribute + " " + simAttribute);
        queryT.addTable(tableName);
        queryT.addJoin("PPV$" + tableName + "_"+ simAttribute + " AS temp ", "temp." + simAttribute + "_id = " + tableName + "." + simAttribute);
        queryT.addWhereAttribute(tableName + ".Id = " + rSet.fetchByColumnId(iteratorRSet, colId));

        //Blocking call - State machine
        ui->lblStatus->setText("Fetching query data " + QString::number(iteratorRSet) + "/" + QString::number(rSet.size()) + ", please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state03);
        webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
    } else {
        rSet.clear();
        rSet = newRSet;

        //Requested data from first influenced entry, then loop...
        counterImgs = 0;
        if (counterImgs < tempRInfset.size()){
            oj.unserializeFromString(FeatureVector::fromBase64(tempRInfset.fetchByColumnId(counterImgs, colInfluenced).toStdString()));
            queryT.addProjectionList( {"Id", "Filename"});
            for (int itS = 0; itS < scopeAtts.size(); itS++){
                queryT.addProjectionAttribute(tableName + "." + scopeAtts[itS]);
            }
            queryT.addTable(tableName);
            queryT.addWhereAttribute(simAttribute + " = " + QString::number(oj.getOID()));

            ui->lblStatus->setText("Fetching info " + QString::number(counterImgs) + "/" + QString::number(tempRInfset.size()) + ", please wait ...");
            connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state04);
            webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
        } else {
            donwloadNonInfluencedMasks();
        }
    }
}

void Analytics::state03(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state03);

    MedicalImageTable tuples(Util::toStringList(message.split('\n')));
    SirenSQLQuery queryT;
    FeatureVector oj;

    int colId, colInfluenced, colFV;
    colInfluenced = rSet.locateColumn(simAttribute + "$bridged");
    colId = rSet.locateImageID();
    colFV = tuples.locateColumn(simAttribute);

    if ((tuples.size()) && (tuples.fetchByColumnId(0, colFV) == rSet.fetchByColumnId(iteratorRSet, colInfluenced))){
        newRSet.addTuple(rSet.fetchTupleByRowId(iteratorRSet));
    }
    tempRInfset.addTuple(rSet.fetchTupleByRowId(iteratorRSet));

    iteratorRSet++;
    if (iteratorRSet < rSet.size()){
        queryT.addProjectionAttribute("temp." + simAttribute + " " + simAttribute);
        queryT.addTable(tableName);
        queryT.addJoin("PPV$" + tableName + "_"+ simAttribute + " AS temp ", "temp." + simAttribute + "_id = " + tableName + "." + simAttribute);
        queryT.addWhereAttribute(tableName + ".Id = " + rSet.fetchByColumnId(iteratorRSet, colId));

        //Blocking call - State machine
        ui->lblStatus->setText("Fetching query data " + QString::number(iteratorRSet) + "/" + QString::number(rSet.size()) + ", please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state03);
        webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
    } else {
        rSet.clear();
        rSet = newRSet;

        //Requested data from first influenced entry, then loop...
        counterImgs = 0;
        if (counterImgs < tempRInfset.size()){
            oj.unserializeFromString(FeatureVector::fromBase64(tempRInfset.fetchByColumnId(counterImgs, colInfluenced).toStdString()));
            queryT.addProjectionList( {"Id", "Filename"});
            for (int itS = 0; itS < scopeAtts.size(); itS++){
                queryT.addProjectionAttribute(tableName + "." + scopeAtts[itS]);
            }
            queryT.addTable(tableName);
            queryT.addWhereAttribute(simAttribute + " = " + QString::number(oj.getOID()));

            ui->lblStatus->setText("Fetching info " + QString::number(counterImgs) + "/" + QString::number(tempRInfset.size()) + ", please wait ...");
            connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state04);
            webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
        } else {
            donwloadNonInfluencedMasks();
        }
    }
}

void Analytics::donwloadNonInfluencedMasks(){

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

    ui->lblStatus->setText("Loading masks, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state07);
    webSocket->sendBinaryMessage(buildMaskStatement.generateQuery().toStdString().c_str());
}

void Analytics::state04(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state04);

    FeatureVector oj;
    int posIdTempSet, posIdInfSet, posIdRSet, colInfluenced;
    MedicalImageTable tuples(Util::toStringList(message.split('\n')));
    SirenSQLQuery queryT;

    if (!rInfset.fetchCaption().size()){
        rInfset.addCaption(tuples.fetchCaption());
    }

    posIdTempSet = tempRInfset.locateImageID();
    posIdInfSet = rInfset.locateImageID();
    posIdRSet = rSet.locateImageID();
    colInfluenced = tempRInfset.locateColumn(simAttribute + "$bridged");

    QString key = tempRInfset.fetchByColumnId(counterImgs, posIdTempSet);
    uint16_t pos = rSet.locateFirstRow(key, posIdRSet);

    if (tuples.fetchByColumnId(0, posIdInfSet) != rSet.fetchByColumnId(pos, posIdRSet)){
        rInfset.addTuple(tuples.fetchTupleByRowId(0));
        mapInfluencedRows.insert(pos, rInfset.size()-1);
    }

    counterImgs++;
    if (counterImgs < tempRInfset.size()){
        oj.unserializeFromString(FeatureVector::fromBase64(tempRInfset.fetchByColumnId(counterImgs, colInfluenced).toStdString()));
        queryT.addProjectionList( {"Id", "Filename"});
        for (int itS = 0; itS < scopeAtts.size(); itS++){
            queryT.addProjectionAttribute(tableName + "." + scopeAtts[itS]);
        }
        queryT.addTable(tableName);
        queryT.addWhereAttribute(simAttribute + " = " + QString::number(oj.getOID()));

        ui->lblStatus->setText("Fetching info " + QString::number(counterImgs) + "/" + QString::number(tempRInfset.size()) + ", please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state04);
        webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
    } else {
        donwloadNonInfluencedMasks();
    }
}

void Analytics::downloadNonInfluencedImages(){

    int posImg;
    counterImgs = 0;

    tempRInfset.clear();
    newRSet.clear();

    posImg = rSet.locateImageFilename();

    while ((counterImgs < rSet.size()) &&
           (QFileInfo::exists(WFS_NAME + rSet.fetchByColumnId(counterImgs, posImg))
            && QFileInfo(WFS_NAME + rSet.fetchByColumnId(counterImgs, posImg)).isFile())){
        counterImgs++;
    }

    if (counterImgs < rSet.size()){
        QString request = "REQUEST " + rSet.fetchByColumnId(counterImgs, posImg);
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state05);
        ui->lblStatus->setText("Downloading " + QString::number(counterImgs) + "/" + QString::number(rSet.size())+ ", please wait ...");
        webSocket->sendBinaryMessage(request.toStdString().c_str());
    } else {
        downloadOqMask();
    }
}

void Analytics::state05(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state05);

    int posImg;
    QString filename;

    posImg = rSet.locateImageFilename();
    filename = rSet.fetchByColumnId(counterImgs, posImg);

    if (!filename.toStdString().empty()){
        Util::saveImageAndThumbnailToFS(filename, message, QSize(300,300), mapNameToMask[filename]);
    } else {
        ui->lblStatus->setText("Local storage fatal error.");
    }

    counterImgs++;

    while ((counterImgs < rSet.size()) &&
           (QFileInfo::exists(WFS_NAME + rSet.fetchByColumnId(counterImgs, posImg))
            && QFileInfo(WFS_NAME + rSet.fetchByColumnId(counterImgs, posImg)).isFile())){
        counterImgs++;
    }

    if (counterImgs < rSet.size()){
        QString request = "REQUEST " + rSet.fetchByColumnId(counterImgs, posImg);
        ui->lblStatus->setText("Downloading " + QString::number(counterImgs) + "/" + QString::number(rSet.size())+ ", please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state05);
        webSocket->sendBinaryMessage(request.toStdString().c_str());
    } else {
        downloadOqMask();
    }
}

void Analytics::downloadOqMask(){

    SirenSQLQuery buildMaskStatement;
    buildMaskStatement.addProjectionAttribute("Id, Mask");
    buildMaskStatement.addTable("U_" + tableName);
    buildMaskStatement.addWhereAttribute("Id = " + QString::number(oqId));

    ui->lblStatus->setText("Loading masks, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state08);
    webSocket->sendBinaryMessage(buildMaskStatement.generateQuery().toStdString().c_str());
}

void Analytics::downloadOq(){


    ui->lblStatus->setText("Downloading query image, please wait ...");
    if (QFileInfo::exists(WFS_NAME + oqFileName.simplified() )
            && QFileInfo(WFS_NAME + oqFileName.simplified()).isFile()){

        dataset2D = pca2D();
        fillQueryData();
        populateData();

        //Predicting query image scope
        ui->lblScopeOq->setText(buildOqScope());
        ui->lblStatsOq->setText(buildOqStats());

        //Unlocking main screen...
        ui->gbxOq->show();
        ui->panelButtons->show();
        ui->panelMining->show();
        ui->lblStatus->setText("CBIR is ready!");
        unlockWidgets();
    } else {
        QString request = "REQUEST " + oqFileName.simplified();
        ui->lblStatus->setText("Saving... Building PCA, please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state06);
        webSocket->sendBinaryMessage(request.toStdString().c_str());
    }
}

void Analytics::state06(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state06);

    if (!oqFileName.toStdString().empty()){
        Util::saveImageAndThumbnailToFS(oqFileName, message, QSize(300,300), mapNameToMask[oqFileName]);
    } else {
        ui->lblStatus->setText("Local storage fatal error.");
    }

    dataset2D = pca2D();

    fillQueryData();
    populateData();

    //Predicting query image scope
    ui->lblScopeOq->setText(buildOqScope());
    ui->lblStatsOq->setText(buildOqStats());

    //Unlocking main screen...
    ui->gbxOq->show();
    ui->panelButtons->show();
    ui->panelMining->show();
    ui->lblStatus->setText("CBIR is ready!");
    unlockWidgets();

}

void Analytics::state07(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state07);

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

void Analytics::state08(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Analytics::state08);

    ResultTable masks(Util::toStringList(message.split('\n')));

    if (masks.size()){
        mapNameToMask[oqFileName] = masks.fetchByColumnId(0, masks.locateColumn("Mask"));
    }

    downloadOq();
}

void Analytics::fillQueryData(){

    QSize icoSize(295, 295);
    QSize btnSize(300, 300);

    Image *i = Util::openThumbnail(oqFileName);
    QImage *img;
    QIcon ico;
    img = Util::convertImageToQImage(i);
    ico.addPixmap(QPixmap::fromImage(img->scaled(btnSize)));
    delete (img);
    delete (i);

    ui->btnOq->setIcon(ico);
    ui->btnOq->setIconSize(icoSize);
    ui->btnOq->setFlat(true);
    ui->btnOq->resize(btnSize);
}

void Analytics::fillNeighborImage(int rSetRowId){

    QSize icoSize(295, 295);
    QSize btnSize(300, 300);
    int colId = rSet.locateImageFilename();

    Image *i = Util::openThumbnail(rSet.fetchByColumnId(rSetRowId, colId));
    QImage *img;
    QIcon ico;

    img = Util::convertImageToQImage(i);
    ico.addPixmap(QPixmap::fromImage(img->scaled(btnSize)));
    delete (img);
    delete (i);

    ui->btnOi->setIcon(ico);
    ui->btnOi->setIconSize(icoSize);
    ui->btnOi->setFlat(true);
    ui->btnOi->resize(btnSize);

    ui->btnOi->setWhatsThis(QString::number(rSetRowId));
}

void Analytics::populateData(){

    on_btnUpdate_clicked();
}

double Analytics::L2ToOqPCA(QPointF p1){

    double answer = 0.0;
    answer = qSqrt((p1.x() - oqRed[0]) * (p1.x() - oqRed[0]) + (p1.y() - oqRed[1]) * (p1.y() - oqRed[1]));
    return answer;
}

void Analytics::unitaryScale(FeatureVectorList *dataset){

    if (!dataset->size()){
        return;
    }

    FeatureVector minD, maxD;
    minD = dataset->at(0);
    maxD = dataset->at(0);

    for (size_t x = 0; x < minD.size(); x++){
        minD[x] = std::numeric_limits<double>::max();
        maxD[x] = std::numeric_limits<double>::min();
    }


    for (size_t x = 0; x < dataset->size(); x++){
        for (size_t y = 0; y < minD.size(); y++){
            if (dataset->at(x)[y] < minD[y]){
                minD[y] = dataset->at(x)[y];
            }
            if (dataset->at(x)[y] > maxD[y]){
                maxD[y] = dataset->at(x)[y];
            }
        }
    }

    for (size_t x = 0; x < dataset->size(); x++){
        for (size_t y = 0; y < minD.size(); y++){
            if  ((maxD[y] - minD[y]) > 0){
                dataset->at(x)[y] =  ((double) (dataset->at(x)[y] - minD[y])/ (double) (maxD[y] - minD[y]));
            }
        }
    }
}

FeatureVectorList Analytics::pca2D(){

    FeatureVectorList dataset, redDataset;
    FeatureVector oi;

    int colInfluenced = rSet.locateColumn(simAttribute + "$bridged");

    //Fast PCA - rSet entries only
    for (int x = 0; x < rSet.size(); x++){
        oi.unserializeFromString(FeatureVector::fromBase64(rSet.fetchByColumnId(x, colInfluenced).toStdString()));
        dataset.push_back(oi);
    }
    //Insert oq at the end
    dataset.push_back(oq);

    size_t cardinality = dataset.size();
    size_t dimensionality = oi.size();

    if (dimensionality > 2){
        //[0-1] scale
        unitaryScale(&dataset);

        Eigen::VectorXd means = Eigen::VectorXd::Zero(dimensionality);
        Eigen::MatrixXd mcov(dimensionality,dimensionality);

        for(size_t j = 0; j < dimensionality; j++){
            for(size_t i = 0; i < cardinality; i++){
                means(j) += (dataset[i][j] / ((double) cardinality));
            }
        }

        for(size_t i = 0; i < cardinality; i++){
            for(size_t j = 0; j < dimensionality; j++){
                dataset[i][j] -= means[j];
            }
        }

        means = Eigen::VectorXd::Zero(dimensionality);
        for(size_t j = 0; j < dimensionality; j++){
            for(size_t i = 0; i < cardinality; i++){
                means(j) += (dataset[i][j] / ((double) cardinality));
            }
        }

        for(size_t k = 0; k < dimensionality; k++){
            for(size_t j = 0; j < dimensionality; j++){
                double sum = 0.0;
                for(size_t i = 0; i < cardinality; i++){
                    sum += (dataset[i][j] - means[j])*(dataset[i][k] - means[k]);
                }
                sum /= ((double) cardinality - 1.0);
                mcov(k, j) = sum;
            }
        }

        Eigen::EigenSolver<Eigen::MatrixXd> es(mcov);
        Eigen::MatrixXd eigenvectors(dimensionality, dimensionality);
        Eigen::MatrixXd sorted_eigenvectors(dimensionality, dimensionality);

        for(size_t k = 0; k < dimensionality;k++){
            for(size_t p = 0; p < dimensionality;p++){
                eigenvectors(k,p) = es.eigenvectors().col(p)[k].real();
            }
        }

        // Sort by ascending eigenvalues
        std::vector<std::pair<double, size_t> > sortEigen;
        sortEigen.reserve(dimensionality);
        for (size_t x = 0; x < dimensionality; x++){
            double aux = (double) es.eigenvalues().real().coeff(x, 0);
            std::pair<double, size_t> pairEigenIndex(aux, x);
            sortEigen.push_back(pairEigenIndex);
        }
        std::sort(sortEigen.begin(), sortEigen.end());

        for (size_t x = 0; x < sortEigen.size(); x++){
            sorted_eigenvectors.col(x) = eigenvectors.col(sortEigen[x].second);
        }

        //Reduce data
        oi.resize(2);
        for (size_t z = 0; z < 2; z++){
            oi[z] = 0.0;
        }
        for (size_t x = 0; x < cardinality; x++){
            oi.setOID(dataset.at(x).GetOID());
            redDataset.push_back(oi);
        }


        for (size_t y = dimensionality-1; y >= dimensionality-2; y--){
            for (size_t x = 0; x < cardinality; x++){
                for (size_t z = 0; z < dimensionality; z++){
                    redDataset[x][dimensionality-y-1] += dataset[x][z]*sorted_eigenvectors.col(y)[z];
                }
            }
        }
    } else {
        redDataset = dataset;
    }
    //PCA-Whitening
    unitaryScale(&redDataset);

    dataset.clear();
    //Remove oq
    oqRed = redDataset.back();
    redDataset.pop_back();

    return redDataset;
}

void Analytics::handleClickedPoint(const QPointF &point){

    QPointF clickedPoint = point;
    QPointF closest(INT_MAX, INT_MAX);
    qreal distance(INT_MAX);

    // Find the closest point of any series
    for (int x = 1; x < nSeries; x++){
        const auto points = seriesByTarget[x]->points();
        for (const QPointF &currentPoint : points) {
            qreal currentDistance = qSqrt((currentPoint.x() - clickedPoint.x())
                                          * (currentPoint.x() - clickedPoint.x())
                                          + (currentPoint.y() - clickedPoint.y())
                                          * (currentPoint.y() - clickedPoint.y()));
            if (currentDistance < distance) {
                distance = currentDistance;
                closest = currentPoint;
            }
        }
    }
    if (ui->gbxOi->isHidden() || currentClosest != closest){
        currentClosest = closest;
        for (size_t x = 0; x < mapPointToRowId.size(); x++){
            if (mapPointToRowId[x].second == closest){
                //Populate Info
                fillNeighborImage(mapPointToRowId[x].first);
                ui->lblScopeOi->setText(buildNeighborScope(rSet, scopeAtts, mapPointToRowId[x].first));
                ui->lblStatsOi->setText(buildNeighborStats(mapPointToRowId[x].first, L2ToOqPCA(closest)));
                break;
            }
        }
        ui->gbxOi->show();
    } else {
        ui->gbxOi->hide();
    }

}


QString Analytics::buildOqScope(){

    QString answer, prediction;
    std::string caption;
    int posScope;
    double maxFrequency;

    //For each scope attribute...
    for (int x = 0; x < 1; x++){

        posScope = Util::findColumn(scopeAtts[x], rSet.fetchCaption());
        prediction = Util::mostFrequentValue(rSet.fetchColumn(posScope));
        maxFrequency = Util::highestFrequencyPercent(rSet.fetchColumn(posScope));

        if (answer.size()){
            answer += "\n";
        } else {
            answer += "== Diversity-based Scope Prediction ==\n\n";
        }
        answer += scopeAtts[x].simplified() + ":\n  ";
        answer += prediction;
        if (scopeCaption.findCaption(scopeAtts[x], prediction, &caption)){
            answer += " = " + QString(caption.c_str());
        }
        answer += " ~" + QString::number(maxFrequency*100.0, 'f', 2) + "% similar cases";
    }

    return answer;
}

/// Stats
///
/// Query Image FV-ID: xx
/// Average distance to neighbors: xx.xx (PCA projection)
/// #Covered neighborhood:  xx elements
/// Average density: xx elements
/// Minimum cluster density: xx elements
/// Maximal cluster density: xx elements
///
QString Analytics::buildOqStats(){

    QString answer;
    std::string caption;
    double sumDist, sumPoints, avgC, minC, maxC;

    answer += "== Diversity-based distance stats ==\n\n";
    answer += "Query Image FV-ID:\n  ";
    answer += QString::number(oq.GetOID()) + "\n";

    sumDist = sumPoints = 0.0;
    for (int x = 1; x < nSeries; x++){
        const auto points = seriesByTarget[x]->points();
        for (const QPointF &currentPoint : points) {
            sumDist += L2ToOqPCA(currentPoint);
            sumPoints += 1.0;
        }
    }

    answer += "Average distance to neighbors:\n  ";
    answer += QString::number(sumDist/sumPoints, 'f', 2) + " (PCA projection)\n";
    answer += "#Covered neighborhood:\n  ";
    answer += QString::number(sumPoints, 'f', 0) + " elements\n";

    QList<uint16_t> keys = mapInfluencedRows.uniqueKeys();

    sumPoints = 0.0;
    for (int x = 1; x < nSeries; x++){
        sumPoints += seriesByTarget[x]->count();
    }
    if (!keys.size()) {
        maxC = minC = avgC = 0.0;
    } else{
        maxC = avgC = 0.0;
        minC = std::numeric_limits<double>::max();
        for (auto x : qAsConst(keys)){
            if (minC > mapInfluencedRows.values(x).size()){
                minC = mapInfluencedRows.values(x).size();
            }
            if (maxC < mapInfluencedRows.values(x).size()){
                maxC = mapInfluencedRows.values(x).size();
            }
            avgC += mapInfluencedRows.values(x).size();
        }
        if ((sumPoints - keys.size()) > 0){
            minC = 1.0;
            avgC += sumPoints - keys.size();
        }
        avgC = avgC / sumPoints;
    }

    answer += "Average density:\n  ";
    answer += QString::number(avgC, 'f', 2) + " elements\n";

    answer += "Minimum cluster density:\n  ";
    answer += QString::number(minC, 'f', 2) + " elements\n";

    answer += "Maximal cluster density:\n  ";
    answer += QString::number(maxC, 'f', 2) + " elements\n";

    return answer;
}

QString Analytics::buildNeighborScope(MedicalImageTable tempSet, QStringList scopeList, const int rowId){

    QString answer;
    std::string caption;
    int posScope;

    for (int x = 0; x < scopeList.size(); x++){
        posScope = tempSet.locateColumn(scopeList.at(x).simplified());
        if (posScope != -1){
            if (!answer.size()){
                answer += "== Scope Data ==\n\n";
            }
            if (scopeList.at(x).simplified().toUpper() != "URL"){
                answer += "\n";
                answer += scopeList.at(x).simplified() + ":\n  ";
                answer += tempSet.fetchByColumnId(rowId, posScope);
                if (scopeCaption.findCaption(tempSet.fetchCaption(posScope),
                                             tempSet.fetchByColumnId(rowId, posScope),
                                             &caption)){
                    answer += " = " + QString(caption.c_str());
                }
            }
        }
    }

    return answer;
}

/// Stats
///
/// Influencer ID: xx
/// Distance to Query Image: xx.xx (PCA projection)
/// Clustered elements:  xx elements
/// Result set max density: xx elements
/// Local cluster density: ~ xx.xx %
/// Global cluster density: ~ xx.xx %
/// #Elements matching visual mining target: xx (target)
/// %Elements matching visual mining target: xx.xx% (target), xx.xx (others)
/// #Values distinct to visual mining target: {xx, xx, xx, xx}
///
QString Analytics::buildNeighborStats(const int rowId, const double distToOq){

    QString answer;
    QStringList nonTargetValues;
    std::string caption;
    int colPos, colPos2;

    answer += "== Diversity-based distance stats ==\n\n";
    colPos = rSet.locateImageID();

    answer += "Influencer ID:\n  ";
    answer += rSet.fetchByColumnId(rowId, colPos) + "\n";

    answer += "Distance to Query Image:\n  ";
    answer += QString::number(distToOq, 'f', 2) + " (PCA projection)\n";

    answer += "Clustered elements:\n  ";
    answer += QString::number(mapInfluencedRows.values(rowId).size() + 1.0, 'f', 0) + " elements\n";

    double maxDensity = 0.0;
    double sumDensity = 1.0;
    QList<uint16_t> keys = mapInfluencedRows.uniqueKeys();
    for (auto x : qAsConst(keys)){
        if (mapInfluencedRows.values(x).size() > maxDensity){
            maxDensity = mapInfluencedRows.values(x).size();
        }
        sumDensity += mapInfluencedRows.values(x).size() + 1.0;
    }
    maxDensity += 1.0;

    answer += "Result set max density:\n  ";
    answer += QString::number(maxDensity, 'f', 0) + " elements\n";

    answer += "Local cluster density:\n  ";
    answer += "~" + QString::number(((mapInfluencedRows.values(rowId).size() + 1.0)/maxDensity)*100.0, 'f', 2) + "% \n";

    answer += "Global cluster density:\n  ";
    answer += "~" + QString::number(((mapInfluencedRows.values(rowId).size() + 1.0)/sumDensity)*100.0, 'f', 2) + "% \n";

    colPos = rInfset.locateColumn(ui->cbxTarget->currentText());
    colPos2 = rSet.locateColumn(ui->cbxTarget->currentText());

    size_t sum = 1;
    for (int x = 0; x < mapInfluencedRows.values(rowId).size(); x++){
        if (rSet.fetchByColumnId(rowId, colPos2).toStdString() ==
            rInfset.fetchByColumnId(mapInfluencedRows.values(rowId).at(x), colPos).toStdString()){
            sum++;
        } else {
            nonTargetValues.append(rInfset.fetchByColumnId(mapInfluencedRows.values(rowId).at(x), colPos));
        }
    }

    answer += "#Elements matching visual mining target:\n  ";
    answer += QString::number(sum, 'f', 0) + " elements (";
    scopeCaption.findCaption(rSet.fetchCaption(colPos2), rSet.fetchByColumnId(rowId, colPos2), &caption);
    answer += QString(caption.c_str()) + ")\n";

    answer += "%Elements matching visual mining target:\n  ";
    answer += "~" + QString::number(( (sum + 0.0)/(mapInfluencedRows.values(rowId).size() + 1.0))*100, 'f', 2) + "% (" + caption.c_str() + ") ";
    answer += "~" + QString::number( (1.0 - ( (sum + 0.0)/(mapInfluencedRows.values(rowId).size() + 1.0)))*100.0, 'f', 2) + "% (others) \n";

    answer += "#Values distinct to visual mining target:\n  ";
    answer += "Targets = {";
    for (int x = 0;  x < nonTargetValues.size(); x++){
        if (x > 0){
            answer += ", ";
        }
        scopeCaption.findCaption(rSet.fetchCaption(colPos2), nonTargetValues.at(x), &caption);
        answer += QString(caption.c_str());
    }
    answer += "}";

    return answer;
}

double Analytics::L2Distance(QPointF p1, QPointF p2){

    double answer = 0.0;
    answer = qSqrt((p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y()));
    return answer;
}

std::vector<std::pair<double, double> > Analytics::spiral(const uint16_t N, double *farthestInAxis){

    std::vector<std::pair<double, double>> answer;
    std::pair<double, double> point;

    float x = 0;
    float y = 0;
    float angle = 0.0f;

    //Space between the spirals
    double b = 0.1;

    for (int i = 100; i < N+100; i++)
    {
        angle = 0.25 * i;
        x = (b * angle) * cos(angle);
        y = (b * angle) * sin(angle);

        point.first = x;
        point.second = y;
        answer.push_back(point);
    }

    if (farthestInAxis != nullptr){
        if (answer.size()){
            *farthestInAxis = 0.0f;
            for (size_t x = 0; x < answer.size(); x++){
                *farthestInAxis = std::max(*farthestInAxis, std::fabs(answer[x].first));
                *farthestInAxis = std::max(*farthestInAxis, std::fabs(answer[x].second));
            }
        }
    }

    return answer;
}


void Analytics::on_btnUpdate_clicked(){

    int colTarget, colInfluenced;
    double maxDensity, maxAbsFar, alpha;
    FeatureVector oi;

    //Destroy current chart if exists
    if (seriesByTarget != nullptr) {
        for (int i = 0; i < nSeries; i++) {
            delete (seriesByTarget[i]);
        }
        delete[] seriesByTarget;
        seriesByTarget = nullptr;
    }
    if (chart != nullptr){
        ui->lytChart->removeWidget(chart);
        delete (chart);
        chart = nullptr;
    }

    colTarget = rSet.locateColumn(ui->cbxTarget->currentText());
    colInfluenced = rSet.locateColumn(simAttribute + "$bridged");

    //rSet labels
    targets.clear();
    for (int x = 0; x < rSet.size(); x++){
        if (Util::findColumn(rSet.fetchByColumnId(x, colTarget), targets) == -1){
            targets.append(rSet.fetchByColumnId(x, colTarget));
        }
    }
    nSeries = targets.size()+1;

    //Query image 'series'
    seriesByTarget = new QScatterSeries*[nSeries];
    for (int x = 0; x < nSeries; x++){
        seriesByTarget[x] = new QScatterSeries();
    }
    seriesByTarget[0]->append(oqRed[0], oqRed[1]);
    seriesByTarget[0]->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    seriesByTarget[0]->setName("Query image");

    //Calculate density and coverage of the largest cluster
    maxDensity = 0.0f;
    maxAbsFar = 0.0f;
    QList<uint16_t> keys = mapInfluencedRows.uniqueKeys();
    for (auto x : qAsConst(keys)){
        if (mapInfluencedRows.values(x).size() > maxDensity){
            maxDensity = mapInfluencedRows.values(x).size();
        }
    }
    maxDensity += 1.0;

    //Instantiate the chart
    chart = new QChartView();
    chart->setRenderHint(QPainter::HighQualityAntialiasing);
    chart->chart()->setTitle("WebHigiia - Diversity-based Medical Visual Mining");
    chart->chart()->setTheme(QChart::ChartThemeDark);

    //For each label...
    for (int x = 1; x < nSeries; x++){

        std::vector<std::pair<double, double>> spiralValues = spiral((uint16_t) maxDensity, &maxAbsFar);
        std::string legend;

        if (scopeCaption.findCaption(ui->cbxTarget->currentText(), targets[x-1], &legend)){
            seriesByTarget[x]->setName(legend.c_str());
        } else {
            seriesByTarget[x]->setName(targets[x-1]);
        }
        seriesByTarget[x]->setPen(QPen(Qt::PenStyle::NoPen));

        //... fetches every element of that label
        for (int y = 0; y < rSet.size(); y++){

            //Matches the element to its 2D PCA reduction...
            oi.unserializeFromString(FeatureVector::fromBase64(rSet.fetchByColumnId(y, colInfluenced).toStdString()));
            std::pair<uint32_t, QPointF> mappedValues;

            bool found = false;
            for (size_t z = 0; ((!found) && (z < dataset2D.size())); z++){
                found = ((oi.GetOID() == dataset2D[z].getOID()) &&
                         (rSet.fetchByColumnId(y, colTarget).toStdString() == targets[x-1].toStdString()));

                if (found){
                    //Create a plot center point for the influencer...                    
                    seriesByTarget[x]->append(QPointF(dataset2D[z][0], dataset2D[z][1]));
                    mappedValues.first = y;
                    mappedValues.second = QPointF(dataset2D[z][0], dataset2D[z][1]);
                    mapPointToRowId.push_back(mappedValues);

                    //Draw an spiral according to the number of influenced elements...
                    spiralValues.clear();
                    spiralValues = spiral((uint16_t) mapInfluencedRows.values(y).size());

                    //... and add the scaled spiral elements centered at the influencer object
                    alpha = 0.018/maxAbsFar;
                    for (size_t rX = 0; rX < spiralValues.size(); rX++){
                        seriesByTarget[x]->append(QPointF((spiralValues[rX].first*alpha)+dataset2D[z][0], (spiralValues[rX].second*alpha)+dataset2D[z][1]));
                        mappedValues.first = y;
                        mappedValues.second = QPointF((spiralValues[rX].first*alpha)+dataset2D[z][0], (spiralValues[rX].second*alpha)+dataset2D[z][1]);
                        mapPointToRowId.push_back(mappedValues);
                    }
                }
            }
        }
        connect(seriesByTarget[x], &QScatterSeries::clicked, this, &Analytics::handleClickedPoint);
    }



    //Populate chart and its appereance
    for (int x = 0; x < nSeries; x++){
        chart->chart()->addSeries(seriesByTarget[x]);
    }
    chart->chart()->createDefaultAxes();
    chart->chart()->setDropShadowEnabled(false);
    chart->chart()->axes(Qt::Horizontal).first()->setRange(-0.1, 1.1);
    chart->chart()->axes(Qt::Vertical).first()->setRange(-0.1, 1.1);
    chart->chart()->axisX()->setGridLineVisible(false);
    chart->chart()->axisY()->setGridLineVisible(false);
    chart->chart()->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    #if WASM_COMPILATION
    {
        chart->setMinimumSize(QSize(300,300));
    }
    #else
        chart->setMinimumSize(QSize(300,720));
    #endif
    chart->setMaximumSize(QSize(720,720));

    chart->show();
    ui->lytChart->addWidget(chart);
    ui->lytChart->update();
}

void Analytics::on_btnScopeOi_clicked(){

    ui->tabDataOi->setCurrentIndex(0);
}


void Analytics::on_btnStatsOi_clicked(){

    ui->tabDataOi->setCurrentIndex(1);
}


void Analytics::on_btnScopeOq_clicked(){

    ui->tabDataOq->setCurrentIndex(0);
}


void Analytics::on_btnStatsOq_clicked(){

    ui->tabDataOq->setCurrentIndex(1);
}


void Analytics::on_btnSearchOi_clicked(){

    #if M_PROV
    {
        Image *currentImage = Util::openImage(oqFileName);
        if (diagnosisForm != nullptr){
            delete (diagnosisForm);
        }
        diagnosisForm = new FormDiagnosis(oqFileName, oqId, tableName, "origin: Before cluster expansion", webSocket, userId, mapNameToMask[oqFileName]);
        connect(diagnosisForm, &FormDiagnosis::finished, this, &Analytics::state10);
        delete (currentImage);
        diagnosisForm->showFullScreen();
    }
    #else
        state10();
    #endif
}


void Analytics::on_btnClose_clicked(){

    this->close();
}

void Analytics::state09(){

    #if M_PROV
    {
        disconnect(diagnosisForm, &FormDiagnosis::finished, this, &Analytics::state09);
        if (diagnosisForm != nullptr){
            delete(diagnosisForm);
            diagnosisForm = nullptr;
        }
    }
    #endif

    searchForm = new QueryParameters(oqId, tableName, oqFileName, webSocket, userId, oq, link, numberK, clusterSize, nullptr);
    connect(searchForm, &QueryParameters::finished, this, &Analytics::on_btnClose_clicked);
    searchForm->showFullScreen();
}

void Analytics::state10(){

    #if M_PROV
    {
        disconnect(diagnosisForm, &FormDiagnosis::finished, this, &Analytics::state10);
        if (diagnosisForm != nullptr){
            delete(diagnosisForm);
            diagnosisForm = nullptr;
        }
    }
    #endif

    lockWidgets();
    MedicalImageTable newRSet;
    QString influencer, scopeAttributes;
    int col;


    newRSet.addCaption(rInfset.fetchCaption());
    for (int x = 0; x < rInfset.dimensionality(); x++){
        col = Util::findColumn(rInfset.fetchCaption(x), rSet.fetchCaption());
        if (x > 0){
            influencer += ", ";
        }
        if (col == -1){
            influencer += "";
        } else{
            influencer += rSet.fetchByColumnId(ui->btnOi->whatsThis().toUInt(), col);
        }
    }

    newRSet.addTuple(influencer);

    for (int x = 0; x < mapInfluencedRows.values(ui->btnOi->whatsThis().toUInt()).size(); x++){
        newRSet.addTuple(rInfset.fetchTupleByRowId(mapInfluencedRows.values(ui->btnOi->whatsThis().toUInt()).at(x)));
    }

    for (int x = 0; x < scopeAtts.size(); x++){
        if (x > 0){
            scopeAttributes += ", ";
        }
        scopeAttributes += scopeAtts[x];
    }

    if (newRSet.size() > 0){
        OberonViewer *bufferViewer = new OberonViewer(false,
                                        scopeAttributes,
                                        tableName,
                                        oqFileName,
                                        simAttribute,
                                        oq,
                                        newRSet,
                                        Util::CLUSTER_EXPANSION_SEARCH,
                                        newRSet.size(),
                                        1,
                                        tableName,
                                        metricName,
                                        webSocket,
                                        oqId,
                                        userId,
                                        link,
                                        nullptr);
        bufferViewer->showFullScreen();
        unlockWidgets();
        ui->lblStatus->setText("CBIR is ready!");
    } else {
        ui->lblStatus->setText("Empty result set!");
        return;
    }
}

void Analytics::on_btnSearchOq_clicked(){

    #if M_PROV
    {
        if (diagnosisForm != nullptr){
            delete (diagnosisForm);
        }
        diagnosisForm = new FormDiagnosis(oqFileName, oqId, tableName, "origin: After checking Analytics", webSocket, userId, mapNameToMask[oqFileName]);
        connect(diagnosisForm, &FormDiagnosis::finished, this, &Analytics::state09);
        diagnosisForm->showFullScreen();
    }
    #else
        state09();
    #endif
}

void Analytics::on_btnPACS_clicked(){

    QDesktopServices::openUrl(QUrl((PACS_BASE_URL + link), QUrl::TolerantMode));
}


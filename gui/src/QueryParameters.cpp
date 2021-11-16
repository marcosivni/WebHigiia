#include "QueryParameters.h"
#include "ui_QueryParameters.h"


/**
* Constructor.
*
* @param study The study data
* @param parent The parent widget
*/
QueryParameters::QueryParameters(int32_t studyId, QString tableName, QString imageFile, QWebSocket *webSocket, int32_t userId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QueryParameters){

    ui->setupUi(this);

    /* Delete this object when the form is closed */
    this->setAttribute(Qt::WA_DeleteOnClose);

    QDesktopWidget *desktop = QApplication::desktop();
    uint16_t screenWidth, width;
    uint16_t screenHeight, height;
    uint16_t x, y;
    QSize windowSize;

    screenWidth = desktop->width(); // get width of screen
    screenHeight = desktop->height(); // get height of screen

    windowSize = size(); // size of our application window
    width = windowSize.width();
    height = windowSize.height();

    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;

    // move window to desired coordinates
    move ( x, y );

    this->webSocket = webSocket;
    this->filename = imageFile;
    this->tableName = tableName;
    this->studyId = studyId;
    this->userId = userId;

    //Locking widgets...
    lockWidgets();
    ui->lblServerSetup->setText("Requesting, please wait ...");


    if (QFileInfo::exists("fs/" +imageFile) && QFileInfo("fs/" + imageFile).isFile()){
        //Load image
        Image *img = Util::openThumbnail(filename);
        if(img != nullptr){
            QImage *converter = Util::convertImageToQImage(img);
            ui->lblImg->setPixmap(QPixmap::fromImage(converter->scaled(300, 300)));
            delete (converter);
        } else {
            ui->lblServerSetup->setText("The query image is an invalid DICOM file!");
        }
        delete (img);
        loadScope();
    } else {
        //Blocking call - State Machine
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state01);
        webSocket->sendBinaryMessage(("REQUEST " + imageFile).toStdString().c_str());
    }
}

void QueryParameters::lockWidgets(){

    ui->cbxHypothesis->setEnabled(false);
    ui->txtMinimum->setEnabled(false);
    ui->txtMaximum->setEnabled(false);
    ui->cbxSimilarityAttribute->setEnabled(false);
    ui->cbxDf->setEnabled(false);
    ui->cbxRF->setEnabled(false);
    ui->txtNumberDiversity->setEnabled(false);
    ui->txtNumberNeighbors->setEnabled(false);
    ui->rbtDiversity->setEnabled(false);
    ui->rbtSimilarity->setEnabled(false);
    ui->rbtBridge->setEnabled(false);


    ui->btnAnalytics->setEnabled(false);
    ui->btnClose->setEnabled(false);
    ui->btnNewSearchParameter->setEnabled(false);
    ui->btnDiagnosis->setEnabled(false);
    ui->btnViewStudy->setEnabled(false);
}

void QueryParameters::unlockWidgets(){

    ui->cbxHypothesis->setEnabled(true);
    ui->txtMinimum->setEnabled(true);
    ui->txtMaximum->setEnabled(true);
    ui->cbxSimilarityAttribute->setEnabled(true);
    ui->cbxDf->setEnabled(true);
    ui->cbxRF->setEnabled(true);
    ui->txtNumberDiversity->setEnabled(true);
    ui->txtNumberNeighbors->setEnabled(true);
    ui->rbtDiversity->setEnabled(true);
    ui->rbtSimilarity->setEnabled(true);
    ui->rbtBridge->setEnabled(true);

    ui->btnAnalytics->setEnabled(true);
    ui->btnClose->setEnabled(true);
    ui->btnNewSearchParameter->setEnabled(true);
    ui->btnDiagnosis->setEnabled(true);
    ui->btnViewStudy->setEnabled(true);
}

/**
* Destructor.
*
*/
QueryParameters::~QueryParameters(){

    delete ui;
}

void QueryParameters::state01(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state01);
    ui->lblServerSetup->setText("Loading, please wait ...");

    if (!filename.toStdString().empty()){
        Util::saveImageAndThumbnailToFS(filename, message, QSize(300,300));
    } else {
        ui->lblServerSetup->setText("Local storage fatal error.");
    }
    ui->lblServerSetup->setText("Image sucessfully downloaded.");

    Image *img = Util::openThumbnail(filename);
    if(img != nullptr){
        QImage *converter = Util::convertImageToQImage(img);
        ui->lblImg->setPixmap(QPixmap::fromImage(converter->scaled(300, 300)));
        delete (converter);
    } else {
        ui->lblServerSetup->setText("The query image is an invalid DICOM file!");
    }
    delete (img);

    loadScope();
}

void QueryParameters::loadScope(){

    ui->lblServerSetup->setText("Fetching scope, please wait ...");

    SirenSQLQuery buildScope;
    buildScope.addProjectionAttribute("attributeName");
    buildScope.addTable("Scope");
    buildScope.addWhereAttribute("tableName = '" + tableName.split("_").last() + "'");

    //Blocking call - State Machine
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state02);
    webSocket->sendBinaryMessage(buildScope.generateQuery().toStdString().c_str());
}

void QueryParameters::state02(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state02);

    ui->lblServerSetup->setText("Loading scope, please wait ...");
    ui->cbxHypothesis->clear();
    ui->cbxHypothesis->addItem("NONE");

    ResultTable scopeTable(Util::toStringList(message.split('\n')));
    for (int x = 0; x < scopeTable.size(); x++){
        ui->cbxHypothesis->addItem(scopeTable.fetchByColumnId(x, 0));
    }


    ui->lblServerSetup->setText("Fetching attributes, please wait ...");
    SirenSQLQuery buildSimAtt;
    buildSimAtt.addProjectionAttribute("DISTINCT ComplexAttribName");
    buildSimAtt.addTable("CDD$ComplexAttribMetric");
    buildSimAtt.addWhereAttribute("TableName = '" + tableName +"'");

    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state03);
    webSocket->sendBinaryMessage(buildSimAtt.generateQuery().toStdString().c_str());
}

void QueryParameters::state03(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state03);
    ui->lblServerSetup->setText("Loading attributes, please wait ...");

    ResultTable simAttTable(Util::toStringList(message.split('\n')));

    for (int x = 0; x < simAttTable.size(); x++){
        ui->cbxSimilarityAttribute->addItem(simAttTable.fetchByColumnId(x, 0));
    }

    ui->lblServerSetup->setText("Fetching functions, please wait ...");


    SirenSQLQuery buildMetric;
    buildMetric.addProjectionAttribute("MetricName");
    buildMetric.addTable("EPD$DistanceFunctions e");
    buildMetric.addJoinList({"CDD$MetricStruct m"}, {"e.DistanceCode = m.DistanceCode"});

    //Every df is returned... Blocking call - State Machine
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state04);
    webSocket->sendBinaryMessage(buildMetric.generateQuery().toStdString().c_str());
}

void QueryParameters::state04(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state04);
    ui->lblServerSetup->setText("Loading functions, please wait ...");

    ResultTable dfTable(Util::toStringList(message.split('\n')));
    ui->cbxDf->clear();
    for (int x = 0; x < dfTable.size(); x++){
        ui->cbxDf->addItem(dfTable.fetchByColumnId(x, 0));
    }

    //@to-do Add other RF methods
    ui->cbxRF->addItem("Rocchio");

    //Unlocking screen for interaction
    unlockWidgets();
    ui->lblServerSetup->setText("Connected to the Server!");
}

void QueryParameters::on_btnViewStudy_clicked(){

    unlockWidgets();

    similarityAttribute = ui->cbxSimilarityAttribute->currentText();

    if(ui->txtNumberNeighbors->text().toInt() <= 0){
        ui->lblServerSetup->setText("Invalid number of neighbors!");
        ui->txtNumberNeighbors->setFocus();
        return;
    }else{
        ui->lblServerSetup->setText("Building the query, please wait...");

        //Locking widgets...
        lockWidgets();

        //Montar objeto de consulta
        SirenSQLQuery buildFV;
        buildFV.addProjectionAttribute("PPV$" + tableName + "_" + similarityAttribute + "." + similarityAttribute);
        buildFV.addTable(tableName);
        buildFV.addJoinList( {"PPV$" + tableName + "_" + similarityAttribute}, {tableName + "." + similarityAttribute + " = " + " PPV$" + tableName + "_" + similarityAttribute + "." + similarityAttribute + "_id"});
        buildFV.addWhereAttribute("Id = " + QString::number(studyId));

        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state05);
        webSocket->sendBinaryMessage(buildFV.generateQuery().toStdString().c_str());
    }

}

void QueryParameters::state05(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state05);
    ui->lblServerSetup->setText("Building query, please wait...");

    QString vTable, tblName, queryObjectValue, condition;

    //Query object
    ResultTable oqTable(Util::toStringList(message.split('\n')));
    if (!oqTable.size()){
        ui->lblServerSetup->setText("Fatal error loading the query object!");
        return;
    }

    oq.unserializeFromString(FeatureVector::fromBase64(oqTable.fetchByColumnId(0, 0).toStdString()));
    queryObjectValue = " { ";
    for (size_t x = 0; x < oq.size(); x++){
        if (x > 0)
            queryObjectValue += ", ";
        queryObjectValue += QString::number(*oq.get(x));
    }
    queryObjectValue += " }";

    //Query table
    if (ui->cbxHypothesis->currentIndex()){
        // Build vTable
        vTable   = " SELECT * FROM " + tableName.split("_").at(1);
        vTable  += " WHERE " + tableName.split("_").at(1) +"."+ ui->cbxHypothesis->currentText();
        vTable  += " >= "    + ui->txtMinimum->text();
        vTable  += " AND "   + tableName.split("_").at(1) +"."+ ui->cbxHypothesis->currentText();
        vTable  += " <= "    + ui->txtMaximum->text();
        vTableName = vTable;
        tblName = "temp";
    } else {
        tblName = tableName.split("_").at(1);
    }

    //Query scope
    for (int x = 1; x < ui->cbxHypothesis->count(); x++){
        if (x > 1){
            scopeAttributes += ", ";
        }
        scopeAttributes += tblName + "." + ui->cbxHypothesis->itemText(x);
    }

    SirenSQLQuery queryT;
    //Projection
    for (int x = 1; x < ui->cbxHypothesis->count(); x++){
        queryT.addProjectionAttribute(tblName + "." + ui->cbxHypothesis->itemText(x));
    }
    queryT.addProjectionAttribute(tblName + ".Filename Filename");
    queryT.addProjectionAttribute(tblName + ".Id Id");
    //Reading/Join
    if (ui->cbxHypothesis->currentIndex()){
        queryT.addTable("( " + vTable + " ) AS temp ");
    } else {
        queryT.addTable(tblName);
    }
    //Selection
    condition = tblName +"."+ similarityAttribute;
    if (ui->rbtSimilarity->isChecked()){
        condition += " NEAR ";
        searchType = Util::SIMILARITY_SEARCH;
    } else {
        if (ui->rbtDiversity->isChecked()){
            condition += " DIVERSITY NEAR ";
            searchType = Util::DIVERSITY_SEARCH;
        } else {
            condition += " DIVERSIFIED NEAR ";
            searchType = Util::BRIDGE_SEARCH;
        }
    }
    condition += queryObjectValue;
    condition += " BY "+ ui->cbxDf->currentText() + " STOP AFTER " + ui->txtNumberNeighbors->text();
    if (ui->rbtBridge->isChecked()){
        condition += " BRIDGE " + ui->txtNumberDiversity->text();
    }
    queryT.addWhereAttribute(condition);
    //Order by
    queryT.addOrderByAttribute(tblName + ".Id");

    ui->lblServerSetup->setText("Searching, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state06);
    provQuery = queryT.generateQuery();
    webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
}


void QueryParameters::state06(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state06);
    ui->lblServerSetup->setText("Connected to the Server!");
    bufferRSet = message;

    if (userId != -1){
        ui->lblServerSetup->setText("Saving provenance, please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state09);
        webSocket->sendBinaryMessage(Util::buildProvenanceInsert(userId, studyId, tableName.split("_").last(), "search", provQuery, "NONE").toStdString().c_str());
    } else {
        state10(bufferRSet);
    }
}

void QueryParameters::state09(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state09);
    ui->lblServerSetup->setText("Connected to the Server!");
    state10(bufferRSet);
}

void QueryParameters::state10(QByteArray message){

    MedicalImageTable rSet(Util::toStringList(message.split('\n')));

    if (rSet.size()){
        OberonViewer *oberonViewer = new OberonViewer(ui->cbxHypothesis->currentIndex(),
                                                      scopeAttributes,
                                                      tableName.split("_").last(),
                                                      filename,
                                                      similarityAttribute,
                                                      oq,
                                                      rSet,
                                                      searchType,
                                                      ui->txtNumberNeighbors->text().toUInt(),
                                                      ui->txtNumberDiversity->text().toUInt(),
                                                      vTableName,
                                                      ui->cbxDf->currentText(),
                                                      webSocket,
                                                      studyId,
                                                      userId,
                                                      "",
                                                      nullptr);
        oberonViewer->showFullScreen();
    } else {
        ui->lblServerSetup->setText("Empty result set!");
    }

    scopeAttributes.clear();
    vTableName.clear();

    unlockWidgets();
}

void QueryParameters::on_btnClose_clicked(){

    this->close();
}


void QueryParameters::on_btnAnalytics_clicked(){

    similarityAttribute = ui->cbxSimilarityAttribute->currentText();

    if(ui->txtNumberNeighbors->text().toInt() <= 0){
        ui->lblServerSetup->setText("Invalid number of neighbors!");
        ui->txtNumberNeighbors->setFocus();
        return;
    }else{
        ui->lblServerSetup->setText("Building the reference, please wait...");

        //Locking widgets...
        lockWidgets();

        //Montar objeto de consulta
        SirenSQLQuery buildFV;
        buildFV.addProjectionAttribute("PPV$" + tableName + "_" + similarityAttribute + "." + similarityAttribute);
        buildFV.addTable(tableName);
        buildFV.addJoinList( {"PPV$" + tableName + "_" + similarityAttribute}, {tableName + "." + similarityAttribute + " = " + " PPV$" + tableName + "_" + similarityAttribute + "." + similarityAttribute + "_id"});
        buildFV.addWhereAttribute("Id = " + QString::number(studyId));

        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state07);
        webSocket->sendBinaryMessage(buildFV.generateQuery().toStdString().c_str());
    }
}

void QueryParameters::state07(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state07);
    ui->lblServerSetup->setText("Building query, please wait...");

    QString vTable, tblName, queryObjectValue, condition;

    //Query object
    ResultTable oqTable(Util::toStringList(message.split('\n')));
    if (!oqTable.size()){
        ui->lblServerSetup->setText("Fatal error loading the query object!");
        return;
    }

    oq.unserializeFromString(FeatureVector::fromBase64(oqTable.fetchByColumnId(0, 0).toStdString()));
    queryObjectValue = " { ";
    for (size_t x = 0; x < oq.size(); x++){
        if (x > 0)
            queryObjectValue += ", ";
        queryObjectValue += QString::number( (*oq.get(x)) );
    }
    queryObjectValue += " }";

    //Query table
    if (ui->cbxHypothesis->currentIndex()){
        // Build vTable
        vTable   = " SELECT * FROM " + tableName.split("_").at(1);
        vTable  += " WHERE " + tableName.split("_").at(1) +"."+ ui->cbxHypothesis->currentText();
        vTable  += " >= "    + ui->txtMinimum->text();
        vTable  += " AND "   + tableName.split("_").at(1) +"."+ ui->cbxHypothesis->currentText();
        vTable  += " <= "    + ui->txtMaximum->text();
        vTableName = vTable;
        tblName = "temp";
    } else {
        tblName = tableName.split("_").at(1);
    }

    //Query scope
    for (int x = 1; x < ui->cbxHypothesis->count(); x++){
        if (x > 1){
            scopeAttributes += ", ";
        }
        scopeAttributes += tblName + "." + ui->cbxHypothesis->itemText(x);
    }

    SirenSQLQuery queryT;
    //Projection
    for (int x = 1; x < ui->cbxHypothesis->count(); x++){
        queryT.addProjectionAttribute(tblName + "." + ui->cbxHypothesis->itemText(x));
    }
    queryT.addProjectionAttribute(tblName + ".Filename Filename");
    queryT.addProjectionAttribute(tblName + ".Id Id");
    //Reading/Join
    if (ui->cbxHypothesis->currentIndex()){
        queryT.addTable("( " + vTable + " ) AS temp ");
    } else {
        queryT.addTable(tblName);
    }
    //Selection
    condition  = tblName +"."+ similarityAttribute;
    condition += " DIVERSIFIED NEAR " + queryObjectValue;
    condition += " BY "+ ui->cbxDf->currentText() + " STOP AFTER " + ui->txtNumberNeighbors->text() + " BRIDGE ";
    if (ui->rbtSimilarity->isChecked() || ui->rbtDiversity->isChecked()){
        condition += " 1 ";
    } else {
        condition += ui->txtNumberDiversity->text();
    }
    queryT.addWhereAttribute(condition);
    //Order by
    queryT.addOrderByAttribute(tblName + ".Id");


    ui->lblServerSetup->setText("Searching, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state08);
    provQuery = queryT.generateQuery();
    webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
}

void QueryParameters::state08(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state08);
    ui->lblServerSetup->setText("Connected to the Server!");
    bufferRSet = message;

    if (userId != -1){
        ui->lblServerSetup->setText("Saving provenance, please wait...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state11);
        webSocket->sendBinaryMessage(Util::buildProvenanceInsert(userId, studyId, tableName.split("_").last(), "analytics", provQuery, "NONE").toStdString().c_str());
    } else {
        state12(bufferRSet);
    }


}

void QueryParameters::state11(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state11);
    ui->lblServerSetup->setText("Connected to the Server!");
    state12(bufferRSet);
}

void QueryParameters::state12(QByteArray message){

    MedicalImageTable rSet(Util::toStringList(message.split('\n')));

    rSet.print();

    if (rSet.size()){
        Analytics *analyticsViewer = new Analytics(tableName.split("_").last(),
                                                   filename,
                                                   oq,
                                                   similarityAttribute,
                                                   ui->cbxDf->currentText(),
                                                   rSet,
                                                   ui->cbxHypothesis->currentIndex(),
                                                   vTableName,
                                                   webSocket,
                                                   studyId,
                                                   userId,
                                                   "",
                                                   nullptr);
        analyticsViewer->showFullScreen();
    } else {
        ui->lblServerSetup->setText("Empty result set!");
    }

    scopeAttributes.clear();
    vTableName.clear();

    //Unlocking widgets...
    unlockWidgets();
}


void QueryParameters::on_btnDiagnosis_clicked(){

    if (userId != -1){
        Image *currentImage = Util::openImage(filename);
        FormDiagnosis *diagnosis = new FormDiagnosis(*currentImage, studyId, tableName.split("_").last(), "origin: Query_Parameters", webSocket, userId);
        diagnosis->showFullScreen();
        delete (currentImage);
    } else {
        ui->lblServerSetup->setText("Reports are not allowed in this mode (provenance-disabled)!");
    }
}


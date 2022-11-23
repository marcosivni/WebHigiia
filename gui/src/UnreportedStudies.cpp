#include "UnreportedStudies.h"
#include "ui_UnreportedStudies.h"

UnreportedStudies::UnreportedStudies(QWebSocket *webSocket, const int userId, QString tableName, QString attName, QString numberK, QString clusterSize, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UnreportedStudies){

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

    //Keep websocket connection
    this->webSocket = webSocket;
    ui->txtSearchPatient->setFocus();

    this->tableName = tableName;
    this->attName = attName;
    this->numberK = numberK;
    this->clusterSize = clusterSize;

    //Provenance-related stuff
    #if M_PROV
        diagnosisForm = nullptr;
    #endif

    //Load studies data
    loadStudyTable(userId);
}

UnreportedStudies::~UnreportedStudies(){

    mapNameToMask.clear();
    delete ui;
}


void UnreportedStudies::loadStudyTable(const int userId){

    std::string readIn;
    SirenSQLQuery buildTable;
    QFile file("://files/conf.cbir");

    this->userId = QString::number(userId);
    ui->lblServerSetup->setText("Fetching user pool, please wait...");

    //Lock screen
    ui->centralwidget->setEnabled(false);

    if ((!tableName.size()) && (!attName.size()) && (!numberK.size()) && (!clusterSize.size())){
        ui->lblServerSetup->setText("Dataset setup is invalid. Please contact your DBA.");
    } else {
        buildTable.addProjectionList({"p.*", "Id", "Patient_Name", "Filename", "p.tableName TableName", "url URL"});
        buildTable.addTable("U_" + tableName + " u ");
        buildTable.addJoinList({"Pool p"}, {"p.imageId = u.Id"});
        buildTable.addWhereListAnd({"p.userId = '" + this->userId + "'", "p.tableName = '" + tableName + "'"});

        ui->lblServerSetup->setText("Loading pool, please wait...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state01);
        webSocket->sendBinaryMessage(buildTable.generateQuery().toStdString().c_str());
    }
}


void UnreportedStudies::populateStudyTable(MedicalImageTable records){

    //Clear rows and columns
    while(ui->tblInfo->rowCount() > 0){
        ui->tblInfo->removeRow(0);
    }
    while(ui->tblInfo->columnCount() > 0){
        ui->tblInfo->removeColumn(0);
    }

    //Create columns
    for(int x = 0; x < records.dimensionality(); x++){
        ui->tblInfo->insertColumn(x);
    }

    //Set headers
    for(int x = 0; x < records.dimensionality(); x++){
        ui->tblInfo->setHorizontalHeaderItem(x, new QTableWidgetItem(records.fetchCaption(x)));
    }


    //Populate table
    for (int row = 0; row < records.size(); row++){
        ui->tblInfo->insertRow(row);
        for (int column = 0; column < records.dimensionality(); column++){
            QTableWidgetItem *newItem = new QTableWidgetItem(records.fetchByColumnId(row, column));
            newItem->setToolTip(records.fetchByColumn(row, "Id") + ","
                                + records.fetchByColumn(row, "TableName") + ","
                                + records.fetchByColumn(row, "Filename") + ","
                                + records.fetchByColumn(row, "url"));
            newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));
            ui->tblInfo->setItem(row, column, newItem);
        }
    }

    //Unlock screen
    ui->centralwidget->setEnabled(true);

    ui->tblInfo->resizeColumnsToContents();
}

void UnreportedStudies::populateStudyTable(const std::vector<int> rowIds){

    //Clear rows
    while(ui->tblInfo->rowCount() > 0){
        ui->tblInfo->removeRow(0);
    }

    MedicalImageTable subsetRecords;
    subsetRecords.addCaption(records.fetchCaption());
    for (size_t x = 0; x < rowIds.size(); x++){
        subsetRecords.addTuple(records.fetchTupleByRowId(rowIds[x]));
    }

    //Populate table
    for (int row = 0; row < subsetRecords.size(); row++){
        ui->tblInfo->insertRow(row);
        for (int column = 0; column < subsetRecords.dimensionality(); column++){
            QTableWidgetItem *newItem = new QTableWidgetItem(subsetRecords.fetchByColumnId(row, column));
            newItem->setToolTip(subsetRecords.fetchByColumn(row, "Id") + ","
                                + subsetRecords.fetchByColumn(row, "TableName") + ","
                                + subsetRecords.fetchByColumn(row, "Filename") + ","
                                + subsetRecords.fetchByColumn(row, "url"));
            newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));
            ui->tblInfo->setItem(row, column, newItem);
        }
    }
    ui->tblInfo->resizeColumnsToContents();
}

void UnreportedStudies::changeEvent(QEvent *e){

    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void UnreportedStudies::on_btnNext_clicked(){

    if ((ui->tblInfo->currentRow() + 1) < (ui->tblInfo->rowCount())){
        ui->tblInfo->selectRow(ui->tblInfo->currentRow() + 1);
    }
}

void UnreportedStudies::on_btnBack_clicked(){

    if ((ui->tblInfo->currentRow() - 1) >= 0){
        ui->tblInfo->selectRow(ui->tblInfo->currentRow() - 1);
    }
}

void UnreportedStudies::on_btnClose_clicked(){

    this->close();
}

void UnreportedStudies::on_txtSearchPatient_textChanged(const QString &arg1){

    std::vector<int> rowIds;

    for (int x = 0; x < records.size(); x++){
        if (arg1.toStdString().empty() || records.fetchByColumn(x, "Patient_Name").contains(arg1, Qt::CaseInsensitive)){
            rowIds.push_back(x);
        }
    }
    populateStudyTable(rowIds);
}

void UnreportedStudies::on_btnViewStudy_clicked(){

    QModelIndexList selectedList = ui->tblInfo->selectionModel()->selectedRows();
    uint8_t pos = ui->tblInfo->currentRow();

    if (selectedList.size() != 0){
        try{
            QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
            QStringList parameters = item->toolTip().split(",");

            for (int k = 0; k < ui->tblInfo->columnCount(); k++){
                ui->tblInfo->item(pos, k)->setBackground(QBrush(QColor(187,187,187)));
            }

            ui->centralwidget->setEnabled(false);
            //Blocking call - State Machine
            if (QFileInfo::exists(WFS_NAME + parameters.at(2).simplified()) && QFileInfo(WFS_NAME + parameters.at(2).simplified()).isFile()){
                state03();
            } else {
                state02();
            }
        } catch(...) {
            ui->lblServerSetup->setText("Error: Invalid patient selection.");
        }
    }else{
        ui->lblServerSetup->setText("Error: Please, select a line with a patient.");
    }
    ui->lblServerSetup->setText("Connected to the Server!");
}

void UnreportedStudies::state01(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state01);

    if(message.isEmpty()){
        ui->lblServerSetup->setText("An error loading the studies ocurred, please try again.");
    } else {
        MedicalImageTable records(Util::toStringList(message.split('\n')));
        populateStudyTable(records);
        this->records = records;

        ui->lblServerSetup->setText("Connected to the Server!");
    }
}

void UnreportedStudies::state02(){

    SirenSQLQuery buildMaskStatement;
    uint8_t pos = ui->tblInfo->currentRow();
    QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
    QStringList parameters = item->toolTip().split(",");

    buildMaskStatement.addProjectionAttribute("Id, Mask");
    buildMaskStatement.addTable("U_" + parameters.at(1).simplified());
    buildMaskStatement.addWhereAttribute("Id = " + parameters.at(0));

    ui->lblServerSetup->setText("Loading masks, please wait ...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state09);
    webSocket->sendBinaryMessage(buildMaskStatement.generateQuery().toStdString().c_str());
}

void UnreportedStudies::state09(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state09);

    ResultTable masks(Util::toStringList(message.split('\n')));
    uint8_t pos = ui->tblInfo->currentRow();
    QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
    QStringList parameters = item->toolTip().split(",");



    if (masks.size()){
        mapNameToMask[parameters.at(1).simplified()] = masks.fetchByColumnId(0, masks.locateColumn("Mask"));
    }

    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state08);
    webSocket->sendBinaryMessage(("REQUEST " + parameters.at(2).simplified()).toStdString().c_str());
}

void UnreportedStudies::state08(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state08);
    ui->lblServerSetup->setText("Loading query image, please wait ...");

    uint8_t pos = ui->tblInfo->currentRow();

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");
        if (!parameters.at(2).simplified().toStdString().empty()){
            Util::saveImageAndThumbnailToFS(parameters.at(2).simplified(), message, QSize(300,300), mapNameToMask[parameters.at(1).simplified()]);
        } else {
            ui->lblServerSetup->setText("Local storage fatal error.");
        }
        ui->lblServerSetup->setText("Image sucessfully downloaded.");
        state03();
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state10(){

    #if M_PROV
        disconnect(diagnosisForm, &FormDiagnosis::finished, this, &UnreportedStudies::state10);
        if (diagnosisForm != nullptr){
            delete(diagnosisForm);
            diagnosisForm = nullptr;
        }
    #endif

    uint8_t pos = ui->tblInfo->currentRow();

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");

        ui->lblServerSetup->setText("Building the query, please wait...");
        QString tableName = "U_" + parameters.at(1).simplified();

        SirenSQLQuery buildFV;
        buildFV.addProjectionAttribute("PPV$" + tableName + "_"+attName+"."+attName+"");
        buildFV.addTable(tableName);
        buildFV.addJoinList( {"PPV$" + tableName + "_"+attName+""},
                                 {tableName + "."+attName+" = " + " PPV$" + tableName + "_"+attName+"."+attName+"_id"});
        buildFV.addWhereAttribute("Id = " + parameters.at(0).simplified());

        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state04);
        webSocket->sendBinaryMessage(buildFV.generateQuery().toStdString().c_str());
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state03(){

    //Provenance-related stuff
    #if M_PROV
    {
        if (diagnosisForm != nullptr){
            delete(diagnosisForm);
        }

        uint8_t pos = ui->tblInfo->currentRow();
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");

        diagnosisForm = new FormDiagnosis(parameters.at(2).simplified(),
                                          parameters.at(0).toInt(),
                                          parameters.at(1).simplified(),
                                          "origin: Initial Diagnosis",
                                          webSocket,
                                          userId.toInt(),
                                          mapNameToMask[parameters.at(1).simplified()]);
        connect(diagnosisForm, &FormDiagnosis::finished, this, &UnreportedStudies::state10);
        diagnosisForm->showFullScreen();
    }
    #else
        state10();
    #endif
}

void UnreportedStudies::state04(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state04);
    ui->lblServerSetup->setText("Building query, please wait...");

    //Query object
    ResultTable oqTable(Util::toStringList(message.split('\n')));
    if (!oqTable.size()){
        ui->lblServerSetup->setText("Fatal error loading the query object!");
        return;
    }
    oq.unserializeFromString(FeatureVector::fromBase64(oqTable.fetchByColumnId(0, 0).toStdString()));

    state05();
}

void UnreportedStudies::state05(){

    ui->lblServerSetup->setText("Fetching scope, please wait ...");
    uint8_t pos = ui->tblInfo->currentRow();

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");
        SirenSQLQuery buildScope;

        buildScope.addProjectionAttribute("attributeName");
        buildScope.addTable("Scope");
        buildScope.addWhereAttribute("tableName = '" + parameters.at(1).simplified() + "'");

        //Blocking call - State Machine
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state06);
        webSocket->sendBinaryMessage(buildScope.generateQuery().toStdString().c_str());
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state06(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state06);
    uint8_t pos = ui->tblInfo->currentRow();
    ui->lblServerSetup->setText("Building query, please wait ...");

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");
        QString tblName = parameters.at(1).simplified();
        QString queryObjectValue, condition;
        SirenSQLQuery queryT;

        queryObjectValue = " { ";
        for (size_t x = 0; x < oq.size(); x++){
            if (x > 0)
                queryObjectValue += ", ";
            queryObjectValue += QString::number(*oq.get(x));
        }
        queryObjectValue += " }";

        ResultTable scopeTable(Util::toStringList(message.split('\n')));
        if (!scopeTable.size()){
            ui->lblServerSetup->setText("Fatal error loading the query object!");
            return;
        }

        //Query scope
        for (int x = 0; x < scopeTable.size(); x++){
            if (x > 0){
                scope += ", ";
            }
            scope += tblName + "." + scopeTable.fetchByColumnId(x, 0);
        }

        queryT.addProjectionAttribute(scope);
        queryT.addProjectionAttribute(tblName + ".Filename Filename");
        queryT.addProjectionAttribute(tblName + ".Id Id");
        queryT.addTable(tblName);

        //Selection
        condition = tblName +"."+attName+"";
        condition += " DIVERSIFIED NEAR ";
        condition += queryObjectValue;
        condition += " STOP AFTER " + numberK;
        condition += " BRIDGE " + clusterSize;

        queryT.addWhereAttribute(condition);

        //Order by
        queryT.addOrderByAttribute(tblName + ".Id");

        ui->lblServerSetup->setText("Searching, please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state07);
        webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state07(QByteArray message){

    ui->lblServerSetup->setText("Connected to the Server!");
    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state07);
    uint8_t pos = ui->tblInfo->currentRow();

    MedicalImageTable rSet(Util::toStringList(message.split('\n')));

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");

        if (rSet.size()){
            Analytics *analyticsViewer = new Analytics(parameters.at(1).simplified(),
                                                       parameters.at(2).simplified(),
                                                       oq,
                                                       ""+attName+"",
                                                       "L2",
                                                       rSet,
                                                       false,
                                                       parameters.at(1).simplified(),
                                                       webSocket,
                                                       parameters.at(0).toInt(),
                                                       userId.toInt(),
                                                       parameters.at(3),
                                                       numberK,
                                                       clusterSize,
                                                       nullptr);
            scope.clear();
            analyticsViewer->showFullScreen();
        } else {
            ui->lblServerSetup->setText("Empty result set!");
        }
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
    ui->centralwidget->setEnabled(true);
}

void UnreportedStudies::on_btnPACS_clicked(){

    uint8_t pos = ui->tblInfo->currentRow();
    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");
        QString link = parameters.at(3).simplified();
        QDesktopServices::openUrl(QUrl(PACS_BASE_URL + link, QUrl::TolerantMode));
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}


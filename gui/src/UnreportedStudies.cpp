#include "UnreportedStudies.h"
#include "ui_UnreportedStudies.h"

/**
* Constructor.
*
* @param parent The parent widget. The default value is NULL.
*/
UnreportedStudies::UnreportedStudies(QWebSocket *webSocket, int userId, bool provenance, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UnreportedStudies){

    ui->setupUi(this);

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

    //Keep websocket connection
    this->webSocket = webSocket;
    this->provenance = provenance;
    this->diagnosis = nullptr;
    ui->txtSearchPatient->setFocus();

    //Load studies data
    loadStudyTable(userId);
}

UnreportedStudies::~UnreportedStudies(){

    if (diagnosis != nullptr){
        delete (diagnosis);
    }
    delete ui;
}


void UnreportedStudies::loadStudyTable(int userId){

    this->userId = QString::number(userId);
    ui->lblServerSetup->setText("Fetching user pool, please wait...");

    SirenSQLQuery buildPool;
    buildPool.addProjectionAttribute("DISTINCT tableName");
    buildPool.addTable("Pool");
    buildPool.addWhereAttribute("userId = " + QString::number(userId));

    //Lock screen
    ui->txtSearchPatient->setEnabled(false);
    ui->btnBack->setEnabled(false);
    ui->btnPACS->setEnabled(false);
    ui->btnNext->setEnabled(false);
    ui->btnViewStudy->setEnabled(false);
    ui->tblInfo->setEnabled(false);

    //Blocking call - State Machine
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state01);
    webSocket->sendBinaryMessage(buildPool.generateQuery().toStdString().c_str());
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
                                + records.fetchByColumn(row, "searchType") + ","
                                + records.fetchByColumn(row, "url"));
            newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));
            ui->tblInfo->setItem(row, column, newItem);
        }
    }

    //Unlock screen
    ui->txtSearchPatient->setEnabled(true);
    ui->btnBack->setEnabled(true);
    ui->btnPACS->setEnabled(true);
    ui->btnNext->setEnabled(true);
    ui->btnViewStudy->setEnabled(true);
    ui->tblInfo->setEnabled(true);

    ui->tblInfo->resizeColumnsToContents();
}

void UnreportedStudies::populateStudyTable(std::vector<int> rowIds){

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
                                + subsetRecords.fetchByColumn(row, "searchType") + ","
                                + subsetRecords.fetchByColumn(row, "url"));
            newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));
            ui->tblInfo->setItem(row, column, newItem);
        }
    }
    ui->tblInfo->resizeColumnsToContents();
}

/**
* This method is responsible to manage the event change.
*
* @param e The captured QEvent.
*/
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

/**
* Increments the selected row by 1 on the qTableWidget.
*
*/
void UnreportedStudies::on_btnNext_clicked(){

    if ((ui->tblInfo->currentRow() + 1) < (ui->tblInfo->rowCount())){
        ui->tblInfo->selectRow(ui->tblInfo->currentRow() + 1);
    }
}

/**
* Decrements the selected row by 1 on the qTableWidget.
*
*/
void UnreportedStudies::on_btnBack_clicked(){

    if ((ui->tblInfo->currentRow() - 1) >= 0){
        ui->tblInfo->selectRow(ui->tblInfo->currentRow() - 1);
    }
}

/**
* Finishes the application.
*
*/
void UnreportedStudies::on_btnClose_clicked(){

    this->close();
}

/**
* Employs the mask arg1 to filter patients by name.
*
* @param arg1 The search name
*/
void UnreportedStudies::on_txtSearchPatient_textChanged(const QString &arg1){

    std::vector<int> rowIds;

    for (int x = 0; x < records.size(); x++){
        if (arg1.toStdString().empty() || records.fetchByColumn(x, "Patient_Name").contains(arg1, Qt::CaseInsensitive)){
            rowIds.push_back(x);
        }
    }
    populateStudyTable(rowIds);
}

/**
* Shows the QueryParameters screen with the selected patient data.
* LANDMARK
*/
void UnreportedStudies::on_btnViewStudy_clicked(){

    QModelIndexList selectedList = ui->tblInfo->selectionModel()->selectedRows();
    uint8_t pos = ui->tblInfo->currentRow();

    if (selectedList.size() != 0){
        try{
            QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
            QStringList parameters = item->toolTip().split(",");

            if (records.fetchCaption().indexOf(QRegularExpression("\\s+searchType")) != -1){

                ui->centralwidget->setEnabled(false);
                //Blocking call - State Machine
                if (QFileInfo::exists(WFS_NAME + parameters.at(2).simplified()) && QFileInfo(WFS_NAME + parameters.at(2).simplified()).isFile()){
                    state05();
                } else {
                    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state04);
                    webSocket->sendBinaryMessage(("REQUEST " + parameters.at(2).simplified()).toStdString().c_str());
                }

            } else {

                QueryParameters *qp = nullptr;
                if (provenance){
                    qp = new QueryParameters(parameters.at(0).toInt(),
                                             parameters.at(1).simplified(),
                                             parameters.at(2).simplified(),
                                             webSocket,
                                             userId.toInt(),
                                             nullptr);
                } else {
                    qp = new QueryParameters(parameters.at(0).toInt(),
                                             parameters.at(1).simplified(),
                                             parameters.at(2).simplified(),
                                             webSocket);
                }
                qp->show();
            }
        } catch(...) {
            ui->lblServerSetup->setText("Error: Invalid patient selection.");
        }
    }else{
        ui->lblServerSetup->setText("Error: Please, select a line with a patient.");
    }
    ui->lblServerSetup->setText("Connected to the Server!");



}


//void UnreportedStudies::on_btnNewDiagnosis_clicked(){

//    uint8_t pos = ui->tblInfo->currentRow();
//    try{
//        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
//        QStringList parameters = item->toolTip().split(",");
//        QString link = parameters.at(4).simplified();
//        QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));


////        if (diagnosis != nullptr){
////            delete (diagnosis);
////        }
////        diagnosis = new FormDiagnosis(*currentImage, parameters.at(0).toInt(),
////                                                     parameters.at(1).simplified(),
////                                                     "origin: Initial_Diagnosis",
////                                                     webSocket,
////                                                     userId.toInt());
////        diagnosis->showFullScreen();
////        delete (currentImage);
//    } catch(...) {
//        ui->lblServerSetup->setText("Error: Invalid patient selection.");
//    }
//}


void UnreportedStudies::state01(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state01);

    if(message.isEmpty()){
           ui->lblServerSetup->setText("Images' pool is empty!");
       } else {
           ui->lblServerSetup->setText("Loading pool, please wait...");

           QString unionQuery;
           ResultTable poolTables(Util::toStringList(message.split('\n')));
           SirenSQLQuery buildTable;

           for (int x = 0; x < poolTables.size(); x++){
               if (x > 0){
                    unionQuery += " UNION ";
               }

               buildTable.clear();
               buildTable.addProjectionList({"p.*", "Id", "Patient_Name", "Filename", "p.tableName TableName", "url URL"});
               buildTable.addTable("U_" + poolTables.fetchByColumnId(x, 0) + " u ");
               buildTable.addJoinList({"Pool p"}, {"p.imageId = u.Id"});
               buildTable.addWhereListAnd({"p.userId = '" + userId + "'", "p.tableName = '" + poolTables.fetchByColumnId(x, 0) + "'"});

               unionQuery += buildTable.generateQuery();
           }

           connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state02);
           webSocket->sendBinaryMessage(unionQuery.toStdString().c_str());
       }
}

void UnreportedStudies::state02(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state02);

    if(message.isEmpty()){
        ui->lblServerSetup->setText("An error loading the studies ocurred, please try again.");
    } else {
        MedicalImageTable records(Util::toStringList(message.split('\n')));
        populateStudyTable(records);
        this->records = records;

        ui->lblServerSetup->setText("Connected to the Server!");
    }
}

void UnreportedStudies::state04(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state04);
    ui->lblServerSetup->setText("Loading query image, please wait ...");

    uint8_t pos = ui->tblInfo->currentRow();

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");
        if (!parameters.at(2).simplified().toStdString().empty()){
            Util::saveImageAndThumbnailToFS(parameters.at(2).simplified(), message, QSize(300,300));
        } else {
            ui->lblServerSetup->setText("Local storage fatal error.");
        }
        ui->lblServerSetup->setText("Image sucessfully downloaded.");
        state05();
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state05(){

    uint8_t pos = ui->tblInfo->currentRow();

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");

        ui->lblServerSetup->setText("Building the query, please wait...");
        QString tableName = "U_" + parameters.at(1).simplified();

        SirenSQLQuery buildFV;
        buildFV.addProjectionAttribute("PPV$" + tableName + "_PcaF.PcaF");
        buildFV.addTable(tableName);
        buildFV.addJoinList( {"PPV$" + tableName + "_PcaF"},
                             {tableName + ".PcaF = " + " PPV$" + tableName + "_PcaF.PcaF_id"});
        buildFV.addWhereAttribute("Id = " + parameters.at(0).simplified());

        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state06);
        webSocket->sendBinaryMessage(buildFV.generateQuery().toStdString().c_str());
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state06(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state06);
    ui->lblServerSetup->setText("Building query, please wait...");

    uint8_t pos = ui->tblInfo->currentRow();

    //Query object
    ResultTable oqTable(Util::toStringList(message.split('\n')));
    if (!oqTable.size()){
        ui->lblServerSetup->setText("Fatal error loading the query object!");
        return;
    }
    oq.unserializeFromString(FeatureVector::fromBase64(oqTable.fetchByColumnId(0, 0).toStdString()));

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");
        Image *currentImage = Util::openImage(parameters.at(2).simplified());
        if (diagnosis != nullptr){
            delete (diagnosis);
        }
        diagnosis = new FormDiagnosis(*currentImage, parameters.at(0).toInt(),
                                                     parameters.at(1).simplified(),
                                                     "origin: Initial_Diagnosis",
                                                     webSocket,
                                                     userId.toInt());
        connect(diagnosis, &FormDiagnosis::finished, this, &UnreportedStudies::state07);
        diagnosis->showFullScreen();
        delete (currentImage);
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state07(){

    disconnect(diagnosis, &FormDiagnosis::finished, this, &UnreportedStudies::state07);
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
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state08);
        webSocket->sendBinaryMessage(buildScope.generateQuery().toStdString().c_str());
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state08(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state08);
    uint8_t pos = ui->tblInfo->currentRow();
    ui->lblServerSetup->setText("Building query, please wait ...");

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");
        QString tblName = parameters.at(1).simplified();
        QString queryObjectValue, condition;
        SirenSQLQuery queryT;
        int posT = parameters.at(3).simplified().toInt();

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
        condition = tblName +".PcaF";
        if (posT == 1){
            condition += " NEAR ";
        } else {
            if (posT == 2){
                condition += " DIVERSITY NEAR ";
            } else {
                condition += " DIVERSIFIED NEAR ";
            }
        }

        condition += queryObjectValue;
        if (posT > 2){
            condition += " STOP AFTER 35";
        } else {
            if (posT > 1){
                condition += " STOP AFTER 10";
            } else {
                condition += " STOP AFTER 15";
            }
        }

        if (posT > 2){
            condition += " BRIDGE 10";
        }

        queryT.addWhereAttribute(condition);
        //Order by
        queryT.addOrderByAttribute(tblName + ".Id");

        ui->lblServerSetup->setText("Searching, please wait ...");
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state09);
        webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}

void UnreportedStudies::state09(QByteArray message){

    ui->lblServerSetup->setText("Connected to the Server!");
    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &UnreportedStudies::state09);
    uint8_t pos = ui->tblInfo->currentRow();

    MedicalImageTable rSet(Util::toStringList(message.split('\n')));

    try{
        QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
        QStringList parameters = item->toolTip().split(",");
        int posT = parameters.at(3).simplified().toInt();

        if (posT <= 2){
            //abrir Oberon
            if (rSet.size()){
                Util::SEARCH_TYPE searchType;
                if (posT == 1){
                    searchType = Util::SIMILARITY_SEARCH;
                } else {
                    if (posT == 2){
                        searchType = Util::DIVERSITY_SEARCH;
                    } else {
                        searchType = Util::BRIDGE_SEARCH;
                    }
                }

                OberonViewer *oberonViewer = new OberonViewer(false,
                                                              scope,
                                                              parameters.at(1).simplified(),
                                                              parameters.at(2).simplified(),
                                                              "PcaF",
                                                              oq,
                                                              rSet,
                                                              searchType,
                                                              10,
                                                              10,
                                                              parameters.at(1).simplified(),
                                                              "L2",
                                                              webSocket,
                                                              parameters.at(0).toInt(),
                                                              userId.toInt(),
                                                              parameters.at(4),
                                                              nullptr);
                scope.clear();
                oberonViewer->showFullScreen();
            } else {
                ui->lblServerSetup->setText("Empty result set!");
            }
        } else {
            //abrir analytics
            if (rSet.size()){
                Analytics *analyticsViewer = new Analytics(parameters.at(1).simplified(),
                                                           parameters.at(2).simplified(),
                                                           oq,
                                                           "PcaF",
                                                           "L2",
                                                           rSet,
                                                           false,
                                                           parameters.at(1).simplified(),
                                                           webSocket,
                                                           parameters.at(0).toInt(),
                                                           userId.toInt(),
                                                           parameters.at(4),
                                                           nullptr);
                scope.clear();
                analyticsViewer->showFullScreen();
            } else {
                ui->lblServerSetup->setText("Empty result set!");
            }
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
        QString link = parameters.at(4).simplified();
        QDesktopServices::openUrl(QUrl("https://www.dicomlibrary.com?study=" + link, QUrl::TolerantMode));

    } catch(...) {
        ui->lblServerSetup->setText("Error: Invalid patient selection.");
    }
}


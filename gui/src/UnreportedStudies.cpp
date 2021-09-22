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
    ui->txtSearchPatient->setFocus();

    //Load studies data
    loadStudyTable(userId);
}

UnreportedStudies::~UnreportedStudies(){

    delete ui;
}


void UnreportedStudies::loadStudyTable(int userId){

    this->userId = QString::number(userId);
    ui->lblServerSetup->setText("Fetching user pool, please wait...");

    SirenSQLQuery buildPool;
    buildPool.addProjectionAttribute("tableName");
    buildPool.addTable("Pool");
    buildPool.addWhereAttribute("userId = " + QString::number(userId));

    //Lock screen
    ui->txtSearchPatient->setEnabled(false);
    ui->btnBack->setEnabled(false);
    ui->btnNewDiagnosis->setEnabled(false);
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
                                + records.fetchByColumn(row, "Filename"));
            newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));
            ui->tblInfo->setItem(row, column, newItem);
        }
    }

    //Unlock screen
    ui->txtSearchPatient->setEnabled(true);
    ui->btnBack->setEnabled(true);
    ui->btnNewDiagnosis->setEnabled(true);
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


    QStringList lines;

    for (size_t x = 0; x < rowIds.size(); x++){
        lines.append(records.at(x));
    }

    //Populate table
    for (int row = 1; row < lines.size(); row++){
        ui->tblInfo->insertRow(row-1);
        QStringList tuple = lines.at(row).split(",");
        for (int column = 0; column < tuple.size(); column++){
            QTableWidgetItem *newItem = new QTableWidgetItem(tuple.at(column));
            newItem->setToolTip(tuple.at(0) + "," + tuple.at(3) + "," + tuple.at(2));
            newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));
            ui->tblInfo->setItem(row-1, column, newItem);
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
* @param arg1 The searched name
*/
void UnreportedStudies::on_txtSearchPatient_textChanged(const QString &arg1){

    std::vector<int> rowIds;
    if (records.size() > 0){
        rowIds.push_back(0);
    }

    for (int x = 1; x < records.size()-1; x++){
        if (arg1.toStdString().empty() || records.at(x).contains(arg1, Qt::CaseInsensitive)){
            rowIds.push_back(x);
        }
    }

    populateStudyTable(rowIds);
    rowIds.clear();
}

/**
* Shows the QueryParameters screen with the selected patient data.
*
*/
void UnreportedStudies::on_btnViewStudy_clicked(){

    QModelIndexList selectedList = ui->tblInfo->selectionModel()->selectedRows();
    uint8_t pos = ui->tblInfo->currentRow();

    if (selectedList.size() != 0){
        try{
            QTableWidgetItem *item = ui->tblInfo->item(pos, 0);
            QStringList parameters = item->toolTip().split(",");
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
        } catch(...) {
            ui->lblServerSetup->setText("Error: Invalid patient selection.");
        }
    }else{
        ui->lblServerSetup->setText("Error: Please, select a line with a patient.");
    }
    ui->lblServerSetup->setText("Connected to the Server!");



}

/**
* Shows the AddStudy screen.
*
*/
void UnreportedStudies::on_btnNewDiagnosis_clicked(){

    //@to-do
}


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
               buildTable.addProjectionList({"p.*", "Id", "Patient_Name", "Filename", "p.tableName TableName"});
               buildTable.addTable(poolTables.fetchByColumnId(x, 0) + " u ");
               buildTable.addJoinList({"Pool p"}, {"p.imageId = u.id"});
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

        ui->lblServerSetup->setText("Connected to the Server!");
    }
}



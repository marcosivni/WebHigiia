#include "QueryParameters.h"
#include "ui_QueryParameters.h"



QueryParameters::QueryParameters(int32_t studyId, QString tableName, QString imageFile, QWebSocket *webSocket, int32_t userId, FeatureVector oq, QString link, QString numberK, QString clusterSize, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QueryParameters){

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

    this->webSocket = webSocket;
    this->filename = imageFile;
    this->tableName = tableName;
    this->studyId = studyId;
    this->userId = userId;
    this->oq = oq;
    this->link = link;
    ui->txtNumberNeighbors->setText(numberK);
    ui->txtNumberDiversity->setText(clusterSize);

    oberonViewer = nullptr;

    //Locking widgets...
    lockWidgets();
    ui->lblServerSetup->setText("Requesting, please wait ...");


    if (QFileInfo::exists(WFS_NAME +imageFile) && QFileInfo(WFS_NAME + imageFile).isFile()){
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

    ui->centralwidget->setEnabled(false);
}

void QueryParameters::unlockWidgets(){

    ui->centralwidget->setEnabled(true);
}

QueryParameters::~QueryParameters(){

    if (oberonViewer != nullptr){
        delete (oberonViewer);
    }
    oberonViewer = nullptr;
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
    buildScope.addWhereAttribute("tableName = '" + tableName + "'");

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

    #if NO_USER_PAM
    {
        on_btnViewStudy_clicked();
    }
    #endif
}

void QueryParameters::on_btnViewStudy_clicked(){

    unlockWidgets();

    similarityAttribute = ui->cbxSimilarityAttribute->currentText();

    if(ui->txtNumberNeighbors->text().toInt() <= 0){
        ui->lblServerSetup->setText("Invalid number of neighbors!");
        ui->txtNumberNeighbors->setFocus();
        return;
    } else {
        ui->lblServerSetup->setText("Building the query, please wait...");

        //Locking widgets...
        lockWidgets();

        //Building querry
        QString vTable, tblName, queryObjectValue, condition;
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
            vTable   = " SELECT * FROM " + tableName;
            vTable  += " WHERE " + tableName +"."+ ui->cbxHypothesis->currentText();
            vTable  += " >= "    + ui->txtMinimum->text();
            vTable  += " AND "   + tableName +"."+ ui->cbxHypothesis->currentText();
            vTable  += " <= "    + ui->txtMaximum->text();
            vTableName = vTable;
            tblName = "temp";
        } else {
            tblName = tableName;
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
        connect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state05);
        webSocket->sendBinaryMessage(queryT.generateQuery().toStdString().c_str());
    }

}


void QueryParameters::state05(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &QueryParameters::state05);
    ui->lblServerSetup->setText("CBIR is ready!");

    MedicalImageTable rSet(Util::toStringList(message.split('\n')));

    if (rSet.size()){
        oberonViewer = new OberonViewer(ui->cbxHypothesis->currentIndex(),
                                                      scopeAttributes,
                                                      tableName,
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
                                                      link,
                                                      nullptr);
        connect(oberonViewer, &OberonViewer::finished, this, &QueryParameters::state06);
        oberonViewer->showFullScreen();
    } else {
        ui->lblServerSetup->setText("Empty result set!");
    }

    scopeAttributes.clear();
    vTableName.clear();

    unlockWidgets();
}

void QueryParameters::state06(){

    disconnect(oberonViewer, &OberonViewer::finished, this, &QueryParameters::state06);
    emit finished();
    on_btnClose_clicked();
}

void QueryParameters::on_btnClose_clicked(){

    this->close();
}

void QueryParameters::on_btnPACS_clicked(){

    QDesktopServices::openUrl(QUrl(PACS_BASE_URL + link, QUrl::TolerantMode));
}


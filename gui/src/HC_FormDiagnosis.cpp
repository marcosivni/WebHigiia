#include "HC_FormDiagnosis.h"
#include "ui_HC_FormDiagnosis.h"

FormDiagnosis::FormDiagnosis(QString imageFilename,
                             uint16_t imageId,
                             QString tableName,
                             QString obsText,
                             QWebSocket *webSocket,
                             uint16_t userId, QString mask,
                             QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FormDiagnosis)
{
    ui->setupUi(this);


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

    // move window to center
    move ( x, y );

    //load query image
    Image *currentImage = Util::openImage(imageFilename, mask);
    if (((currentImage->type() == Image::KRL) || (currentImage->type() == Image::DICOM)) && !currentImage->windowedPixels()){
        Image *bck = currentImage->windowing(currentImage->getWindowWidth(), currentImage->getWindowCenter());
        delete (currentImage);
        currentImage = bck;
    }
    QImage *aux = Util::convertImageToQImage(currentImage);
    ui->lblImage->clear();
    ui->lblImage->setPixmap(QPixmap::fromImage(aux->scaled(800, 800)));
    delete (aux);
    delete (currentImage);
    //

    this->imageId = imageId;
    this->tableName = tableName;
    this->webSocket = webSocket;
    this->userId = userId;
    this->obsText = obsText;
    ui->txtDiagnosis->setText(ui->txtDiagnosis->text() + "(" + obsText + "):");


    ui->lblStatus->setText("Fetching expected diagnoses...");

    SirenSQLQuery buildDiagnoses;
    buildDiagnoses.addProjectionAttribute("diagnosis");
    buildDiagnoses.addTable("H_Diagnoses");
    buildDiagnoses.addWhereAttribute("tableName = '" + tableName + "'");

    //Lock buttons and widgets
    ui->centralwidget->setEnabled(false);

    //Blocking call - State Machine
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &FormDiagnosis::state01);
    webSocket->sendBinaryMessage(buildDiagnoses.generateQuery().toStdString().c_str());
}

void FormDiagnosis::state01(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &FormDiagnosis::state01);

    ResultTable scopeTable(Util::toStringList(message.split('\n')));

    //Load expected diagnoses...
    for (int x = 0; x < scopeTable.size(); x++){
        ui->cbxDiagnosis->addItem(scopeTable.fetchByColumnId(x, 0));
    }

    ui->centralwidget->setEnabled(true);
    ui->lblStatus->setText("CBIR is ready. Test stage: " + obsText.split(":").at(1).simplified() + ". Waiting your diagnosis report ...");
}


FormDiagnosis::~FormDiagnosis(){

    delete ui;
}

void FormDiagnosis::on_btnSave_clicked(){

    QString values;

    values.append(ui->txtDiagnosis->text());
    values.append("\nClass = " + ui->cbxDiagnosis->currentText());

    QList<QRadioButton *> allButtons = this->findChildren<QRadioButton *>();
    for (int x = 0; x < allButtons.size(); x++){
        QRadioButton *rdb = allButtons.at(x);
        if (rdb->isChecked()){
            values.append("\nConfidence level = " + rdb->text());
        }
    }
    ui->lblStatus->setText("Saving, please wait...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &FormDiagnosis::state02);
    webSocket->sendBinaryMessage(buildProvenanceInsert(userId, imageId, tableName, "diagnosis report", values, obsText).toStdString().c_str());
}

void FormDiagnosis::state02(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &FormDiagnosis::state02);
    ui->lblStatus->setText("Saved! Connected to the Server!");

    emit finished();
}

QString FormDiagnosis::buildProvenanceInsert(uint32_t userId, uint32_t imageId, QString tableName, QString op, QString opValue, QString obs){

    QString insertStatement;

    insertStatement =  "INSERT INTO H_Provenance (userId, imageId, tStamp, tableName, operation, opValue, observation";
    insertStatement += ") VALUES ('" + QString::number(userId) + "'";
    insertStatement += ", '" + QString::number(imageId) + "'";
    insertStatement += ", '" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "'";
    insertStatement += ", '" + QString("U_" + tableName) + "'";
    insertStatement += ", '" + op + "'";
    insertStatement += ", '" + opValue + "'";
    insertStatement += ", '" + obs + "')";

    return insertStatement;
}


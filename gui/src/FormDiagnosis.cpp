#include "FormDiagnosis.h"
#include "ui_FormDiagnosis.h"

FormDiagnosis::FormDiagnosis(Image queryImage, uint16_t imageId, QString tableName, QWebSocket *webSocket, uint16_t userId, QWidget *parent ) :
    QMainWindow(parent),
    ui(new Ui::FormDiagnosis)
{
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

    // move window to center
    move ( x, y );

    //load query image
    QImage *aux = Util::convertImageToQImage(&queryImage);
    ui->lblImage->clear();
    ui->lblImage->setPixmap(QPixmap::fromImage(aux->scaled(600, 600)));
    delete (aux);
    //

    this->imageId = imageId;
    this->tableName = tableName;
    this->webSocket = webSocket;
    this->userId = userId;

    SirenSQLQuery buildScope;
    buildScope.addProjectionAttribute("attributeName");
    buildScope.addTable("Scope");
    buildScope.addWhereAttribute("tableName = '" + tableName + "'");


    //Lock buttons and widgets
    ui->frmButtons->setEnabled(false);
    ui->lytScrollArea->setEnabled(false);
    ui->lblStatus->setText("Loading report attributes, please wait...");


    //Blocking call - State Machine
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &FormDiagnosis::state01);
    webSocket->sendBinaryMessage(buildScope.generateQuery().toStdString().c_str());
}

void FormDiagnosis::state01(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &FormDiagnosis::state01);

    ResultTable scopeTable(Util::toStringList(message.split('\n')));

    //Create QLabels for the query scope
    for (int x = 0; x < scopeTable.size(); x++){
        ui->lytScrollArea->addWidget(new QLabel("- ITEM: " + scopeTable.fetchByColumnId(x, 0), this));
        attributeList.append(scopeTable.fetchByColumnId(x, 0));
        ui->lytScrollArea->addWidget(new QPlainTextEdit("<Empty>", this));
    }
    ui->lytScrollArea->addWidget(new QLabel("Details", this));
    ui->lytScrollArea->addWidget(new QPlainTextEdit("<Empty>", this));

    ui->frmButtons->setEnabled(true);
    ui->lytScrollArea->setEnabled(true);

    ui->lblStatus->setText("Connected to the Server!");
}

FormDiagnosis::~FormDiagnosis(){

    ui->lytScrollArea->deleteLater();
    delete ui;
}

void FormDiagnosis::on_btnExit_clicked(){

    this->close();
}


void FormDiagnosis::on_btnSave_clicked(){

    QString values;

    for (int x = 0; x < (attributeList.size()); x++){
        if (x > 0){
            values.append("\n");
        }
        values.append(attributeList.at(x) + " = " + qobject_cast<QPlainTextEdit*>(ui->lytScrollArea->itemAt((2*x)+1)->widget())->toPlainText());
    }

    QList<QRadioButton *> allButtons = this->findChildren<QRadioButton *>();
    for (int x = 0; x < allButtons.size(); x++){
        QRadioButton *rdb = allButtons.at(x);
        if (rdb->isChecked()){
            values.append("\nConfidence level = " + rdb->text());
        }
    }

    ui->lblStatus->setText("Saving, please wait...");
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &FormDiagnosis::state02);
    webSocket->sendBinaryMessage(Util::buildProvenanceInsert(userId, imageId, tableName, "diagnosis report", values,
                                                             qobject_cast<QPlainTextEdit*>(ui->lytScrollArea->itemAt((2*attributeList.size())+1)->widget())->toPlainText()).toStdString().c_str());
}

void FormDiagnosis::state02(QByteArray message){

    ui->lblStatus->setText("Saved! Connected to the Server!");
    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &FormDiagnosis::state02);
}


#include "Login.h"
#include "ui_Login.h"

Login::Login(QWebSocket *webSocket, QWidget *parent) : QMainWindow(parent), ui(new Ui::Login) {

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

    this->webSocket = webSocket;
    ui->txtUser->setFocus();
}

Login::~Login() {

    Util::removeDirectoryAndContent();
    delete ui;
}

void Login::changeEvent(QEvent *e) {

    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Login::on_btnLogin_clicked(){

    if (webSocket->isValid()){
        state01();
    } else {
        QUrl url{QString("ws://" + ui->txtHostname->text() + ":" + ui->txtPort->text())};

        ui->txtHostname->setEnabled(false);
        ui->txtPort->setEnabled(false);
        ui->lblServerSetup->setText("Requesting connection, please wait...");

        //Blocking call - State Machine
        disconnect(webSocket, &QWebSocket::connected, this, &Login::state01);
        connect(webSocket, &QWebSocket::connected, this, &Login::state01);
        webSocket->open(url);
    }
}


void Login::state01(){

    disconnect(webSocket, &QWebSocket::connected, this, &Login::state01);

    if (!webSocket->isValid()){;
        ui->lblServerSetup->setText("Server is unavailable!");
        ui->txtHostname->setEnabled(true);
        ui->txtPort->setEnabled(true);
        ui->txtAttribute->setEnabled(true);
        ui->txtTable->setEnabled(true);
        ui->txtKNN->setEnabled(true);
        ui->txtBridgeK->setEnabled(true);
        ui->txtUser->clear();
        ui->txtPass->clear();
        ui->txtUser->setFocus();
    } else {

        if ((ui->txtUser->text().size() != 0) && (ui->txtPass->text().size() != 0)){
            ui->lblServerSetup->setText("Logging in, please wait...");

            //Locking widgets
            ui->txtHostname->setEnabled(false);
            ui->txtPort->setEnabled(false);
            ui->txtAttribute->setEnabled(false);
            ui->txtTable->setEnabled(false);
            ui->txtKNN->setEnabled(false);
            ui->txtBridgeK->setEnabled(false);

            //Create query -- Lazy auth until QSsl (client side) gets full support on Qt WASM
            SirenSQLQuery buildLogin;
            buildLogin.addProjectionAttribute("id");
            buildLogin.addTable("Login");
            buildLogin.addWhereListAnd( {("nick = '" + ui->txtUser->text() + "'") , ("pass = '" + ui->txtPass->text() + "'")} );

            //Blocking call - State Machine
            connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Login::state02);
            webSocket->sendBinaryMessage(buildLogin.generateQuery().toStdString().c_str());

        } else {
            ui->txtUser->clear();
            ui->txtPass->clear();
            ui->txtUser->setFocus();
            ui->lblServerSetup->setText("Empty username/password!");
        }
    }
}

void Login::on_btnSetup_clicked(){

    ui->txtHostname->setEnabled(true);
    ui->txtPort->setEnabled(true);
    ui->txtAttribute->setEnabled(true);
    ui->txtTable->setEnabled(true);
    ui->txtKNN->setEnabled(true);
    ui->txtBridgeK->setEnabled(true);
}


void Login::on_btnExit_clicked(){

    this->close();
}

void Login::state02(QByteArray message){

    disconnect(webSocket, &QWebSocket::binaryMessageReceived, this, &Login::state02);

    //Unlocking widgets
    ui->btnLogin->setEnabled(true);
    ui->btnExit->setEnabled(true);

    if (message.toStdString().empty()){
        ui->lblServerSetup->setText("Invalid user or password!");
        ui->txtUser->clear();
        ui->txtPass->clear();
        ui->txtUser->setFocus();
    } else {
        ResultTable idTable(Util::toStringList(message.split('\n')));
        ui->lblServerSetup->setText("Connected to the Server!");

        if (idTable.size()){
            UnreportedStudies *unreportedStudies = new UnreportedStudies(webSocket,
                                                                         idTable.fetchByColumnId(0, 0).toInt(),
                                                                         ui->txtTable->text(),
                                                                         ui->txtAttribute->text(),
                                                                         ui->txtKNN->text(),
                                                                         ui->txtBridgeK->text(),
                                                                         nullptr);
            unreportedStudies->show();
        } else {
            ui->lblServerSetup->setText("Invalid user or password!");
        }

    }
}




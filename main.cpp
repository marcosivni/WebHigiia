//Qt includes
#include <QApplication>
#include <QWebSocket>



//GUI includes
#include <Login.h>

#include <DcmLib.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWebSocket webSocket;

    Login l(&webSocket);
    l.show();

    if (webSocket.isValid()){
        webSocket.close();
    }

//    DCMImage *dcm = new DCMImage("/home/ivni/Documents/FAPESP/ESPINHA/Imgs/01-T1-S07.dcm");
//    delete (dcm);

    return a.exec();
}

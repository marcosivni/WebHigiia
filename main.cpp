//Qt includes
#include <QApplication>
#include <QWebSocket>

//GUI includes
#include <Login.h>
#include <Analytics.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWebSocket webSocket;

    Login l(&webSocket);
    l.show();

    if (webSocket.isValid()){
        webSocket.close();
    }

//    Analytics an;
//    an.showMaximized();

    return a.exec();
}

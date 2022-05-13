//Qt includes
#include <QApplication>
#include <QWebSocket>
#include <QDebug>

//GUI includes
#include <Login.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWebSocket webSocket;

    Login l(&webSocket);
    l.show();

    if (webSocket.isValid()){
        webSocket.close();
    }

    return a.exec();
}

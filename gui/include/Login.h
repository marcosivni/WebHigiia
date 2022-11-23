#ifndef LOGIN_H
#define LOGIN_H

//Qt includes
#include <QMainWindow>
#include <QDesktopWidget>
#include <QWebSocket>
#include <QUrl>

//WebHigiia includes
#include <ResultTable.h>
#include <SirenSqlQuery.h>
#include <UnreportedStudies.h>

namespace Ui {
    class Login;
}

class Login : public QMainWindow { Q_OBJECT

    private:
        Ui::Login *ui;
        QWebSocket *webSocket;

    private slots:
        void on_btnLogin_clicked();
        void on_btnExit_clicked();
        void on_btnSetup_clicked();

    protected:
        void changeEvent(QEvent *e);

    public:
        explicit Login(QWebSocket *webSocket, QWidget *parent = 0);
        ~Login();

    public slots:
        void state01();
        void state02(QByteArray message);
};


#endif // LOGIN_H

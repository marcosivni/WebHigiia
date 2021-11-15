#ifndef FORMDIAGNOSIS_H
#define FORMDIAGNOSIS_H

//Qt includes
#include <QMainWindow>
#include <QDesktopWidget>
#include <QWebSocket>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>


//Higiia includes
#include <SirenSqlQuery.h>
#include <ResultTable.h>

namespace Ui {
class FormDiagnosis;
}

class FormDiagnosis : public QMainWindow{
    Q_OBJECT

    private:
        Ui::FormDiagnosis *ui;
        QStringList attributeList;
        QWebSocket *webSocket;
        QString tableName, obsText;
        uint16_t imageId, userId;

    private:
        void state01(QByteArray message);
        void state02(QByteArray message);

    public:
        explicit FormDiagnosis(Image queryImage,
                               uint16_t imageId,
                               QString tableName,
                               QString obsText,
                               QWebSocket *webSocket,
                               uint16_t userId = -1,
                               QWidget *parent = nullptr);
        ~FormDiagnosis();

    signals:
        void finished();

    private slots:
        //void on_btnExit_clicked();
        void on_btnSave_clicked();
};

#endif // FORMDIAGNOSIS_H

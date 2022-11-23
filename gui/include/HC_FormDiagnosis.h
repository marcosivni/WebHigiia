#ifndef HC_FORMDIAGNOSIS_H
#define HC_FORMDIAGNOSIS_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QWebSocket>
#include <QLabel>

//WebHigiia includes
#include <SirenSqlQuery.h>
#include <ResultTable.h>

namespace Ui {
    class FormDiagnosis;
}

class FormDiagnosis : public QMainWindow
{
    Q_OBJECT

    private:
        Ui::FormDiagnosis *ui;
        QWebSocket *webSocket;
        QString tableName, obsText;
        uint16_t imageId, userId;

    private:
        QString buildProvenanceInsert(uint32_t userId, uint32_t imageId, QString tableName, QString op, QString opValue, QString obs);
        void state01(QByteArray message);
        void state02(QByteArray message);

    signals:
        void finished();

    public:
        explicit FormDiagnosis(QString imageFilename,
                           uint16_t imageId,
                           QString tableName,
                           QString obsText,
                           QWebSocket *webSocket,
                           uint16_t userId,
                           QString mask = QString(),
                           QWidget *parent = nullptr);
        ~FormDiagnosis();

    private slots:
        void on_btnSave_clicked();
};

#endif // HC_FORMDIAGNOSIS_H

#ifndef UNREPORTEDSTUDIES_HPP
#define UNREPORTEDSTUDIES_HPP

//Qt includes
#include <QMainWindow>
#include <QDesktopWidget>
#include <QStringList>
#include <QTableWidgetItem>
#include <QWebSocket>

//Higiia includes
#include <MedicalImageTable.h>
#include <SirenSqlQuery.h>
#include <QueryParameters.h>

namespace Ui {
    class UnreportedStudies;
}


class UnreportedStudies : public QMainWindow{ Q_OBJECT

    private:
        Ui::UnreportedStudies *ui;
        QWebSocket *webSocket;
        QStringList records;
        QString userId;
        bool provenance;


    private:
        void loadStudyTable(int userId);
        void populateStudyTable(MedicalImageTable records);
        void populateStudyTable(std::vector<int> rowIds);


    private slots:
        void on_btnNext_clicked();
        void on_btnBack_clicked();
        void on_btnClose_clicked();;
        void on_txtSearchPatient_textChanged(const QString &arg1);
        void on_btnViewStudy_clicked();
        void on_btnNewDiagnosis_clicked();

    public slots:
        void state01(QByteArray message);
        void state02(QByteArray message);

    protected:
        void changeEvent(QEvent *e);

    public:
        explicit UnreportedStudies(QWebSocket *webSocket, int userId, bool provenance = false, QWidget *parent = 0);
        ~UnreportedStudies();
};

#endif // UNREPORTEDSTUDIES_H

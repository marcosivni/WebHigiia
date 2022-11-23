#ifndef UNREPORTEDSTUDIES_HPP
#define UNREPORTEDSTUDIES_HPP

//Hermes includes
#include <BasicArrayObject.h>

//Qt includes
#include <QMainWindow>
#include <QDesktopWidget>
#include <QStringList>
#include <QTableWidgetItem>
#include <QWebSocket>
#include <QRegularExpression>
#include <QUrl>

//WebHigiia includes
#include <MedicalImageTable.h>
#include <SirenSqlQuery.h>
#include <QueryParameters.h>
#include <OberonViewer.h>
#include <Analytics.h>


//Provenance-related stuff
#if M_PROV
    #include <HC_FormDiagnosis.h>
#endif


namespace Ui {
    class UnreportedStudies;
}


class UnreportedStudies : public QMainWindow{ Q_OBJECT

    private:
        Ui::UnreportedStudies *ui;
        QWebSocket *webSocket;
        MedicalImageTable records;
        QString userId, scope, tableName, attName, numberK, clusterSize;
        FeatureVector oq;

        //Mask stuff
        std::map<QString, QString> mapNameToMask;

        //Provenance-related stuff
        #if M_PROV
            FormDiagnosis *diagnosisForm;
        #endif


    private:
        void loadStudyTable(const int userId);
        void populateStudyTable(MedicalImageTable records);
        void populateStudyTable(const std::vector<int> rowIds);


    private slots:
        void on_btnNext_clicked();
        void on_btnBack_clicked();
        void on_btnClose_clicked();;
        void on_txtSearchPatient_textChanged(const QString &arg1);
        void on_btnViewStudy_clicked();
        void on_btnPACS_clicked();

    public slots:
        void state01(QByteArray message);
        void state02();
        void state03();
        void state04(QByteArray message);
        void state05();
        void state06(QByteArray message);
        void state07(QByteArray message);
        void state08(QByteArray message);
        void state09(QByteArray message);
        void state10();

    protected:
        void changeEvent(QEvent *e);

    public:
        explicit UnreportedStudies(QWebSocket *webSocket, const int userId,
                                   QString tableName = "",
                                   QString attName = "",
                                   QString numberK = "",
                                   QString clusterSize = "",
                                   QWidget *parent = 0);
        ~UnreportedStudies();
};

#endif // UNREPORTEDSTUDIES_H

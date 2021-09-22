#ifndef QUERYPARAMETERS_HPP
#define QUERYPARAMETERS_HPP

//Qt includes
#include <QMainWindow>
#include <QDesktopWidget>
#include <QWebSocket>
#include <QFile>
#include <QDir>

//Higiia includes
#include <Util.h>
#include <OberonViewer.h>
#include <Analytics.h>
#include <MedicalImageTable.h>
#include <SirenSqlQuery.h>

namespace Ui {
    class QueryParameters;
}

class QueryParameters : public QMainWindow{ Q_OBJECT

	private:
        Util::SEARCH_TYPE searchType;
		Ui::QueryParameters *ui;
        QWebSocket *webSocket;
        QString filename, tableName, vTableName, similarityAttribute, scopeAttributes;
        FeatureVector oq;
        int32_t studyId;
        int32_t userId;
        //Store both query and result set before the insertion of provenance data
        QByteArray bufferRSet;
        QString provQuery;

    private:
        void loadScope();

	private slots:
        void state01(QByteArray message);
        void state02(QByteArray message);
        void state03(QByteArray message);
        void state04(QByteArray message);
        void state05(QByteArray message);
        void state06(QByteArray message);
        void state07(QByteArray message);
        void state08(QByteArray message);

        void state09(QByteArray message);
        void state10(QByteArray message);

        void state11(QByteArray message);
        void state12(QByteArray message);

        void on_btnClose_clicked();
        void on_btnViewStudy_clicked();
        void on_btnAnalytics_clicked();

    public:
        explicit QueryParameters(int32_t studyId, QString tableName, QString imageFile, QWebSocket *webSocket, int32_t userId = -1, QWidget *parent = 0);
		~QueryParameters();
};

#endif // QUERYPARAMETERS_H

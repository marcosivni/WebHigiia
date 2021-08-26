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
        int studyId;

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

        void on_btnClose_clicked();
        void on_btnViewStudy_clicked();
        void on_btnAnalytics_clicked();

    public:
        explicit QueryParameters(uint16_t studyId, QString tableName, QString imageFile, QWebSocket *webSocket, QWidget *parent = 0);
		~QueryParameters();
};

#endif // QUERYPARAMETERS_H

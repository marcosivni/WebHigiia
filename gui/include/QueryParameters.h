#ifndef QUERYPARAMETERS_HPP
#define QUERYPARAMETERS_HPP

//Qt includes
#include <QMainWindow>
#include <QDesktopWidget>
#include <QWebSocket>

//WebHigiia includes
#include <Util.h>
#include <OberonViewer.h>
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
        QString filename, tableName, vTableName, similarityAttribute, scopeAttributes, link;
        FeatureVector oq;
        int32_t studyId;
        int32_t userId;
        OberonViewer *oberonViewer;

    private:
        void loadScope();
        void lockWidgets();
        void unlockWidgets();

	private slots:
        void state01(QByteArray message);
        void state02(QByteArray message);
        void state03(QByteArray message);
        void state04(QByteArray message);
        void state05(QByteArray message);
        void state06();

        void on_btnClose_clicked();
        void on_btnViewStudy_clicked();
        void on_btnPACS_clicked();

    signals:
        void finished();

    public:
        explicit QueryParameters(int32_t studyId, QString tableName, QString imageFile, QWebSocket *webSocket, int32_t userId, FeatureVector oq, QString link, QString numberK, QString clusterSize, QWidget *parent = 0);
		~QueryParameters();
};

#endif // QUERYPARAMETERS_H

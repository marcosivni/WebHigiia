#ifndef ANALYTICS_H
#define ANALYTICS_H

//Stl includes
#include <limits>

//Qt includes
#include <QMainWindow>
#include <QDesktopWidget>
#include <QWebSocket>
#include <QFileInfo>
#include <QDir>

//Qt Chart includes - stat lin in libqt5charts5-dev (apt-get)
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLegendMarker>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtCore/QtMath>

//Hermes includes
#include <BasicArrayObject.h>

//Higiia includes
#include <Util.h>
#include <ScopeCaption.h>
#include <QPushButtonAlter.h>
#include <MedicalImageTable.h>
#include <SirenSqlQuery.h>
#include <OberonViewer.h>
#include <FormDiagnosis.h>

//Eigen includes
#include <Eigen/Sparse>
#include <Eigen/Eigenvalues>


namespace Ui {
    class Analytics;
}

QT_CHARTS_USE_NAMESPACE

class Analytics : public QMainWindow {
    Q_OBJECT

    private:
        bool vTable;
        QString tableName, oqFileName, simAttribute, metricName, vTableName;
        MedicalImageTable rSet, tempRInfset, rInfset, newRSet;
        FeatureVector oq, oqRed;
        QStringList scopeAtts;
        Util::SEARCH_TYPE searchType;

        int counterImgs, iteratorRSet;
        //Mapper rSet -> rInfSet
        QMultiMap<uint16_t, uint16_t> mapInfluencedRows;

        //Chart
        QStringList targets;
        QChartView *chart;
        QScatterSeries **seriesByTarget;
        int nSeries;

        //Chart points
        std::vector< std::pair<uint32_t, QPointF> > mapPointToRowId;
        QPointF currentClosest;
        FeatureVectorList dataset2D;
        QWebSocket *webSocket;

        //Scope caption
        ScopeCaption scopeCaption;

        //Provenance info
        int32_t oqId, userId;
        OberonViewer *bufferViewer;
        QString bufferQuery;

    private:
        std::vector<std::pair<double, double>> spiral(const uint16_t N, double *farthestInAxis = nullptr);
        double L2Distance(QPointF p1, QPointF p2);

        void loadScope();
        void loadInfluencedImages();
        void downloadNonInfluencedImages();
        void downloadOq();
        void fillQueryData();
        void fillNeighborImage(int rSetRowId);
        void populateData();

        void lockWidgets();
        void unlockWidgets();

        //State-machine for sync data loading
        void state01(QByteArray message);
        void state02(QByteArray message);
        void state03(QByteArray message);
        void state04(QByteArray message);
        void state05(QByteArray message);
        void state06(QByteArray message);
        void state07(QByteArray message);

        //State-machine for provenance collection
        void state08(QByteArray message);
        void state09(QByteArray message);

        QString buildOqScope();
        QString buildOqStats();
        QString buildNeighborScope(MedicalImageTable tempSet, QStringList scopeList, int rowId);
        QString buildNeighborStats(int rowId, double distToOq);


        FeatureVectorList pca2D();
        void unitaryScale(FeatureVectorList *dataset);
        double L2ToOqPCA(QPointF p1);

    public:
        explicit Analytics(QString tableName,
                           QString queryCenterName,
                           FeatureVector oq,
                           QString simAttribute,
                           QString metricName,
                           MedicalImageTable rSet,
                           bool vTable,
                           QString vTableName,
                           QWebSocket *webSocket,
                           int32_t oqId,
                           int32_t userId,
                           QWidget *parent = nullptr);
        ~Analytics();

    private:
        Ui::Analytics *ui;


    private slots:
        void handleClickedPoint(const QPointF &point);
        void on_btnUpdate_clicked();
        void on_btnScopeOi_clicked();
        void on_btnStatsOi_clicked();
        void on_btnScopeOq_clicked();
        void on_btnStatsOq_clicked();
        void on_btnSearchOi_clicked();
        void on_btnClose_clicked();
        void on_btnSearchOq_clicked();
        void on_btnNewDiagnosis_clicked();
};

#endif // ANALYTICS_H

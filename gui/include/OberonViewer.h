#ifndef OBERONVIEWER_H
#define OBERONVIEWER_H

// Qt includes
#include <QMainWindow>
#include <QDesktopWidget>
#include <QFile>
#include <QPrinter>
#include <QPageSize>
#include <QPagedPaintDevice>
#include <QFileDialog>
#include <QtGui>
#include <QWebSocket>
#include <QMultiMap>


//WebHigiia includes
#include <QPushButtonAlter.h>
#include <Util.h>
#include <ScopeCaption.h>
#include <WindowingInterval.h>
#include <MedicalImageTable.h>
#include <SirenSqlQuery.h>

//Provenance-related stuff
#if M_PROV
    #include <HC_FormDiagnosis.h>
#endif

namespace Ui {
    class OberonViewer;
}


class OberonViewer : public QMainWindow { Q_OBJECT

	private:
        //Search type
        Util::SEARCH_TYPE searchType;
        //Button vector for influenced images
        std::vector<QPushButton *> thumbsImagesDown;
        //Button vector for influencer images
        std::vector<QPushButton *> thumbsImagesRight;
        //Button vector for relevant images
        std::vector<QPushButton *> thumbsImagesMyList;
        //Global variable for the state-machine (sync purpose - WebAssembly)
        uint16_t counterImgsRightPanel, counterImgsDownPanel;
        //Result sets:
        // - rSet: Result set with influencers only
        // - rInfSet: Result set with influenced images
        // - temp: Swap for splitting influencers and influenced images
        MedicalImageTable rSet, rInfset, tempRInfset;
        //Search attributes (required for relevance feedback cycles)
        QString simAttribute, tableName, oqFileName, scopeAttributes, vTableName, metricName, userId;
        FeatureVector oq;
        uint32_t oqId;
        bool vTable;
        //Mapper rSet -> rInfSet
        QMultiMap<uint16_t, uint16_t> mapInfluencedRows;
        //Global variable for tabbing
        uint16_t diversityIndex;
        //Mapper Id -> filename
        std::map<uint16_t, std::string> mapOidToNames;
        //Id vector of relevant images
        std::vector<uint32_t> myList;
        //Tabs for nested results
        QWidget *tabs[2];
        //Id vector of non-relevant images (session variable)
        std::vector<uint32_t> removedListOfIds;
        //Query image
        Image *queryImage;
        //Image displayed for analysis (screen center)
        Image *currentImage;

        //Scope caption
        ScopeCaption scopeCaption;
        //Thumbnail icon size
        QSize icoThumbSize;
        //Web Socket
        QWebSocket *webSocket;
        //UI
        Ui::OberonViewer *ui;

        //Viewing info
        QString link, kBridge;

        //Mask stuff
        std::map<QString, QString> mapNameToMask;

        //Provenance-related stuff
        #if M_PROV
            FormDiagnosis *diagnosisForm;
        #endif

    private:
        //Auxiliary methods for loading images into the file system
        void downloadOqMask();
        void downloadNonInfluencedMasks();
        void downloadInfluencedMasks();
        void downloadOq();
        void downloadNonInfluencedImages();
        void loadInfluencedImages();
        void downloadInfluencedImages();

        //Methods for populating the UI
        QString toolTipBuilder(MedicalImageTable tempSet, int row);
        void loadThumbnailsRight(MedicalImageTable tempSet);
        void loadThumbnailsMyList();
        void tabManager(uint8_t operation);
        void loadThumbnailsDown(QList<uint16_t> rowIds);

        //Methods for image manipulation
        void showWindowing();
        void adjustSliders();

        //Methods for destroying private variables
        void clearAllThumbnails();
        void clearResultSets();

        //Methods for initialization
        void fillThumbnails();
        void splitResultSets();

    private slots:
        //State machine slots for loading result sets and images
        void state01(QByteArray message);
        void state02(QByteArray message);
        void state03(QByteArray message);
        void state04(QByteArray message);
        void state05(QByteArray message);

        //State machine slots for relevance feedback cycles
        void state06(QByteArray message);
        void state07(QByteArray message);

        //State machine slots for masks
        void state08(QByteArray message);
        void state09(QByteArray message);
        void state10(QByteArray message);

        //State-machine for provenance stuff
        void state11();
        void state12();

        //Slots for user-interaction
        void unRelevantImageClickedRight();
        void fillQueryCenter(Image *src, uint32_t scale = 600);
        void changeCenterImage();
        void diversityImageClicked();
        void relevantImageClicked();
        void relevantImageClickedAddMyList();
        void unRelevantImageClickedDown();
        void unRelevantImageClickedMyList();

        //UI-iteraction slots
        void on_btnZoomIn_clicked();
        void on_btnExit_clicked();
        void on_btnZoomOut_clicked();
        void on_btnPDF_clicked();
        void on_btnQueryCenter_clicked();
        void on_btnRestart_clicked();
        void on_btnFeedback_clicked();
        void on_sliderCenter_sliderReleased();
        void on_sliderWidth_sliderReleased();
        void on_cbxWindowing_currentIndexChanged(int index);
        void on_btnPACS_clicked();

    signals:
        void finished();

    public:
        explicit OberonViewer(bool vTable,
                              QString scopeAttributes,
                              QString tableName,
                              QString queryCenterName,
                              QString similarityAttribute,
                              FeatureVector queryCenter,
                              MedicalImageTable rSet,
                              Util::SEARCH_TYPE searchType,
                              uint16_t k,
                              uint16_t kBridge,
                              QString vTableName,
                              QString metricName,
                              QWebSocket *webSocket,
                              int32_t oqId,
                              int32_t userId,
                              QString link,
                              QWidget *parent = 0);
		~OberonViewer();
};

#endif // OBERONVIEWER_H

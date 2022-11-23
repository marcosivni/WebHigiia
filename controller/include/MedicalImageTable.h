#ifndef MEDICALIMAGETABLE_H
#define MEDICALIMAGETABLE_H

//Higiia includes
#include <ResultTable.h>

class MedicalImageTable : public ResultTable {

    public:
        explicit MedicalImageTable(QStringList csvResultSet, bool header = true);
        explicit MedicalImageTable();
        virtual ~MedicalImageTable();

        QString imageFilename(int rowId);
        QString imageID(int rowId);
        QString patientName(int rowId);

        int locateImageFilename();
        int locateImageID();
        int locatePatientName();

        FeatureVector featureVector(int rowId, QString simAttribute);

        void print();
        QString toString();
};

#endif // MEDICALIMAGETABLE_H

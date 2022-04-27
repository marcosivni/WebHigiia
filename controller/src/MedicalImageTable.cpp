#include "MedicalImageTable.h"

MedicalImageTable::MedicalImageTable(QStringList csvResultSet, bool header) :
    ResultTable(csvResultSet, header){
}

MedicalImageTable::MedicalImageTable() : ResultTable(){
}

MedicalImageTable::~MedicalImageTable(){
}

QString MedicalImageTable::imageFilename(int rowId){

    QString answer;
    int pos = locateColumn("Filename");

    if ((pos != -1) && (rowId < size())){
        answer = fetchByColumnId(rowId, pos);
    }

    return answer;
}

QString MedicalImageTable::imageID(int rowId){

    QString answer;
    int pos = locateColumn("Id");

    if ((pos != -1) && (rowId < size())){
        answer = fetchByColumnId(rowId, pos);
    }

    return answer;
}

QString MedicalImageTable::patientName(int rowId){

    QString answer;
    int pos = locateColumn("Patient_Name");

    if ((pos != -1) && (rowId < size())){
        answer = fetchByColumnId(rowId, pos);
    }

    return answer;
}

int MedicalImageTable::locateImageFilename(){

    return locateColumn("Filename");
}

int MedicalImageTable::locateImageID(){

    return locateColumn("Id");
}

int MedicalImageTable::locatePatientName(){

    return locateColumn("Patient_Name");
}

FeatureVector MedicalImageTable::featureVector(int rowId, QString simAttribute){

    FeatureVector answer;
    int pos = locateColumn(simAttribute);

    if ((pos != -1) && (rowId < size())){
        answer.unserializeFromString(FeatureVector::fromBase64(fetchByColumnId(rowId, pos).toStdString()));
    }

    return answer;
}

void MedicalImageTable::print(){

    qDebug() << fetchCaption();

    for (int x = 0; x < size(); x++){
        qDebug() << fetchTupleByRowId(x);
    }
}

QString MedicalImageTable::toString(){

    QString answer;

    for (int x = 0; x < fetchCaption().size(); x++){
        answer.append(fetchCaption(x) + " ");
    }

    for (int x = 0; x < size(); x++){
        answer.append(fetchTupleByRowId(x) + "\n");
    }

    return answer;
}




#include "ResultTable.h"

ResultTable::ResultTable(){
}

ResultTable::ResultTable(QStringList tuples, QStringList caption){

    this->tuples = tuples;
    this->caption = caption;
}

ResultTable::ResultTable(QStringList csvResultSet, bool header){

    if (header && csvResultSet.size()){
        caption = csvResultSet.at(0).split(",");
        csvResultSet.removeAt(0);
    }

    while (csvResultSet.size() && csvResultSet.at(0).size()){
        tuples.append(csvResultSet.at(0));
        csvResultSet.removeAt(0);
    }
}

ResultTable::~ResultTable(){

    clear();
}

void ResultTable::addTuple(QString csvTuple){

    tuples.append(csvTuple);
}

void ResultTable::addCaption(QStringList csvCaption){

    if (caption.size()){
        caption.clear();
    }
    caption = csvCaption;
}

void ResultTable::clear(){

    caption.clear();
    tuples.clear();
}

QStringList ResultTable::fetchCaption(){

    return caption;
}

QString ResultTable::fetchCaption(int pos){

    QString answer;

    if (caption.size() > pos){
        answer = caption.at(pos).simplified();
    }

    return answer;
}

QString ResultTable::fetchTupleByRowId(int rowId){

    return tuples.at(rowId);
}

QString ResultTable::fetchByColumnId(int rowId, int columnId){

    return tuples.at(rowId).split(",").at(columnId).simplified();
}

QString ResultTable::fetchByColumn(int rowId, QString columnName){

    QString answer;
    int idColumn = locateColumn(columnName);

    if (idColumn != -1){
        answer = fetchByColumnId(rowId, idColumn);
    }

    return answer;
}

QStringList ResultTable::fetchColumn(int columnId){

    QStringList answer;

    for (int x = 0; x < size(); x++){
        answer.append(fetchByColumnId(x, columnId));
    }

    return answer;
}

int ResultTable::locateColumn(QString columnName){

    return Util::findColumn(columnName, caption);
}

int ResultTable::locateFirstRow(QString value, int columnId){

    int answer = -1;

    for (int x = 0; x < tuples.size(); x++){
        if (tuples.at(x).split(",").at(columnId).simplified() == value.simplified()){
            answer = x;
            break;
        }
    }

    return answer;
}

QList<int> ResultTable::locateRows(QString value, int columnId){

    QList<int> answer;

    for (int x = 0; x < tuples.size(); x++){
        if (tuples.at(x).split(",").at(columnId).simplified() == value.simplified()){
            answer.append(x);
        }
    }

    return answer;
}

QList<int> ResultTable::locateRows(QString value){

    QList<int> answer;

    for (int x = 0; x < tuples.size(); x++){
        for (int y = 0; y < caption.size(); y++){
            if (tuples.at(x).split(",").at(y).simplified() == value.simplified()){
                answer.append(x);
            }
        }
    }

    return answer;
}


int ResultTable::size(){

    return tuples.size();
}

int ResultTable::dimensionality(){

    return caption.size();
}


bool ResultTable::removeTuple(int rowId){

    bool answer = false;

    answer = (tuples.size() < rowId);
    tuples.removeAt(rowId);

    return answer;
}

bool ResultTable::removeCaption(){

    bool answer = false;

    answer = caption.size();
    caption.clear();

    return answer;
}




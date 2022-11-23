#include "SirenSqlQuery.h"

SirenSQLQuery::SirenSQLQuery(){
}

SirenSQLQuery::~SirenSQLQuery(){

    clear();
}

void SirenSQLQuery::clear(){

    clearProjectionList();
    clearJoinList();
    clearWhereList();
    clearGroupByList();
    clearOrderByList();
}

void SirenSQLQuery::clearProjectionList(){

    pList.clear();
}

void SirenSQLQuery::addProjectionList(QStringList pList){

    this->pList.append(pList);
}

void SirenSQLQuery::addProjectionAttribute(QString att){

    pList.append(att);
}

void SirenSQLQuery::clearWhereList(){

    wList.clear();
}

void SirenSQLQuery::addWhereListAnd(QStringList wList){

    this->wList.append(wList);
}

void SirenSQLQuery::addWhereAttribute(QString att){

    wList.append(att);
}

void SirenSQLQuery::clearGroupByList(){

    gList.clear();
}

void SirenSQLQuery::addGroupByList(QStringList gList){

    this->gList.append(gList);
}

void SirenSQLQuery::addGroupByAttribute(QString att){

    gList.append(att);
}

void SirenSQLQuery::clearOrderByList(){

    oList.clear();
}

void SirenSQLQuery::addOrderByList(QStringList oList){

    this->oList.append(oList);
}

void SirenSQLQuery::addOrderByAttribute(QString att){

    oList.append(att);
}

void SirenSQLQuery::clearJoinList(){

    jList.clear();
    jcList.clear();
}

void SirenSQLQuery::addJoinList(QStringList jList, QStringList onClauseList){

    this->jList.append(jList);
    this->jcList.append(onClauseList);
}

void SirenSQLQuery::addJoin(QString tableName, QString onClause){

    jList.append(tableName);
    jcList.append(onClause);
}

void SirenSQLQuery::addTable(QString tableName){

    this->tableName = tableName;
}

QString SirenSQLQuery::generateQuery(){

    QString query;

    query += "SELECT ";
    for (int x = 0; x < pList.size(); x++){
        if (x > 0){
            query += " , ";
        }
        query += pList.at(x) + " ";
    }
    query += "FROM " + tableName + " ";
    for (int x = 0; x < jList.size(); x++){
        query += " JOIN " + jList.at(x) + " ON " + jcList.at(x) + " ";
    }

    if (wList.size()){
        query += "WHERE ";
        for (int x = 0; x < wList.size(); x++){
            if (x > 0){
                query += " AND ";
            }
            query += wList.at(x) + " ";
        }
    }

    if (gList.size()){
        query += "GROUP BY (";
        for (int x = 0; x < gList.size(); x++){
            if (x > 0){
                query += " , ";
            }
            query += gList.at(x) + " ";
        }
        query += ") ";
    }

    if (oList.size()){
        query += "ORDER BY ";
        if (oList.size() > 1)
            query += "(";
        for (int x = 0; x < oList.size(); x++){
            if (x > 0){
                query += " , ";
            }
            query += oList.at(x) + " ";
        }
        if (oList.size() > 1)
            query += ") ";
    }


    return query;
}


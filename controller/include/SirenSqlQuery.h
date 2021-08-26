#ifndef SIRENSQLQUERY_H
#define SIRENSQLQUERY_H

//Qt includes
#include <QStringList>

class SirenSQLQuery{

    private:
        QStringList pList, wList, gList, oList, jList, jcList;
        QString tableName;

    public:
        SirenSQLQuery();
        ~SirenSQLQuery();

        void clear();

        void clearProjectionList();
        void addProjectionList(QStringList pList);
        void addProjectionAttribute(QString att);

        void clearWhereList();
        void addWhereListAnd(QStringList wList);
        void addWhereAttribute(QString att);

        void clearGroupByList();
        void addGroupByList(QStringList gList);
        void addGroupByAttribute(QString att);

        void clearOrderByList();
        void addOrderByList(QStringList oList);
        void addOrderByAttribute(QString att);

        void clearJoinList();
        void addJoinList(QStringList jList, QStringList onClauseList);
        void addJoin(QString tableName, QString onClause);

        void addTable(QString tableName);

        QString generateQuery();
};

#endif // SIRENSQLQUERY_H

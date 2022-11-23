#ifndef RESULTTABLE_H
#define RESULTTABLE_H

//Qt includes
#include <QStringList>
#include <QDebug>

//Higiia includes
#include <Util.h>

class ResultTable{

    private:
        QStringList tuples;
        QStringList caption;

    public:
        ResultTable();
        ResultTable(QStringList tuples, QStringList caption);
        ResultTable(QStringList csvResultSet, bool header = true);
        ~ResultTable();

        void addTuple(QString csvTuple);
        void addCaption(QStringList csvCaption);

        void clear();

        QStringList fetchCaption();
        QString fetchCaption(int pos);

        QString fetchTupleByRowId(int rowId);
        QString fetchByColumnId(int rowId, int columnId);
        QString fetchByColumn(int rowId, QString columnName);
        QStringList fetchColumn(int columnId);

        int locateColumn(QString columnName);
        int locateFirstRow(QString value, int columnId);
        QList<int> locateRows(QString value, int columnId);
        QList<int> locateRows(QString value);

        int size();
        int dimensionality();

        bool removeTuple(int rowId);
        bool removeCaption();
};

#endif // RESULTTABLE_H

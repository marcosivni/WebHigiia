#ifndef SCOPECAPTION_H
#define SCOPECAPTION_H

//Stl includes
#include <vector>
#include <iostream>

//Qt includes
#include <QString>

class ScopeCaption{

    private:
        std::vector<QString> attNames;
        std::vector<QString> valueNames;
        std::vector<QString> captionNames;

    public:
        ScopeCaption();
        ~ScopeCaption();

        void addEntry(QString const &attName, QString const &valueName, QString const &captionName);

        bool findCaption(QString const &attName, QString const &valueName, std::string *captionValue = nullptr);
        QString matchCaption(QString const &attName, QString const &valueName);

        QString generateSQLBaseQuery(QString const &tableName);

        void print();
};

#endif // SCOPECAPTION_H

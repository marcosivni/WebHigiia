#include "ScopeCaption.h"

ScopeCaption::ScopeCaption(){
}


ScopeCaption::~ScopeCaption(){

    attNames.clear();
    valueNames.clear();
    captionNames.clear();
}

void ScopeCaption::addEntry(const QString &attName, const QString &valueName, const QString &captionName){

    attNames.push_back(attName);
    valueNames.push_back(valueName);
    captionNames.push_back(captionName);
}

bool ScopeCaption::findCaption(const QString &attName, const QString &valueName, std::string *captionValue){

    bool found = false;
    size_t x;

    for (x = 0; ((!found) && (x < attNames.size())); x++){
        found = ((attNames[x].toStdString() == attName.toStdString()) && (valueName.toStdString() == valueNames[x].toStdString()));
    }
    if (found){
        (*captionValue) = captionNames[x-1].toStdString();
    }

    return found;
}

QString ScopeCaption::matchCaption(const QString &attName, const QString &valueName){

    std::string answer;

    findCaption(attName, valueName, &answer);

    return QString(answer.c_str());
}

QString ScopeCaption::generateSQLBaseQuery(const QString &tableName){

    QString answer;

    answer = "SELECT attributeN, valueN, caption FROM Caption WHERE tableN = '" + tableName + "'";

    return answer;
}

void ScopeCaption::print(){

    for (size_t x = 0; x < attNames.size(); x++){
        std::cout << attNames[x].toStdString() << ", " << valueNames[x].toStdString() << ", " << captionNames[x].toStdString() << std::endl;
    }
}

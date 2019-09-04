#ifndef PARSER_H
#define PARSER_H

#include "Signal/SignalValue.h"
#include "./Factory/Factory.h"

#include <QString>
#include <QObject>

class Parser : public QObject
{
    Q_OBJECT

public:
    Parser();
    ~Parser();


    void setConfigFile(QString& filePath);
    int parse(Factory* factory);

signals:
    void parseFinished();

private:
    QString m_configFile;
};

#endif

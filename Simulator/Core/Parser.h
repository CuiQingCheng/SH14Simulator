#ifndef PARSER_H
#define PARSER_H

#include "Signal/SignalValue.h"
#include "./Factory/Factory.h"

#include <QString>

class Parser
{
public:
    Parser();
    ~Parser();

    void setConfigFile(QString& filePath);
    int parse(Factory* factory);

private:
    QString m_configFile;
};

#endif

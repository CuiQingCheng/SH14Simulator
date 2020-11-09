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
    QString getProtocolType();
    int getFixSignalSize(QString cmd);
    QString getDeviceName();

signals:
    void parseFinished();

private:
    QString m_configFile;
    QString m_protocolType;
    QString m_deviceName;
    int m_pollFixedSignalSize;
    int m_recvFixedSignalSize;
};

#endif

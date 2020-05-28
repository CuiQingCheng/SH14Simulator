#include "Parser.h"
#include "../Telegram/Telegram.h"
#include "Factory/Factory.h"
#include "Handler/WidgetHandler.h"

#include <QDebug>
#include <QtXml>
#include <QFile>
#include <QDomDocument>


Parser::Parser()
{

}

Parser::~Parser()
{

}

void Parser::setConfigFile(QString &filePath)
{
    m_configFile = filePath;
}

int Parser::parse(Factory *factory)
{
    QFile file(m_configFile);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("Can't open configuration file : %s", m_configFile.toLatin1().data());
        return -1;
    }

    QDomDocument doc;

    QString error;
    int row = 0, column = 0;

    if(!doc.setContent(&file, false, &error, &row, &column))
    {
        qDebug("Configuration error.");
        qDebug("xml error : %s; row : %d, column : %d.", error.toLatin1().data(), row, column);
        file.close();
        return -1;
    }

    QDomElement root = doc.documentElement();
    QDomElement node = root.firstChildElement();

    QString fieldName;
    QString dataType;
    quint16 byteOffset;
    quint8 bitOffset;
    quint32 defaultValue;
    QString valueType;
    QString element;
    QString elementText;
    QString elementId;
    QStringList tcmsDataLeng;
    bool tcmsIsExisted = false;

    Telegram* telegram = factory->get<Telegram>("Telegram");
    WidgetHandler* widgetHandler = factory->get<WidgetHandler>("WidgetHandler");
    AutoTestHandler* autoTestHandler = factory->get<AutoTestHandler>("AutoTestHandler");

    static bool findCheckBoxlist = false;

    while(!node.isNull())
    {
        if(node.tagName() == "communication" && node.hasAttribute("processType") && node.hasAttribute("sendType"))
        {
            QString sendType = node.attributeNode("sendType").value();
            QDomElement objNode = node.firstChildElement();

            while(!objNode.isNull())
            {
                if(objNode.tagName() == "tcmsData")
                {
                    tcmsIsExisted = true;
                    if(objNode.hasAttribute("portSize"))
                    {
                        tcmsDataLeng.append(objNode.attributeNode("portSize").value());
                    }
                }
                else
                {
                    tcmsIsExisted = false;
                }
                if(objNode.hasAttribute("fieldName"))
                    fieldName = objNode.attributeNode("fieldName").value();
                if(objNode.hasAttribute("type"))
                    dataType = objNode.attributeNode("type").value();
                if(objNode.hasAttribute("byteOffset"))
                    byteOffset = objNode.attributeNode("byteOffset").value().toUInt();
                if(objNode.hasAttribute("bitOffset"))
                    bitOffset = objNode.attributeNode("bitOffset").value().toUInt();
                if(objNode.hasAttribute("defaultValue"))
                    defaultValue = objNode.attributeNode("defaultValue").value().toUInt();
                if(objNode.hasAttribute("valueType"))
                {
                    valueType = objNode.attributeNode("valueType").value();
                }
                else
                {
                    valueType = "Hex";
                }
                SignalValue* signalValue = NULL;
                if(!tcmsIsExisted)
                {
                     signalValue = new SignalValue(fieldName, dataType, byteOffset, bitOffset, defaultValue, valueType);
                }

                if(sendType == "vobc-tod")
                {
                    if(signalValue != NULL)
                    {
                        telegram->setSendSignalMap(signalValue);
                        widgetHandler->addEleNameList(signalValue->getName(), WidgetHandler::Send_Signal);
                    }
                }

                if(sendType == "tod-vobc")
                {
                    if(signalValue != NULL)
                    {
                        telegram->setReceiveSignalMap(signalValue);
                        widgetHandler->addEleNameList(signalValue->getName(), WidgetHandler::Receiver_Signal);
                    }
                }

                objNode = objNode.nextSiblingElement();
            }
        }


        if(node.tagName() == "checkboxList"&& node.hasAttribute("elemtype") && node.hasAttribute("id"))
        {
            findCheckBoxlist = true;
            QString id = node.attributeNode("id").value();
            QDomElement objNode = node.firstChildElement();

            while(!objNode.isNull())
            {
                if(objNode.hasAttribute("name"))
                   elementText = objNode.attributeNode("name").value();
                if(objNode.hasAttribute("id"))
                   elementId = objNode.attributeNode("id").value();


                element = elementText + ":" + elementId;
                if(id == "InfoModule")
                {
                   widgetHandler->addEleNameList(element, WidgetHandler::Info);
                }

                if(id == "FaultModule")
                {
                    widgetHandler->addEleNameList(element, WidgetHandler::Fault);
                }
                objNode = objNode.nextSiblingElement();
            }
        }


        if(node.tagName() == "station")
        {
            AutoTestHandler::Station_Node* const stationNode = new AutoTestHandler::Station_Node;
            stationNode->itsStationid = node.attributeNode("id").value().toInt();

            QDomElement objNode = node.firstChildElement();

            while (!objNode.isNull())
            {
                if(objNode.hasAttribute("StationSkipStatus"))
                    stationNode->itsSkipStatus = objNode.attributeNode("StationSkipStatus").value().toInt();
                if(objNode.hasAttribute("StationHoldStatus"))
                    stationNode->itsHoldStatus = objNode.attributeNode("StationHoldStatus").value().toInt();
                if(objNode.hasAttribute("StationDwell"))
                    stationNode->itsDwell = objNode.attributeNode("StationDwell").value().toInt();
                if(objNode.hasAttribute("PlatformDoorSides"))
                    stationNode->itsPlatformDoorSides = objNode.attributeNode("PlatformDoorSides").value().toInt();
                if(objNode.hasAttribute("DistanceToStoppingPoint"))
                    stationNode->itsDistanceToStopPoint = objNode.attributeNode("DistanceToStoppingPoint").value().toInt();
                if(objNode.hasAttribute("TrainDoorsEnableLeftStatus"))
                    stationNode->itsLeftTrainDoorEnabled = objNode.attributeNode("TrainDoorsEnableLeftStatus").value().toInt();
                if(objNode.hasAttribute("TrainDoorsEnableRightStatus"))
                    stationNode->itsRightTrainDoorEnabled = objNode.attributeNode("TrainDoorsEnableRightStatus").value().toInt();
                if(objNode.hasAttribute("LeftTrainDoorsClosedStatus"))
                    stationNode->itsLeftTrainDoorClosed = objNode.attributeNode("LeftTrainDoorsClosedStatus").value().toInt();
                if(objNode.hasAttribute("RightTrainDoorsClosedStatus"))
                    stationNode->itsRightTrainDoorClosed = objNode.attributeNode("RightTrainDoorsClosedStatus").value().toInt();
                if(objNode.hasAttribute("LeftDoorCloseCommandStatus"))
                    stationNode->itsLeftDoorCloseCommandStatus = objNode.attributeNode("LeftDoorCloseCommandStatus").value().toInt();
                if(objNode.hasAttribute("RightDoorCloseCommandStatus"))
                    stationNode->itsRightDoorCloseCommandStatus = objNode.attributeNode("RightDoorCloseCommandStatus").value().toInt();
                if(objNode.hasAttribute("LeftPSDClosedAndLockedStatus"))
                    stationNode->itsLeftPsdCloseAndLockedStatus = objNode.attributeNode("LeftPSDClosedAndLockedStatus").value().toInt();
                if(objNode.hasAttribute("RightPSDClosedAndLockedStatus"))
                    stationNode->itsRightPsdCloseAndLockedStatus = objNode.attributeNode("RightPSDClosedAndLockedStatus").value().toInt();
                if(objNode.hasAttribute("NextStationId"))
                    stationNode->itsNextStationId = objNode.attributeNode("NextStationId").value().toInt();

                objNode = objNode.nextSiblingElement();
            }
            autoTestHandler->addStationId(QString::number(stationNode->itsStationid), stationNode);
        }

        node = node.nextSiblingElement();
    }

    if(tcmsDataLeng.size()!=0)
    {
        widgetHandler->registerTcmsPort(tcmsDataLeng);
    }

    if(findCheckBoxlist)
    {
        widgetHandler->getWidgetMap()->value("checkBoxWidget")->show();
        widgetHandler->setTrainNumber(WidgetHandler::TrainNumber::Shanghai_14);
        autoTestHandler->setTrainNumber(AutoTestHandler::TrainNumber::Shanghai_14);
        findCheckBoxlist = false;
    }
    else
    {
        widgetHandler->getWidgetMap()->value("checkBoxWidget")->hide();
        widgetHandler->setTrainNumber(WidgetHandler::TrainNumber::Wuhan_11);
        autoTestHandler->setTrainNumber(AutoTestHandler::TrainNumber::Wuhan_11);
    }

    file.close();
    emit parseFinished();

    return 0;
}


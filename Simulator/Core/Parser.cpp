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

    Telegram* telegram = factory->get<Telegram>("Telegram");
    WidgetHandler* widgetHandler = factory->get<WidgetHandler>("WidgetHandler");
//    AutoTestHandler* autoTestHandler = factory->get<AutoTestHandler>("AutoTestHandler");


    while(!node.isNull())
    {
        if(node.tagName() == "communication" && node.hasAttribute("processType") && node.hasAttribute("sendType"))
        {
            QString sendType = node.attributeNode("sendType").value();
            QDomElement objNode = node.firstChildElement();

            while(!objNode.isNull())
            {
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
                    valueType = "hexadecimal";
                }

                SignalValue* signalValue = new SignalValue(fieldName, dataType, byteOffset, bitOffset, defaultValue, valueType);

                if(sendType == "vobc-tod")
                {
                    telegram->setSendSignalMap(signalValue);
                    widgetHandler->addEleNameList(signalValue->getName(), WidgetHandler::Send_Signal);
                }

                if(sendType == "tod-vobc")
                {
                    telegram->setReceiveSignalMap(signalValue);
                    widgetHandler->addEleNameList(signalValue->getName(), WidgetHandler::Receiver_Signal);
                }

                objNode = objNode.nextSiblingElement();
            }
        }


        if(node.tagName() == "checkboxList"&& node.hasAttribute("elemtype") && node.hasAttribute("id"))
        {
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
        node = node.nextSiblingElement();
    }

    file.close();

    return 0;
}


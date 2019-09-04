#include "WidgetHandler.h"


WidgetHandler::WidgetHandler()
{

}

WidgetHandler::~WidgetHandler()
{

}

void WidgetHandler::addEleNameList(QString name, int type)
{
    if(Send_Signal == type)
    {
        if(!m_sendSignalList.contains(name))
        {
            m_sendSignalList.append(name);
        }
    }
    else if(Receiver_Signal == type)
    {
        if(!m_receiveSignalList.contains(name))
        {
            m_receiveSignalList.append(name);
        }
    }
    else if(Info == type)
    {
        if(!m_infoList.contains(name))
        {
            m_infoList.append(name);
        }
    }
    else if(Fault == type)
    {
        if(!m_faultList.contains(name))
        {
            m_faultList.append(name);
        }
    }

}

void WidgetHandler::addWidget(QString &key, QWidget* widget)
{

    if(m_widgetMap.contains(key))
    {
        return ;
    }

    if( widget != NULL )
    {
        m_widgetMap.insert(key, widget);
    }
}

WidgetMap* WidgetHandler::getWidgetMap()
{
    return &m_widgetMap;
}




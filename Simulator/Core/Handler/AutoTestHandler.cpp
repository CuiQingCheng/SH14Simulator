#include "AutoTestHandler.h"
#include <QDebug>

AutoTestHandler::AutoTestHandler()
{

}

AutoTestHandler::~AutoTestHandler()
{

}

void AutoTestHandler::addStationId(QString key, Station_Node const *cfgNode)
{
    if(cfgNode != NULL)
    {
        if(!m_stationNodeMap.contains(key))
        {
            m_stationNodeMap.insert(key, cfgNode);
        }
        else
        {
            qDebug() << "[AutoTestHandler::addStationId] Warning: Failed to add Station_Node " << key << " into m_stationNodeMap";
        }
    }
    else
    {
        qDebug() << "[AutoTestHandler::addStationId] Error: Station_Node " << key << " is NULL";
    }
}

void AutoTestHandler::start()
{

}

void AutoTestHandler::changeCurrentStationNode()
{

}

void AutoTestHandler::actualSpeedControl()
{

}

void AutoTestHandler::dwellControl()
{

}

void AutoTestHandler::stableRunControl()
{

}



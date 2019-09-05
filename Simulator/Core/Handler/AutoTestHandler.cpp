#include "AutoTestHandler.h"


AutoTestHandler::AutoTestHandler()
{
    m_dwellTimer = new QTimer(this);
    m_variableSpeedRunTimer = new QTimer(this);
    m_stableRunTimer = new QTimer(this);
}

AutoTestHandler::~AutoTestHandler()
{

}

void AutoTestHandler::configure(QString key, Station_Node* cfgNode)
{
    if(!m_stationNodeMap.contains(key))
    {
        m_stationNodeMap.insert(key, cfgNode);
    }
}

void AutoTestHandler::start()
{

}

void AutoTestHandler::initializeNodeData()
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



#include "AutoTestHandler.h"
#include <QDebug>

AutoTestHandler::AutoTestHandler()
    : m_currentNode(NULL)
{
    m_dwellTimer = new QTimer(this);
    m_variableSpeedRunTimer = new QTimer(this);
    m_stableRunTimer = new QTimer(this);

    connect(m_dwellTimer, SIGNAL(timeout()), this, SLOT(dwellControl()));
}

AutoTestHandler::~AutoTestHandler()
{

}

void AutoTestHandler::addStationId(QString key, Station_Node *cfgNode)
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
    if(m_stationNodeMap.isEmpty())
    {
        qDebug() << "[AutoTestHandler::start()], stationNodeMap is empty!";
        return;
    }

    if(m_currentNode == NULL)
    {
        m_currentNode = (m_stationNodeMap.begin()).value();
    }
    m_dwell = m_currentNode->itsDwell;
    m_actualSpeed = 0;
    m_targetSpeed = TARGET_SPEED;
    m_permittedSpeed = PERMITTED_SPEED;
    m_ebTriggerSpeed = EB_SPEED;

    m_distanceToStopPoint = 0;

    m_dwellTimer->start(DWELLSEC);
}


void AutoTestHandler::changeCurrentStationNode()
{

}

void AutoTestHandler::actualSpeedControl()
{

}

void AutoTestHandler::dwellControl()
{   if(m_dwell > 0)
    {
       --m_dwell;
    }
    else
    {
        m_dwellTimer->stop();
    }

    emit signalValueUpdated("StationDwell", QString::number(m_dwell));
}

void AutoTestHandler::stableRunControl()
{

}



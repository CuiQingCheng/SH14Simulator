#include "AutoTestHandler.h"
#include <QDebug>

AutoTestHandler::AutoTestHandler()
    : m_currentNode(NULL)
    , m_isaccelerate(true)
{
    m_dwellTimer = new QTimer(this);
    m_variableSpeedRunTimer = new QTimer(this);
    m_stableRunTimer = new QTimer(this);

    connect(m_dwellTimer, SIGNAL(timeout()), this, SLOT(dwellControl()));
    connect(m_variableSpeedRunTimer, SIGNAL(timeout()), this, SLOT(actualSpeedControl()));
    connect(m_stableRunTimer, SIGNAL(timeout()), this, SLOT(stableRunControl()));
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
    m_isaccelerate = true;

    emit signalValueUpdated("ActualSpeed", QString::number(m_actualSpeed));
    emit signalValueUpdated("TargetSpeed", QString::number(m_targetSpeed));
    emit signalValueUpdated("PermittedSpeed", QString::number(m_permittedSpeed));
    emit signalValueUpdated("EbTriggerSpeed", QString::number(m_ebTriggerSpeed));

    emit signalValueUpdated("StationDwell", QString::number(m_dwell));

    emit signalValueUpdated("DistanceToStoppingPoint", QString::number(m_distanceToStopPoint));

}


void AutoTestHandler::changeCurrentStationNode()
{
    quint8 id = m_currentNode->itsStationid;
    QMap<QString, Station_Node*>::iterator iter = m_stationNodeMap.begin();
    static bool isforward = true;

    while(iter != m_stationNodeMap.end())
    {
        if(iter.key() == QString::number(id))
        {
            if(iter+1 != m_stationNodeMap.end())
            {
                if(isforward)
                {
                    m_currentNode = (iter++).value();
                }
                else
                {
                    m_currentNode = (iter--).value();
                }
            }
            break;
        }
        ++iter;
    }

}

void AutoTestHandler::emitChangeNodeSignal()
{
    emit signalValueUpdated("StationSkipStatus", QString::number(m_currentNode->itsSkipStatus));
    emit signalValueUpdated("StationHoldStatus", QString::number(m_currentNode->itsHoldStatus));
    emit signalValueUpdated("NextPlatformDoorSide", QString::number(m_currentNode->itsHoldStatus));
    emit signalValueUpdated("TrainDoorsEnableLeftStatus", QString::number(m_currentNode->itsLeftTrainDoorEnabled));
    emit signalValueUpdated("TrainDoorsEnableRightStatus", QString::number(m_currentNode->itsRightTrainDoorEnabled));
    emit signalValueUpdated("LeftTrainDoorsClosedStatus", QString::number(m_currentNode->itsLeftTrainDoorClosed));
    emit signalValueUpdated("RightTrainDoorsClosedStatus", QString::number(m_currentNode->itsRightTrainDoorClosed));
    emit signalValueUpdated("LeftDoorCloseCommandStatus", QString::number(m_currentNode->itsLeftDoorCloseCommandStatus));
    emit signalValueUpdated("RightDoorCloseCommandStatus", QString::number(m_currentNode->itsRightDoorCloseCommandStatus));

    emit signalValueUpdated("LeftPSDClosedAndLockedStatus", QString::number(m_currentNode->itsLeftPsdCloseAndLockedStatus));
    emit signalValueUpdated("RightPSDClosedAndLockedStatus", QString::number(m_currentNode->itsRightPsdCloseAndLockedStatus));
    emit signalValueUpdated("NextPlatformID", QString::number(m_currentNode->itsStationid));
}

void AutoTestHandler::actualSpeedControl()
{
    static int i = 0;
    if(i == 0)
    {
        emit signalValueUpdated("StationDwell", QString::number(RUNDWELL));
        m_distanceToStopPoint = m_currentNode->itsDistanceToStopPoint;
        emitChangeNodeSignal();
    }
    ++i;

    if(m_actualSpeed > MAXRUN_SPEED)
    {
        m_actualSpeed = MAXRUN_SPEED;
        m_stableRunTimer->start(INTERVAL2);
        m_variableSpeedRunTimer->stop();
        m_isaccelerate = false;
    }

    if(m_actualSpeed < 0)
    {
        m_actualSpeed = 0;
        m_dwellTimer->start(DWELLSEC);
        m_variableSpeedRunTimer->stop();
        m_isaccelerate = true;
        i = 0;
    }

    emit signalValueUpdated("ActualSpeed", QString::number(m_actualSpeed));

    if(m_isaccelerate)
    {
         m_actualSpeed += 3;
         if(m_distanceToStopPoint >= 2000000)
            m_distanceToStopPoint -= i * 10000;
    }
    else
    {
        m_actualSpeed -= 3;
        m_distanceToStopPoint -= (20 - i%20) * 1000;

        if(m_distanceToStopPoint < 0)
        {
            m_distanceToStopPoint = 0;
        }

    }

    emit signalValueUpdated("DistanceToStoppingPoint", QString::number(m_distanceToStopPoint));
}

void AutoTestHandler::dwellControl()
{
    emit signalValueUpdated("StationDwell", QString::number(m_dwell));

    if(m_dwell > 0)
    {
       --m_dwell;
    }
    else
    {
        m_dwellTimer->stop();
        m_variableSpeedRunTimer->start(INTERVAL1);
        changeCurrentStationNode();
    }

}

void AutoTestHandler::stableRunControl()
{
    m_isaccelerate = false;
    m_variableSpeedRunTimer->start(INTERVAL1);
    m_stableRunTimer->stop();

}



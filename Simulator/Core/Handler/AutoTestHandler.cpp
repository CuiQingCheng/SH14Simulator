#include "AutoTestHandler.h"
#include <QDebug>

AutoTestHandler::AutoTestHandler()
    : m_currentNode(NULL)
    , m_isaccelerate(true)
    , m_isForward(true)
    , m_skipStatus(1)
    , m_holdStatus(2)
    , m_platformDoorSides(1)
    , m_leftTrainDoorEnabled(1)
    , m_rightTrainDoorEnabled(1)
    , m_leftTrainDoorClosed(0)
    , m_rightTrainDoorClosed(1)
    , m_leftDoorCloseCommandStatus(0)
    , m_rightDoorCloseCommandStatus(0)
    , m_leftPsdCloseAndLockedStatus(0)
    , m_rightPsdCloseAndLockedStatus(1)
    , m_nextStationId(1)
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
        if(!m_stationNodeMap.contains(key.toInt()))
        {
            m_stationNodeMap.insert(key.toInt(), cfgNode);
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
    m_distanceToStopPoint = 0;
    m_dwellTimer->start(DWELLSEC);
    m_isaccelerate = true;

    emit signalValueUpdated("ActualSpeed", QString::number(m_actualSpeed));

    emit signalValueUpdated("StationDwell", QString::number(m_dwell));

    emit signalValueUpdated("DistanceToStoppingPoint", QString::number(m_distanceToStopPoint));

    int TerminusId;

    if(!m_isForward)
    {
       TerminusId = m_stationNodeMap.begin().key();
    }
    else
    {
        TerminusId = (m_stationNodeMap.end()-1).key();
    }

    emit signalValueUpdated("TerminusPlatformID", QString::number(TerminusId));

    m_skipStatus = m_currentNode->itsSkipStatus;
    m_holdStatus = m_currentNode->itsHoldStatus;
    m_platformDoorSides = m_currentNode->itsPlatformDoorSides;
    m_leftTrainDoorEnabled = m_currentNode->itsLeftTrainDoorEnabled;
    m_rightTrainDoorEnabled = m_currentNode->itsRightTrainDoorEnabled;
    m_leftTrainDoorClosed = m_currentNode->itsLeftTrainDoorClosed;
    m_rightTrainDoorClosed = m_currentNode->itsRightTrainDoorClosed;
    m_leftDoorCloseCommandStatus = m_currentNode->itsLeftDoorCloseCommandStatus;
    m_rightDoorCloseCommandStatus = m_currentNode->itsRightDoorCloseCommandStatus;
    m_leftPsdCloseAndLockedStatus = m_currentNode->itsLeftPsdCloseAndLockedStatus;
    m_rightPsdCloseAndLockedStatus = m_currentNode->itsRightPsdCloseAndLockedStatus;
    m_nextStationId = m_currentNode->itsStationid;

    emitChangeNodeSignal();
}

void AutoTestHandler::stop()
{
    if(m_dwellTimer->isActive())
    {
        m_dwellTimer->stop();
    }

    if(m_variableSpeedRunTimer->isActive())
    {
        m_variableSpeedRunTimer->stop();
    }

    if(m_stableRunTimer->isActive())
    {
        m_stableRunTimer->stop();
    }
}

void AutoTestHandler::clear()
{
    if(m_currentNode != NULL)
    {
        delete m_currentNode;
        m_currentNode = NULL;
    }
    QMap<int, Station_Node*>::iterator iter = m_stationNodeMap.begin();
    while(iter != m_stationNodeMap.end())
    {
        iter = m_stationNodeMap.erase(iter);
    }

}

void AutoTestHandler::initDefSignalValue()
{
    emit signalValueUpdated("TargetSpeed", QString::number(TARGET_SPEED));
    emit signalValueUpdated("PermittedSpeed", QString::number(PERMITTED_SPEED));
    emit signalValueUpdated("EbTriggerSpeed", QString::number(EB_SPEED));
    emit signalValueUpdated("CurrentOperatingMode", QString::number(ATO));
    emit signalValueUpdated("NumberOfCars", QString::number(NUMOFCAR));
    emit signalValueUpdated("CurrentOperatingMode", QString::number(ATO));
    emit signalValueUpdated("NumberOfCars", QString::number(NUMOFCAR));
    emit signalValueUpdated("FAMModeAvailability", "0");
    emit signalValueUpdated("ATPMModeAvaliability", "0");
    emit signalValueUpdated("RunTypeIndication", "2");
    emit signalValueUpdated("NumberOfAvailableVOBCs", "1");
    emit signalValueUpdated("ActiveCab", "1");
}


void AutoTestHandler::changeCurrentStationNode()
{
    quint8 id = m_currentNode->itsStationid;
    QMap<int, Station_Node*>::iterator iter = m_stationNodeMap.begin();

    if(id == (m_stationNodeMap.begin().value())->itsStationid)
    {
        m_isForward = true;

        emit signalValueUpdated("TerminusPlatformID", QString::number((m_stationNodeMap.end()-1).key()));
    }
    else if(id == ((m_stationNodeMap.end()-1).value())->itsStationid)
    {
        m_isForward = false;
        emit signalValueUpdated("TerminusPlatformID", QString::number(m_stationNodeMap.begin().key()));

    }

    while(iter != m_stationNodeMap.end())
    {
        if(iter.key() == id)
        {
            if(m_isForward)
            {
               m_currentNode = (++iter).value();
            }
            else
            {
               m_currentNode = (--iter).value();
            }
            qDebug() << "current id" << m_currentNode->itsStationid;
            break;
        }
        ++iter;
    }

    m_skipStatus = m_currentNode->itsSkipStatus;
    m_holdStatus = m_currentNode->itsHoldStatus;
    m_platformDoorSides = m_currentNode->itsPlatformDoorSides;
    m_nextStationId = m_currentNode->itsStationid;
}

void AutoTestHandler::emitChangeNodeSignal()
{
    if(m_currentTrainNumber == Shanghai_14)
    {
        emit signalValueUpdated("NextPlatformDoorSide", QString::number(m_platformDoorSides));

        emit signalValueUpdated("TrainDoorsEnableLeftStatus", QString::number(m_leftTrainDoorEnabled));
        emit signalValueUpdated("TrainDoorsEnableRightStatus", QString::number(m_rightTrainDoorEnabled));

        emit signalValueUpdated("LeftTrainDoorsClosedStatus", QString::number(m_leftTrainDoorClosed));
        emit signalValueUpdated("RightTrainDoorsClosedStatus", QString::number(m_rightTrainDoorClosed));
        emit signalValueUpdated("LeftDoorCloseCommandStatus", QString::number(m_leftDoorCloseCommandStatus));
        emit signalValueUpdated("RightDoorCloseCommandStatus", QString::number(m_rightDoorCloseCommandStatus));

        emit signalValueUpdated("LeftPSDClosedAndLockedStatus", QString::number(m_leftPsdCloseAndLockedStatus));
        emit signalValueUpdated("RightPSDClosedAndLockedStatus", QString::number(m_rightPsdCloseAndLockedStatus));
    }
    if(m_currentTrainNumber == Wuhan_11)
    {

    }

    emit signalValueUpdated("StationSkipStatus", QString::number(m_skipStatus));
    emit signalValueUpdated("StationHoldStatus", QString::number(m_holdStatus));
    emit signalValueUpdated("NextPlatformID", QString::number(m_nextStationId));
}

void AutoTestHandler::actualSpeedControl()
{
    static int i = 0;

    if(m_dwell == 0)
    {
        i = 0;
        qDebug() << "[AutoTestHandler::actualSpeedControl]: dwell:" << m_dwell;
    }

    if(i == 0)
    {
        emit signalValueUpdated("StationDwell", QString::number(RUNDWELL));
        qDebug() << "[AutoTestHandler::actualSpeedControl]: dwell222:" << m_dwell;
        m_distanceToStopPoint = m_currentNode->itsDistanceToStopPoint;
        emit signalValueUpdated("DepartureStatus", "2");
        emit signalValueUpdated("DockingStatus", "1");
        emit signalValueUpdated("ATOModeAvaliability", "3");
        m_leftTrainDoorClosed = 1;
        m_rightTrainDoorClosed = 1;
        m_leftPsdCloseAndLockedStatus = 1;
        m_rightPsdCloseAndLockedStatus = 1;
        m_nextStationId = m_currentNode->itsStationid;
        m_skipStatus = m_currentNode->itsSkipStatus;
        m_holdStatus = m_currentNode->itsHoldStatus;
        m_platformDoorSides = m_currentNode->itsPlatformDoorSides;
        m_leftTrainDoorEnabled = m_currentNode->itsLeftTrainDoorEnabled;
        m_rightTrainDoorEnabled = m_currentNode->itsRightTrainDoorEnabled;
        m_dwell = m_currentNode->itsDwell;
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
         ++m_actualSpeed;
         if(m_distanceToStopPoint >= 2000000)
            m_distanceToStopPoint -= i * 1000;
    }
    else
    {
        --m_actualSpeed;
        m_distanceToStopPoint -= (60 - i % 60) * 700;

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
    emit signalValueUpdated("DepartureStatus", "1");
    emit signalValueUpdated("DockingStatus", "2");
    emit signalValueUpdated("ATOModeAvaliability", "2");

    if(m_dwell > 0)
    {
        if(m_dwell == m_currentNode->itsDwell)
        {

            if(m_platformDoorSides == 1)
            {
                m_leftTrainDoorClosed = 0;
                m_rightTrainDoorClosed = 1;
                m_leftPsdCloseAndLockedStatus = 0;
                m_rightPsdCloseAndLockedStatus = 1;
            }
            else if(m_platformDoorSides == 2)
            {
                m_leftTrainDoorClosed = 1;
                m_rightTrainDoorClosed = 0;
                m_leftPsdCloseAndLockedStatus = 1;
                m_rightPsdCloseAndLockedStatus = 0;
            }
            else if(m_platformDoorSides == 3)
            {
                m_leftTrainDoorClosed = 0;
                m_rightTrainDoorClosed = 0;
                m_leftPsdCloseAndLockedStatus = 0;
                m_rightPsdCloseAndLockedStatus = 0;
            }

            emit signalValueUpdated("LeftTrainDoorsClosedStatus", QString::number(m_leftTrainDoorClosed));
            emit signalValueUpdated("RightTrainDoorsClosedStatus", QString::number(m_rightTrainDoorClosed));
            emit signalValueUpdated("LeftPSDClosedAndLockedStatus", QString::number(m_leftPsdCloseAndLockedStatus));
            emit signalValueUpdated("RightPSDClosedAndLockedStatus", QString::number(m_rightPsdCloseAndLockedStatus));
        }
       --m_dwell;

        if(m_dwell == 2)
        {
            if(m_platformDoorSides == 1)
            {
               m_leftDoorCloseCommandStatus = 1;
            }
            else if(m_platformDoorSides == 2)
            {
               m_rightDoorCloseCommandStatus = 1;
            }
            else if(m_platformDoorSides == 3)
            {
               m_rightDoorCloseCommandStatus = 1;
               m_leftDoorCloseCommandStatus = 1;
            }

            emit signalValueUpdated("LeftDoorCloseCommandStatus", QString::number(m_leftDoorCloseCommandStatus));
            emit signalValueUpdated("RightDoorCloseCommandStatus", QString::number(m_rightDoorCloseCommandStatus));
            m_rightDoorCloseCommandStatus = 0;
            m_leftDoorCloseCommandStatus = 0;

        }
    }
    else  if(m_dwell == 0)
    {
        m_dwellTimer->stop();
        m_variableSpeedRunTimer->start(INTERVAL1);
        changeCurrentStationNode();
    }

}

void AutoTestHandler::stableRunControl()
{

    m_isaccelerate = false;
    if(m_skipStatus == 1)
    {
        m_variableSpeedRunTimer->start(INTERVAL1);
        m_stableRunTimer->stop();
        m_distanceToStopPoint = 1200000;

        emit signalValueUpdated("DistanceToStoppingPoint", QString::number(m_distanceToStopPoint));
    }
    else if(m_skipStatus == 2)
    {
        changeCurrentStationNode();
        emit signalValueUpdated("StationSkipStatus", QString::number(m_skipStatus));
        emit signalValueUpdated("StationHoldStatus", QString::number(m_holdStatus));
        emit signalValueUpdated("NextPlatformDoorSide", QString::number(m_platformDoorSides));
        emit signalValueUpdated("NextPlatformID", QString::number(m_nextStationId));
    }

}



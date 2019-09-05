#ifndef AUTOTESTHANDLER_H
#define AUTOTESTHANDLER_H

#include <QObject>
#include <QMap>
#include <QTimer>

class AutoTestHandler:public QObject
{
    Q_OBJECT

public:
    AutoTestHandler();
    ~AutoTestHandler();

    struct Station_Node{
        Station_Node()
        {
            itsStationid = 0;
            itsSkipStatus = 0;
            itsHoldStatus = 0;
            itsPlatformDoorSides = 0;
            itsLeftTrainDoorEnabled = 0;
            itsRightTrainDoorEnabled = 0;
            itsLeftTrainDoorClosed = 0;
            itsRightTrainDoorClosed = 0;
            itsLeftDoorCloseCommandStatus = 0;
            itsRightDoorCloseCommandStatus = 0;
            itsLeftPsdCloseAndLockedStatus = 0;
            itsRightPsdCloseAndLockedStatus = 0;
            itsNextStationId = 0;
            itsDwell = 0;
            itsDistanceToStopPoint = 0;
        }
        Station_Node &operator=(const Station_Node &rhs)
        {
            if(this != &rhs)
            {
                itsStationid = rhs.itsStationid;
                itsSkipStatus = rhs.itsSkipStatus;
                itsHoldStatus = rhs.itsHoldStatus;
                itsPlatformDoorSides = rhs.itsPlatformDoorSides;
                itsLeftTrainDoorEnabled = rhs.itsLeftTrainDoorEnabled;
                itsRightTrainDoorEnabled = rhs.itsRightTrainDoorEnabled;
                itsLeftTrainDoorClosed = rhs.itsLeftTrainDoorClosed;
                itsRightTrainDoorClosed = rhs.itsRightTrainDoorClosed;
                itsLeftDoorCloseCommandStatus = rhs.itsLeftDoorCloseCommandStatus;
                itsRightDoorCloseCommandStatus = rhs.itsRightDoorCloseCommandStatus;
                itsLeftPsdCloseAndLockedStatus = rhs.itsLeftPsdCloseAndLockedStatus;
                itsRightPsdCloseAndLockedStatus = rhs.itsRightPsdCloseAndLockedStatus;
                itsNextStationId = rhs.itsNextStationId;
                itsDwell = rhs.itsDwell;
                itsDistanceToStopPoint = rhs.itsDistanceToStopPoint;
            }

            return *this;
        }
        quint8 itsStationid;
        quint8 itsSkipStatus;
        quint8 itsHoldStatus;
        quint8 itsPlatformDoorSides;

        quint8 itsLeftTrainDoorEnabled;
        quint8 itsRightTrainDoorEnabled;

        quint8 itsLeftTrainDoorClosed;
        quint8 itsRightTrainDoorClosed;

        quint8 itsLeftDoorCloseCommandStatus;
        quint8 itsRightDoorCloseCommandStatus;

        quint8 itsLeftPsdCloseAndLockedStatus;
        quint8 itsRightPsdCloseAndLockedStatus;

        quint8 itsNextStationId;

        quint32 itsDwell;
        quint32 itsDistanceToStopPoint;
    };


    void addStationId( QString key , Station_Node const * cfgNode);

    void start(); //start Auto Test

    void initializeNodeData();

    void changeCurrentStationNode();

 signals:
    void signalValueUpdated(QString signal, QString value);

public slots:
    void actualSpeedControl();
    void dwellControl();
    void stableRunControl();

private:
    QMap<QString, Station_Node const*> m_stationNodeMap;
    const quint8 MAX_SPEED = 65;

    quint8 m_dwell;
    quint8 m_actualSpeed; //<! km/h
    quint8 m_targetSpeed;
    quint8 m_permittedSpeed;
    quint8 m_ebTriggerSpeed;

    quint32 m_distanceToStopPoint;
    quint8 m_runStatus; // 0: accelerate,  1:decelerate 2: constant speed

    QTimer* m_dwellTimer;
    QTimer* m_variableSpeedRunTimer;
    QTimer* m_stableRunTimer;

};

#endif

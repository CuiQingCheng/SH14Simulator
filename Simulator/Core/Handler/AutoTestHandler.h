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


    void addStationId( QString key , Station_Node * cfgNode);

    void start(); //start Auto Test
    void stop(); //stop Auto Test

    void changeCurrentStationNode();

    void emitChangeNodeSignal();

 signals:
    void signalValueUpdated(QString signal, QString value);

public slots:
    void actualSpeedControl();
    void dwellControl();
    void stableRunControl();

private:
    QMap<QString, Station_Node*> m_stationNodeMap;
    const quint8 MAXRUN_SPEED = 60;
    const quint8 TARGET_SPEED = 65;
    const quint8 PERMITTED_SPEED = 75;
    const quint8 EB_SPEED = 85;
    const quint32 DWELLSEC = 1000;
    const quint32 INTERVAL1 = 500;
    const quint32 INTERVAL2 = 20000;
    const quint32 RUNDWELL = 65536;


    Station_Node* m_currentNode;
    quint8 m_dwell;
    qint16 m_actualSpeed; //<! km/h
    quint8 m_targetSpeed;
    quint8 m_permittedSpeed;
    quint8 m_ebTriggerSpeed;
    bool m_isaccelerate;

    qint64 m_distanceToStopPoint;
    quint8 m_runStatus; // 0: accelerate,  1:decelerate 2: constant speed 3: Stop

    QTimer* m_dwellTimer;
    QTimer* m_variableSpeedRunTimer;
    QTimer* m_stableRunTimer;
    bool m_isForward;
};

#endif

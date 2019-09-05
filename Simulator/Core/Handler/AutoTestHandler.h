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
        QString itsstationid;
        quint8 itsSkipStatus;
        quint8 itsHoldStatus;
        quint32 itsDwell;
        quint32 itsDistanceToStopPoint;

        quint8 itsPlatformDoorSides;
        quint8 itsLeftTrainDoorEnabled;
        quint8 itsRightTrainDoorEnabled;
        quint8 itsLeftTrainDoorClosed;
        quint8 itsRightTrainDoorClosed;
        quint8 itsLeftDoorCloseCommandStatus;
        quint8 itsRightDoorCloseCommandStatus;

        quint8 itsLeftPsdCloseAndLockedStatus;
        quint8 itsRightPsdCloseAndLockedStatus;
    };

    void configure( QString key, Station_Node* cfgNode);

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
    QMap<QString, Station_Node*> m_stationNodeMap;
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

#ifndef TODVOBCCHANNEL_H
#define TODVOBCCHANNEL_H

#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <QTimer>

#include <QSerialPort>

class QUdpSocket;
class QByteArray;
class QDateTime;
class TodVobcTelegram;
class TodTcmsTelegram;

class TodCommChannel : public QObject
{
    Q_OBJECT
public:

    enum SERIAL_MEG_TYPE{
            INIT_MESSAGE = 0,
            PERIODIC_MESSAGE
    };

    TodCommChannel(QObject *parent = 0);

    bool openUdpCom();
    bool openSerialPort(int baudRate);

    void closeUdpCom();
    void closeSerialPort();

    void clear();

    void setDefPollDataSize(int size);
    void setDefRecvDataSize(int size);

    void setjudgeInterval(QString cmd);

    QString getLocalIp();

    void setTodAdress(const QString &todAddress);

    void setVobcAdress(const QString &vobcAddress);

    void setTodPort(quint16 todPort);

    void setVobcPort(quint16 vobcPort);

    void setTodId(quint16 id);

    void sendDataTelegram(QString cmd);

    void sendDataTelegramForRFC(QString cmd);

    void setDataByByteSize(const QString& byteOffset,const QString& bitOffset,const QString& sizetype,const QString& numType, const QString& value);

    void setOptionTag(int byteoffset,int tag);

    void setDataByInfoFault(const QList<int> &lst, int offset, int type);

    void setDataByTCMS(const QStringList &slst, int offset);

    QDateTime getCurDateTime();

    QByteArray receiveData();

    bool getComConnectState();

    void setComConnectState(bool setStateFlag);

    void setTSN(quint16 TSN);
    void setRSN(quint16 RSN);
    quint16 getTSN();
    quint16 getRSN();

    void setSerialPortName(QString name);
    bool isSerialPortOpen();

    int checkTcmsTelegram(QByteArray& baTelegram, bool& bValid, int& iStart, int& iEnd, qint8& crc);

signals:
    void updateRecvWidgetData();

private:
    bool checkTelegram(const TodVobcTelegram &telegram);

public slots:
    void receiveTelegrams();
    void setTimerStop();
    void recvSerialData();

private:
    QUdpSocket *m_socket;
    QSerialPort* m_serialDriver;
    QHostAddress m_todAddress;
    QHostAddress m_vobcAddress;
    quint16 m_todPort;
    quint16 m_vobcPort;
    quint16 m_todId;
    quint16 m_RSN;
    quint16 m_TSN;


    QByteArray m_receiveAppData;
    QByteArray m_serialTempQueue;
    QByteArray m_serialInputQueue;
    static const int MAX_SIZE = 500;
    unsigned char m_pucData[MAX_SIZE];

    int m_actualDataSize;
    int m_recvFixDataSize;
    bool m_comConnectStateFlag;

    int m_gap;
    QTimer *judgeCommStateTimer;

};

#endif // TODVOBCCHANNEL_H

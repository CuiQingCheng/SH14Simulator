#ifndef TODVOBCCHANNEL_H
#define TODVOBCCHANNEL_H

#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <QTimer>

class QUdpSocket;
class QByteArray;
class QDateTime;
class TodVobcTelegram;

class TodVobcChannel : public QObject
{
    Q_OBJECT
public:
    TodVobcChannel(QObject *parent = 0);

    bool open();

    void close();

    void clear();

    QString getLocalIp();

    void setTodAdress(const QString &todAddress);

    void setVobcAdress(const QString &vobcAddress);

    void setTodPort(quint16 todPort);

    void setVobcPort(quint16 vobcPort);

    void setTodId(quint16 id);

    void sendDataTelegram();

    void sendDataTelegramForRFC();

    void setDataByByteSize(const QString& byteOffset,const QString& bitOffset,const QString& sizetype,const QString& numType, const QString& value);

    void setOptionTag(int tag);

    void setDataByInfoFault(const QList<int> &lst, int offset, int type);

    void setDataByTCMS(const QStringList &slst, int offset);

    QDateTime getCurDateTime();

    QByteArray receiveData();

    bool getComConnectState();

    void setComConnectState(bool setStateFlag);

    void endTimer();

    void setTSN(quint16 TSN);
    void setRSN(quint16 RSN);
    quint16 getTSN();
    quint16 getRSN();

private:
    bool checkTelegram(const TodVobcTelegram &telegram);

public slots:
    void receiveTelegrams();
    void setTimerStop();

private:
    QUdpSocket *m_socket;
    QHostAddress m_todAddress;
    QHostAddress m_vobcAddress;
    quint16 m_todPort;
    quint16 m_vobcPort;
    quint16 m_todId;
    quint16 m_RSN;
    quint16 m_TSN;

    QByteArray m_receiveAppData;

    static const int MAX_SIZE = 260;
    static const int FIX_SIZE = 79;
    unsigned char m_pucData[MAX_SIZE];

    int m_actualDataSize;
    bool m_comConnectStateFlag;

    int m_gap;
    QTimer *judgeCommStateTimer;
};

#endif // TODVOBCCHANNEL_H

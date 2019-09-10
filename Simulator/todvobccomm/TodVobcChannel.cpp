#include "TodVobcChannel.h"
#include "Todvobctelegram.h"

#include <QDebug>
#include <QUdpSocket>
#include <QDateTime>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QProcess>


TodVobcChannel::TodVobcChannel(QObject *parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
    , m_todPort(0)
    , m_todId(0)
    , m_vobcPort(0)
    , m_RSN(0)
    , m_TSN(0)
    , m_comConnectStateFlag(false)
    , m_gap(6)
    , m_actualDataSize(113)
{
    clear();

    judgeCommStateTimer = new QTimer(this);
    connect(judgeCommStateTimer, SIGNAL(timeout()), this, SLOT(setTimerStop()));

    judgeCommStateTimer->setInterval(3000);
    judgeCommStateTimer->setSingleShot(true);
}

void TodVobcChannel::clear()
{
    memset(m_pucData, 0, sizeof(m_pucData));
    m_actualDataSize = 113;
}

bool TodVobcChannel::open()
{
    bool ret = false;
    if(m_socket->isOpen())
    {
        m_socket->close();
    }
    m_vobcAddress = getLocalIp();
    if (m_vobcAddress.isNull()) {
        qDebug() << "TodVobcChannel::open, m_vobcAddress is NULL";
        return ret;
    }
    if (m_socket->bind(m_vobcAddress, m_vobcPort)) {
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(receiveTelegrams()));
        ret = true;
    }

    qDebug() << "TodVobcChannel::open, ret" << ret;
    return ret;
}

void TodVobcChannel::close()
{
    m_RSN = 0;
    m_socket->disconnect();
    m_socket->close();
}

void TodVobcChannel::receiveTelegrams()
{
    while (m_socket->hasPendingDatagrams()) {                                                        // 拥有等待的数据报, always waiting read the Databuffer until the reading is finished.
        QByteArray bg;
        bg.resize(m_socket->pendingDatagramSize());                                                  // 让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
        m_socket->readDatagram(bg.data(), bg.size(), &m_todAddress, &m_todPort);                     // 接收数据报，将其存放到datagram中
        TodVobcTelegram tg(bg);


        // m_TSN = tg.TSN();
        m_RSN = tg.TSN();

//        qDebug() << "[TodVobcChannel::receiveTelegrams] RSN = " << tg.RSN() << " m_TSN = "<< m_RSN;

        // start to wait a Interval time to set connect state false;
        judgeCommStateTimer->start();

//        qDebug() << "receive telegram Length:" << bg.length();
//        for (int i = 0; i < tg.size(); i++) {
//             printf("0x%02x ", (unsigned char)(tg.at(i)));
//        }
//        printf("\n");

        if (checkTelegram(tg))
        {
            switch (tg.telegramType()){
            case TodVobcTelegram::Response:
            default:
                m_comConnectStateFlag = true;
                m_receiveAppData = QByteArray(tg.appData(), tg.appDataLength());
                break;
            }
        }
    }
}

void TodVobcChannel::setTimerStop()
{
    qDebug() << "TodVobcChannel::setTimerStop()";
    if(judgeCommStateTimer->isActive())
    {
        judgeCommStateTimer->stop();
    }
    m_comConnectStateFlag = false;
}

void TodVobcChannel::endTimer()
{
    if(judgeCommStateTimer->isActive())
    {
        judgeCommStateTimer->stop();
    }
    m_comConnectStateFlag = false;
}

QByteArray TodVobcChannel::receiveData()
{
    if(m_comConnectStateFlag)
        return m_receiveAppData;
    else
        return NULL;
}

bool TodVobcChannel::getComConnectState()
{
    return m_comConnectStateFlag;
}

void TodVobcChannel::setComConnectState(bool setStateFlag)
{
    m_comConnectStateFlag = setStateFlag;
}

void TodVobcChannel::setDataByByteSize(const QString& byteOffset,const QString& bitOffset,const QString& sizetype,const QString& numType, const QString& value)
{
    bool turnOk = false;
    quint16 byteOffNum = byteOffset.toInt(&turnOk, 10);
    quint8 bitOffNum = bitOffset.toInt(&turnOk, 10);

    //if no value input from the emulator, set the default value with zero.
    if(value.isNull())
    {
        QString zero = "0";
        value.arg(zero);
    }

    //make it different between hex and dec.
    quint64 valueNum;  // quint32

    if(numType == "hexadecimal")
    {
        valueNum = value.toLongLong(&turnOk, 16);
    }
    else if(numType == "Dec")
    {
        valueNum = value.toLongLong(&turnOk, 10);
    }

    if("BOOLEAN.1" == sizetype)
    {
        m_pucData[byteOffNum] |= (unsigned char)((valueNum & 0x01) << bitOffNum) ;
    }
    else if("UNSIGNED.8" == sizetype)
    {
        m_pucData[byteOffNum] = (unsigned char)valueNum;
    }
    else if("UNSIGNED.16" == sizetype)
    {
        m_pucData[byteOffNum] = (unsigned char)(valueNum & 0xFF);
        m_pucData[byteOffNum + 1] = (unsigned char)((valueNum >> 8) & 0xFF);
    }
    else if("UNSIGNED.24" == sizetype)
    {
        m_pucData[byteOffNum] = (unsigned char)(valueNum & 0xFF);
        m_pucData[byteOffNum + 1] = (unsigned char)((valueNum >> 8) & 0xFF);
        m_pucData[byteOffNum + 2] = (unsigned char)((valueNum >> 8*2) & 0xFF);
    }
    else if("UNSIGNED.32" == sizetype)
    {
        m_pucData[byteOffNum] = (unsigned char)(valueNum & 0xFF);
        m_pucData[byteOffNum + 1] = (unsigned char)((valueNum >> 8) & 0xFF);
        m_pucData[byteOffNum + 2] = (unsigned char)((valueNum >> 8*2) & 0xFF);
        m_pucData[byteOffNum + 3] = (unsigned char)((valueNum >> 8*3) & 0xFF);
    }
    else if("UNSIGNED.56" == sizetype)
    {
        m_pucData[byteOffNum] = (unsigned char)(valueNum & 0xFF);
        m_pucData[byteOffNum + 1] = (unsigned char)((valueNum >> 8) & 0xFF);
        m_pucData[byteOffNum + 2] = (unsigned char)((valueNum >> 8*2) & 0xFF);
        m_pucData[byteOffNum + 3] = (unsigned char)((valueNum >> 8*3) & 0xFF);
        m_pucData[byteOffNum + 4] = (unsigned char)((valueNum >> 8*4) & 0xFF);
        m_pucData[byteOffNum + 5] = (unsigned char)((valueNum >> 8*5) & 0xFF);
        m_pucData[byteOffNum + 6] = (unsigned char)((valueNum >> 8*6) & 0xFF);
        for(int i = byteOffNum; i < byteOffNum+7; ++i)
        {

            qDebug() << " itsValue[" << i << "] is " << m_pucData[i] << " ";
        }
    }
    else if(sizetype.contains("NULL", Qt::CaseInsensitive))
    {
        // handle "80-112"
        QString byteoffSetStart = value.section('-', 0, 0);
        QString byteoffSetEnd = value.section('-', 1, 1);
        quint8 byteoffSetStartNum = byteoffSetStart.toInt();
        quint8 byteoffSetEndNum = byteoffSetEnd.toInt();

        for(int i=byteoffSetStartNum; i<byteoffSetEndNum; i++)
        {
            m_pucData[i] = 0;
        }
    }

    //qDebug("setDataByByteSize: byteOffNum-%d,bitOffNum-%d,m_pucData[70]-0x%2x",byteOffNum,bitOffNum,m_pucData[70]);
}

void TodVobcChannel::setOptionTag(int tag)
{
    m_pucData[FIX_SIZE - 1] = (unsigned char) tag;
}



void TodVobcChannel::setDataByInfoFault(const QList<int> &lst, int offset, int type)
{
    int count = lst.size();
    int byteOffSet = FIX_SIZE + offset;
    m_pucData[byteOffSet] = (unsigned char) (count + 1);
//    qDebug() << "type:" << type;
    m_pucData[++byteOffSet] = (unsigned char) type;
    for(int i = 1; i <= count; ++i)
    {
        m_pucData[byteOffSet + i] = (unsigned char) lst[i-1];
    }
}

void TodVobcChannel::setDataByTCMS(const QStringList &slst, int offset)
{
    int byteOffSet = FIX_SIZE + offset;
    int count = slst.size();
    for(int i = 0; i < count; ++i)
    {
        bool ok;
        m_pucData[byteOffSet + i] = (unsigned char)((slst[i]).toLongLong(&ok, 16));
    }
    m_actualDataSize = byteOffSet + count;
}



void TodVobcChannel::sendDataTelegram()
{
    QByteArray bg = QByteArray::fromRawData((const char*)m_pucData, m_actualDataSize);   // 由于需要在qUdpsocket发送数据需要QByteArray类型，所以需要对unsigned char[]与QByteArray进行转化；
    TodVobcTelegram telegram = TodVobcTelegram::createTelegram(bg);
    telegram.setInterfaceType(1004);
    telegram.setInterfaceVersion(0);

    telegram.setTelegramType(TodVobcTelegram::Poll);

    telegram.setReceiverClass(6);
    telegram.setReceiverId(m_todId);
    telegram.setTransmitterClass(1);
    telegram.setTransmitterId(m_todId);
    telegram.setDateTime(getCurDateTime());

    telegram.setRSN(m_RSN);

    if(++m_TSN == 0)
        ++m_TSN;
    telegram.setTSN(m_TSN);
    telegram.setAppDataLength(m_actualDataSize);
    telegram.setAppData(bg);
    telegram.updateCRC();


//    for (int i = 0; i < 34 + m_actualDataSize; i++)
//    {
//        qDebug("-- m_pucData[%d], %d", i, (int)(telegram.data()[i]));
//    }

    qDebug() << "send telegram size:" << telegram.size();
    m_socket->writeDatagram(telegram, m_todAddress, m_todPort);
}

void TodVobcChannel::sendDataTelegramForRFC()
{
    QByteArray bg = QByteArray::fromRawData((const char*)m_pucData, m_actualDataSize);
    TodVobcTelegram telegram = TodVobcTelegram::createTelegram(bg);
    telegram.setInterfaceType(1004);
    telegram.setInterfaceVersion(0);
    telegram.setTelegramType(TodVobcTelegram::RFC);
    telegram.setReceiverClass(6);
    telegram.setReceiverId(m_todId);
    telegram.setTransmitterClass(1);
    telegram.setTransmitterId(m_todId);
    telegram.setDateTime(getCurDateTime());

    if(++m_TSN == 0)
        ++m_TSN;

    telegram.setRSN(0);
    telegram.setTSN(m_TSN);
    telegram.setAppDataLength(m_actualDataSize);
    telegram.setAppData(bg);
    telegram.updateCRC();

//    for (int i = 0; i < 34 + m_actualDataSize; i++)
//    {
//        qDebug("-- m_pucData[%d], %d", i, (int)(telegram.data()[i]));
//    }

    qDebug() << "send RFC telegram size:" << telegram.size();
    m_socket->writeDatagram(telegram, m_todAddress, m_todPort);
}

void TodVobcChannel::setTSN(quint16 TSN)
{
    m_TSN = TSN;
}

void TodVobcChannel::setRSN(quint16 RSN)
{
    m_RSN = RSN;
}

quint16 TodVobcChannel::getTSN()
{
    return m_TSN;
}

quint16 TodVobcChannel::getRSN()
{
    return m_RSN;
}

void TodVobcChannel::setTodAdress(const QString &todAddress)
{
//    qDebug() << "TodVobcChannel::setTodAdress, todAddress" << todAddress;
    m_todAddress = todAddress;
}

void TodVobcChannel::setVobcAdress(const QString &vobcAddress)
{
//    qDebug() << "TodVobcChannel::setVobcAdress, todAddress" << vobcAddress;
    m_vobcAddress = vobcAddress;
}

void TodVobcChannel::setTodPort(quint16 todPort)
{
//    qDebug() << "TodVobcChannel::setTodPort, todPort" << todPort;
    m_todPort = todPort;
}

void TodVobcChannel::setVobcPort(quint16 vobcPort)
{
//    qDebug() << "TodVobcChannel::setVobcPort, vobcPort" << vobcPort;
    m_vobcPort = vobcPort;
}

void TodVobcChannel::setTodId(quint16 id)
{
//    qDebug() << "TodVobcChannel::setTodId, id" << id;
    m_todId = id;
}

QDateTime TodVobcChannel::getCurDateTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    return dateTime;
}

QString TodVobcChannel::getLocalIp()
{
    QString strLocalAddress;
    QList<QHostAddress> localAddress = QNetworkInterface::allAddresses();
    int iSize = localAddress.size();
    for (int i = 0; i < iSize; ++i) {
//        qDebug() << "TodVobcChannel::getLocalIp()" << localAddress.at(i) << localAddress.at(i).toString() << localAddress.at(i).toIPv4Address() << QHostAddress::LocalHost;
        if ((localAddress.at(i) != QHostAddress::LocalHost)
                && localAddress.at(i).toIPv4Address()) {
            strLocalAddress = localAddress.at(i).toString();
            break;
        }
    }
//    qDebug() << "TodVobcChannel::getLocalIp()" << strLocalAddress;
    return strLocalAddress;
}

bool TodVobcChannel::checkTelegram(const TodVobcTelegram &telegram)
{
    //if (!telegram.isValid()) // for test
    //    return false;

    if (telegram.interfaceType() != 1004) {
        qWarning("TodVobcChannel: the interface type invalid");
        return false;
    }

    if (telegram.receiverClass() != 1) {
        qWarning("TodVobcChannel: the receiver class invalid");
        return false;
    }

    if (telegram.receiverID() != m_todId) {
        qWarning("TodVobcChannel: the receiver id invalid");
        qDebug() << "Receiver id : " << telegram.receiverID();
        qDebug() << "Tod id : " << m_todId;
        return false;
    }

    if (telegram.transmitterClass() != 6) {
        qWarning("TodVobcChannel: the transmitter class invalid");
        return false;
    }

    return true;
}

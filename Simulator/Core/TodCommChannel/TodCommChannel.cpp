#include "TodCommChannel.h"
#include "../../todvobccomm/Todvobctelegram.h"
#include "../../todtcmsccomm/Todtcmstelegram.h"
#include "../../CRC/mycrc.h"

#include <QDebug>
#include <QUdpSocket>
#include <QDateTime>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QProcess>

#define DLE 0x10
#define STX 0x02
#define ETX 0x03


TodCommChannel::TodCommChannel(QObject *parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
    , m_todPort(0)
    , m_todId(0)
    , m_vobcPort(0)
    , m_RSN(0)
    , m_TSN(0)
    , m_comConnectStateFlag(false)
    , m_gap(6)
    , m_actualDataSize(0)
{
    clear();
    m_serialDriver = new QSerialPort(this);
    judgeCommStateTimer = new QTimer(this);
    connect(judgeCommStateTimer, SIGNAL(timeout()), this, SLOT(setTimerStop()));

    judgeCommStateTimer->setInterval(3000);
    judgeCommStateTimer->setSingleShot(true);
    connect(m_serialDriver, SIGNAL(readyRead()), this, SLOT(recvSerialData()) );
}



void TodCommChannel::clear()
{
    memset(m_pucData, 0, sizeof(m_pucData));
}

void TodCommChannel::setDefPollDataSize(int size)
{
    m_actualDataSize = size;
}

void TodCommChannel::setDefRecvDataSize(int size)
{
    m_recvFixDataSize = size;
}

void TodCommChannel::setjudgeInterval(QString cmd)
{
    if(cmd == "serial")
    {
        judgeCommStateTimer->setInterval(60000);
    }
    else
    {
        judgeCommStateTimer->setInterval(3000);
    }
}

bool TodCommChannel::openUdpCom()
{
    bool ret = false;
    if(m_socket->isOpen())
    {
        m_socket->close();
    }
    m_vobcAddress = getLocalIp();
    if (m_vobcAddress.isNull()) {
        qDebug() << "TodCommChannel::open, m_vobcAddress is NULL";
        return ret;
    }
    if (m_socket->bind(m_vobcAddress, m_vobcPort)) {
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(receiveTelegrams()));
        ret = true;
    }

    qDebug() << "TodCommChannel::open, ret" << ret;
    return ret;
}

void TodCommChannel::closeUdpCom()
{
    m_RSN = 0;
    m_socket->disconnect();
    m_socket->close();
}

void TodCommChannel::receiveTelegrams()
{
    while (m_socket->hasPendingDatagrams()) {                                                        // 拥有等待的数据报, always waiting read the Databuffer until the reading is finished.
        QByteArray bg;
        bg.resize(m_socket->pendingDatagramSize());                                                  // 让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
        m_socket->readDatagram(bg.data(), bg.size(), &m_todAddress, &m_todPort);                     // 接收数据报，将其存放到datagram中
        TodVobcTelegram tg(bg);


        // m_TSN = tg.TSN();
        m_RSN = tg.TSN();

//        qDebug() << "[TodCommChannel::receiveTelegrams] RSN = " << tg.RSN() << " m_TSN = "<< m_RSN;

        // start to wait a Interval time to set connect state false;
        judgeCommStateTimer->start();

        qDebug() << "TCMS telegram ";
        for (int i = 42; i < tg.size(); i++) {
             printf("0x%02x ", (unsigned char)(tg.at(i)));
        }
        printf("\n\n");

        if (checkTelegram(tg))
        {
            switch (tg.telegramType()){
            case TodVobcTelegram::Response:
            default:
                m_comConnectStateFlag = true;
                m_receiveAppData = QByteArray(tg.appData(), tg.appDataLength());
//                qDebug()<<"appdata size: "<< m_receiveAppData.size();
                break;
            }
        }
    }
}

void TodCommChannel::setTimerStop()
{
    qDebug() << "TodCommChannel::setTimerStop()";
    if(judgeCommStateTimer->isActive())
    {
        judgeCommStateTimer->stop();
    }
    m_comConnectStateFlag = false;
}

void TodCommChannel::recvSerialData()
{
//    qDebug() << "TodCommChannel::recvSerialData()";
    QByteArray data = m_serialDriver->readAll();
    m_serialDriver->flush();
    int recvSize = data.size();

//    for(int i = 0; i < recvSize; ++i)
//    {
//        qDebug("-- recvSize[%d], %x", i, (int)(data.data()[i]));
//    }
//    qDebug() << "recv data size : " << recvSize;

    if(recvSize <= 0)
    {
        return;
    }

    judgeCommStateTimer->start();

    m_serialTempQueue.append(data);

    int mark = 1;

    for(;mark > 0;)
    {
        bool bValid = false;
        int iStart = 0;
        int iEnd = 0;

        qint8 inputCrc;

        mark = checkTcmsTelegram(m_serialTempQueue,bValid,iStart,iEnd, inputCrc);
        int telesize = m_serialInputQueue.size();
        if(bValid)
        {
            // crash protection
            if(m_serialTempQueue.size() >= (iEnd + 2))
            {
                m_serialTempQueue.remove(0, telesize + 1);
            }

            if(m_serialTempQueue.size() < 3 )
            {
                mark = -1;
            }
        }
        else
        {
            continue;
        }

//        for(int i = 0; i < telesize; ++i)
//        {
//            qDebug("-- telegram[%d], %x", i, (int)(m_serialInputQueue.data()[i]));
//        }
//        qDebug() << "whole serial telegram size : " << telesize;

        TodTcmsTelegram todTcmsTelegram = TodTcmsTelegram::decoderPacket(m_serialInputQueue);
        quint8 iMessageType = 2;
        quint8 chr0 = todTcmsTelegram.appDataCommType();
        quint8 chr1 = todTcmsTelegram.appDataDeviceAddress();

        if((chr1 == 0x07) && (chr0 == 0x07)) // Initialisation message from TCMS.
        {
            iMessageType = INIT_MESSAGE;
        }
        else if((chr1 == 0x07) && (chr0 == 0x0F)) // Periodic status message from TCMS.
        {
            iMessageType = PERIODIC_MESSAGE;
        }

        if ((iMessageType != INIT_MESSAGE) && (iMessageType != PERIODIC_MESSAGE))
        {
            continue;
        }

        QByteArray appdata = todTcmsTelegram.appData(2);
        appdata.append(ETX);
        int appsize = appdata.size();
        qint8 crc = MyCrc::calculateBcc(appdata);

//        qDebug() << "MyCrc::calculateBcc(appdata) : "<< crc;
        if(crc != inputCrc )
        {
            continue;
        }

        m_comConnectStateFlag = true;
        if (iMessageType == PERIODIC_MESSAGE)
        {
            m_receiveAppData = todTcmsTelegram.appData(4);
//            qDebug()<<"Rec Serial appdata size: "<< m_receiveAppData.size();
            emit updateRecvWidgetData();
        }
     }
}

QByteArray TodCommChannel::receiveData()
{
    if(m_comConnectStateFlag)
        return m_receiveAppData;
    else
        return NULL;
}

bool TodCommChannel::getComConnectState()
{
    return m_comConnectStateFlag;
}

void TodCommChannel::setComConnectState(bool setStateFlag)
{
    m_comConnectStateFlag = setStateFlag;
}

void TodCommChannel::setDataByByteSize(const QString& byteOffset,const QString& bitOffset,const QString& sizetype,const QString& numType, const QString& value)
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

    if(numType == "Hex")
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

//    qDebug("setDataByByteSize: byteOffNum-%d,bitOffNum-%d,m_pucData[70]-0x%2x",byteOffNum,bitOffNum,m_pucData[70]);
}

void TodCommChannel::setOptionTag(int byteoffset, int tag)
{
    m_pucData[byteoffset] = (unsigned char) tag;
}



void TodCommChannel::setDataByInfoFault(const QList<int> &lst, int offset, int type)
{
    int count = lst.size();
    int byteOffSet = offset;
    m_pucData[byteOffSet] = (unsigned char) (count + 1);
//    qDebug() << "type:" << type;
    m_pucData[++byteOffSet] = (unsigned char) type;
    for(int i = 1; i <= count; ++i)
    {
        m_pucData[byteOffSet + i] = (unsigned char) lst[i-1];
    }
    m_actualDataSize = byteOffSet + count;
}

void TodCommChannel::setDataByTCMS(const QStringList &slst, int offset)
{
    int byteOffSet = offset;
    int count = slst.size();
    for(int i = 0; i < count; ++i)
    {
        bool ok;
        m_pucData[byteOffSet + i] = (unsigned char)((slst[i]).toLongLong(&ok, 16));
    }
    m_actualDataSize = byteOffSet + count;
}



void TodCommChannel::sendDataTelegram(QString cmd)
{
    QByteArray bg = QByteArray::fromRawData((const char*)m_pucData, m_actualDataSize);   // 由于需要在qUdpsocket发送数据需要QByteArray类型，所以需要对unsigned char[]与QByteArray进行转化；
    if(cmd == "Udp")
    {
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
        telegram.setAppData(bg);
        telegram.updateCRC();

        qDebug() << "m_actualDataSize is "<< m_actualDataSize;

        for (int i = 0; i < 34 + m_actualDataSize; i++)
        {
            qDebug("-- m_pucData[%d], %x", i, (int)(telegram.data()[i]));
        }

        qDebug() << "send Udp telegram size:" << telegram.size();

        m_socket->writeDatagram(telegram, m_todAddress, m_todPort);
    }
    else if(cmd == "Serial")
    {
        TodTcmsTelegram telegram = TodTcmsTelegram::createTelegram(bg, m_actualDataSize, 4);
        telegram.setAppDataDeviceAddress(0x05);
        telegram.setAppDataCommType(0x0F);
        telegram.setTelegramHeader(0x1002);
        telegram.setTelegramTail(0x03);
        telegram.updateCRC();

        QByteArray sendArr = telegram.encoderPacket();

//        for (int i = 0; i < sendArr.size(); i++)
//        {
//            qDebug("-- m_pucData[%d], %x", i, (int)(sendArr.data()[i]));
//        }

        qDebug() << "send Serial telegram size:" << sendArr.size();

        m_serialDriver->write(sendArr.data(),sendArr.size());
    }
}

void TodCommChannel::sendDataTelegramForRFC(QString cmd)
{

    if(cmd == "Udp")
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
        telegram.setAppData(bg);
        telegram.updateCRC();

        qDebug() << "send RFC telegram size:" << telegram.size();
        for (int i = 0; i < 34 + m_actualDataSize; i++)
        {
            qDebug("-- m_pucData[%d], %x", i, (int)(telegram.data()[i]));
        }

       m_socket->writeDatagram(telegram, m_todAddress, m_todPort);
    }
    else if(cmd == "Serial")
    {
        QByteArray arr ;
        arr.resize(1);
        arr.data()[0] = 0;
        TodTcmsTelegram telegram = TodTcmsTelegram::createTelegram(arr, 8, 4);
        telegram.setAppDataDeviceAddress(0x05);
        telegram.setAppDataCommType(0x07);
        telegram.setTelegramHeader(0x1002);
        telegram.setTelegramTail(0x03);
        telegram.updateCRC();

        QByteArray sendArr = telegram.encoderPacket();

//        for (int i = 0; i < sendArr.size(); i++)
//        {
//            qDebug("-- m_pucData[%d], %x", i, (int)(sendArr.data()[i]));
//        }

//        qDebug() << "send INIT telegram size:" << sendArr.size();

        m_serialDriver->write(sendArr.data(),sendArr.size());
    }
}

void TodCommChannel::setTSN(quint16 TSN)
{
    m_TSN = TSN;
}

void TodCommChannel::setRSN(quint16 RSN)
{
    m_RSN = RSN;
}

quint16 TodCommChannel::getTSN()
{
    return m_TSN;
}

quint16 TodCommChannel::getRSN()
{
    return m_RSN;
}

void TodCommChannel::setSerialPortName(QString name)
{
    m_serialDriver->setPortName(name);
}

bool TodCommChannel::openSerialPort(int baudRate)
{
    bool isOpen = false;
    if(m_serialDriver->open(QIODevice::ReadWrite))
    {
        m_serialDriver->setBaudRate(baudRate, QSerialPort::AllDirections);
        m_serialDriver->setDataBits(QSerialPort::Data8);
        m_serialDriver->setParity(QSerialPort::EvenParity);
        m_serialDriver->setStopBits(QSerialPort::OneStop);
        isOpen = true;
    }
    return isOpen;
}

bool TodCommChannel::isSerialPortOpen()
{
    return m_serialDriver->isOpen();
}

void TodCommChannel::closeSerialPort()
{
    m_serialDriver->close();
}


int TodCommChannel::checkTcmsTelegram(QByteArray &baTelegram, bool &bValid, int &iStart, int &iEnd, qint8& crc)
{
    unsigned char* data = (unsigned char*)baTelegram.data();

    int dataSize = baTelegram.size();

     bValid = false;

    int iStartIndex = -1;
    int iEndIndex = -1;
    int temp = 1;

    m_serialInputQueue.clear();

    if(dataSize < 3)
    {
       temp = -1;
    }

    for(int i = 0; i < dataSize - 1; i++)
    {
        if (iStartIndex == -1)
        {
            if((data[i] == DLE) && (data[i+1] == STX))
            {
                if(i != 0)
                    if(data[i-1] == DLE)
                        continue;

                iStartIndex = i;
            }
            else if(i == dataSize - 2)
            {
                temp = -1;
            }
        }
        else
        {
            if((data[i] == DLE) && (data[i+1] == ETX) && (iEndIndex == -1))
            {
                if(i != 0)
                {
                    if(data[i-1] == DLE)
                    {
                        continue;
                    }
                }

                iEndIndex = i + 1;

                if((iStartIndex != -1) && (iEndIndex > iStartIndex))
                    break;
            }
            else if( i == dataSize - 2)
            {
                temp = -1;
            }
        }
    }

    if( (iStartIndex != -1) && (iEndIndex > iStartIndex))
    {
        if(dataSize -iEndIndex > 1)
        {
            int j = iStartIndex;
            for(; j <= iEndIndex; j++)
            {
                 m_serialInputQueue.append((char)data[j]);
            }
            crc = (qint8) data[j];
            bValid = true;
            iStart = iStartIndex;
            iEnd = iEndIndex;
        }
        else
        {
            temp = -1;
        }
    }
    return temp;
}

void TodCommChannel::setTodAdress(const QString &todAddress)
{
//    qDebug() << "TodCommChannel::setTodAdress, todAddress" << todAddress;
    m_todAddress = todAddress;
}

void TodCommChannel::setVobcAdress(const QString &vobcAddress)
{
//    qDebug() << "TodCommChannel::setVobcAdress, todAddress" << vobcAddress;
    m_vobcAddress = vobcAddress;
}

void TodCommChannel::setTodPort(quint16 todPort)
{
//    qDebug() << "TodCommChannel::setTodPort, todPort" << todPort;
    m_todPort = todPort;
}

void TodCommChannel::setVobcPort(quint16 vobcPort)
{
//    qDebug() << "TodCommChannel::setVobcPort, vobcPort" << vobcPort;
    m_vobcPort = vobcPort;
}

void TodCommChannel::setTodId(quint16 id)
{
//    qDebug() << "TodCommChannel::setTodId, id" << id;
    m_todId = id;
}

QDateTime TodCommChannel::getCurDateTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    return dateTime;
}

QString TodCommChannel::getLocalIp()
{
    QString strLocalAddress;
    QList<QHostAddress> localAddress = QNetworkInterface::allAddresses();
    int iSize = localAddress.size();
    for (int i = 0; i < iSize; ++i) {
//        qDebug() << "TodCommChannel::getLocalIp()" << localAddress.at(i) << localAddress.at(i).toString() << localAddress.at(i).toIPv4Address() << QHostAddress::LocalHost;
        if ((localAddress.at(i) != QHostAddress::LocalHost)
                && localAddress.at(i).toIPv4Address()) {
            strLocalAddress = localAddress.at(i).toString();
            break;
        }
    }
//    qDebug() << "TodCommChannel::getLocalIp()" << strLocalAddress;
    return strLocalAddress;
}

bool TodCommChannel::checkTelegram(const TodVobcTelegram &telegram)
{
    //if (!telegram.isValid()) // for test
    //    return false;

    if (telegram.interfaceType() != 1004) {
        qWarning("TodCommChannel: the interface type invalid");
        return false;
    }

    if (telegram.receiverClass() != 1) {
        qWarning("TodCommChannel: the receiver class invalid");
        return false;
    }

    if (telegram.receiverID() != m_todId) {
        qWarning("TodCommChannel: the receiver id invalid");
        qDebug() << "Receiver id : " << telegram.receiverID();
        qDebug() << "Tod id : " << m_todId;
        return false;
    }

    if (telegram.transmitterClass() != 6) {
        qWarning("TodCommChannel: the transmitter class invalid");
        return false;
    }

    return true;
}

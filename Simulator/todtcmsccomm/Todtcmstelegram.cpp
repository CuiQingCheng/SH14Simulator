#include "Todtcmstelegram.h"
#include "../CRC/mycrc.h"

#define DLE 0x10
#define STX 0x02
#define ETX 0x03


TodTcmsTelegram::TodTcmsTelegram(int size)
    : QByteArray(size, 0)
{

}

TodTcmsTelegram::TodTcmsTelegram(const QByteArray &datagram)
    : QByteArray(datagram)
{

}

void TodTcmsTelegram::setTelegramHeader(quint16 header)
{
    char *d = data();
    d[1] = header& 0xff;                                 // 把该16位的数分高低字节分别存进数组d的第0个元素和0个元素；
    d[0] = header >> 8;
}

void TodTcmsTelegram::setTelegramTail(quint8 tail)
{
    char *d = data();
    int sz = size();

    d[sz -1] = tail;
}

void TodTcmsTelegram::setAppDataDeviceAddress(quint8 addr)
{
    char *d = data();
    d[2] = addr;
}

quint8 TodTcmsTelegram::appDataDeviceAddress()
{
    QByteArray d = appData(2);
    return (quint8)(d.data()[0]);
}

void TodTcmsTelegram::setAppDataCommType(quint8 type)
{
    char *d = data();
    d[3] = type;
}

quint8 TodTcmsTelegram::appDataCommType()
{
    QByteArray d = appData(2);
    return (quint8)(d.data()[1]);
}

void TodTcmsTelegram::updateCRC()
{
    QByteArray d = appData(2);


    qint8 crcCalc = MyCrc::calculateBcc(d);
    append(crcCalc);
}

QByteArray TodTcmsTelegram::appData(int pos )
{
    QByteArray arr = mid(pos);

   return arr ;
}

void TodTcmsTelegram::setAppData(const QByteArray &srceData, int pos)
{
    char *d = data();
    int size = srceData.size();
    memcpy(d + pos, srceData.data(),  size);
}


TodTcmsTelegram TodTcmsTelegram::createTelegram(const QByteArray &appData , quint8 size,int appDataPos)
{
    TodTcmsTelegram tg(size);
    tg.setAppData(appData, appDataPos);
    return tg;
}

QByteArray TodTcmsTelegram::encoderPacket()
{
    int sz = size();
    char *d = data();
    QByteArray packetArr;
    packetArr.append(d[0]);
    int i = 1;
    for( ; i <= sz-2; ++i)
    {
        if(d[i] == DLE || (i  == sz-2 && d[i] == ETX))
        {
            packetArr.append(DLE);
        }
        packetArr.append(d[i]);
    }
    packetArr.append(d[sz - 1]);

    return packetArr;
}

TodTcmsTelegram TodTcmsTelegram::decoderPacket(QByteArray &baTelegram)
{
    int dataSize = baTelegram.size();
    baTelegram.remove(dataSize-2,2);
//    baTelegram.remove(0,2);

    QByteArray oneDLE;
    oneDLE.append(DLE);

    QByteArray twoDLE;
    twoDLE.append(DLE);
    twoDLE.append(DLE);

    baTelegram.replace(twoDLE,oneDLE);
    int size = baTelegram.size();
    TodTcmsTelegram tg(size);

    tg.setAppData(baTelegram, 0);
    return tg;
}


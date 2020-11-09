#include "Todvobctelegram.h"
#include "../CRC/mycrc.h"

#include <QDateTime>



TodVobcTelegram::TodVobcTelegram(int size)
    : QByteArray(size, 0)
{

}

TodVobcTelegram::TodVobcTelegram(const QByteArray &datagram)
    : QByteArray(datagram)
{

}

quint16 TodVobcTelegram::interfaceType() const
{
    const char *d = data();
    return ((quint8)d[1] << 8) | (quint8)d[0];
}

void TodVobcTelegram::setInterfaceType(quint16 interfaceType)
{
    char *d = data();
    d[0] = interfaceType & 0xff;
    d[1] = interfaceType >> 8;
}

quint8 TodVobcTelegram::interfaceVersion() const
{
    const char *d = data();
    return (quint8)d[2];
}

void TodVobcTelegram::setInterfaceVersion(quint8 interfaceVersion)
{
    char *d = data();
    d[2] = interfaceVersion;
}

quint8 TodVobcTelegram::telegramType() const
{
    const char *d = data();
    return (quint8)d[3];
}

void TodVobcTelegram::setTelegramType(quint8 telegramType)
{
    char *d = data();
    d[3] = telegramType;
}

quint16 TodVobcTelegram::receiverClass() const
{
    const char *d = data();
    return ((quint8)d[5] << 8) | (quint8)d[4];
}

void TodVobcTelegram::setReceiverClass(quint16 receiverClass)
{
    char *d = data();
    d[4] = receiverClass & 0xff;
    d[5] = receiverClass >> 8;
}

quint16 TodVobcTelegram::receiverID() const
{
    const char *d = data();
    return ((quint8)d[7] << 8) | (quint8)d[6];
}

void TodVobcTelegram::setReceiverId(quint16 receiverId)
{
    char *d = data();
    d[6] = receiverId & 0xff;
    d[7] = receiverId >> 8;
}

quint16 TodVobcTelegram::transmitterClass() const
{
    const char *d = data();
    return ((quint8)d[9] << 8) | (quint8)d[8];
}

void TodVobcTelegram::setTransmitterClass(quint16 transmitterClass)
{
    char *d = data();
    d[8] = transmitterClass & 0xff;
    d[9] = transmitterClass >> 8;
}

quint16 TodVobcTelegram::transmitterId() const
{
    const char *d = data();
    return ((quint8)d[11] << 8) | (quint8)d[10];
}

void TodVobcTelegram::setTransmitterId(quint16 transmitterId)
{
    char *d = data();
    d[10] = transmitterId & 0xff;
    d[11] = transmitterId >> 8;
}

QDateTime TodVobcTelegram::dateTime() const
{
    const char *d = data();
    quint16 year = ((quint8)d[13] << 8) | (quint8)d[12];
    quint16 mon = ((quint8)d[15] << 8) | (quint8)d[14];
    quint16 day = ((quint8)d[17] << 8) | (quint8)d[16];
    quint16 hour = ((quint8)d[19] << 8) | (quint8)d[18];
    quint16 min = ((quint8)d[21] << 8) | (quint8)d[20];
    quint16 sec = ((quint8)d[23] << 8) | (quint8)d[22];
    quint16 msec = ((quint8)d[25] << 8) | (quint8)d[24];
    return QDateTime(QDate(year, mon, day), QTime(hour, min, sec, msec));
}

void TodVobcTelegram::setDateTime(QDateTime dateTime)
{
    QDate dt = dateTime.date();
    QTime tm = dateTime.time();

    char *d = data();

    quint16 year = dt.year();
    d[12] = year & 0xff;
    d[13] = year >> 8;

    quint16 mon = dt.month();
    d[14] = mon & 0xff;
    d[15] = mon >> 8;

    quint16 day = dt.day();
    d[16] = day & 0xff;
    d[17] = day >> 8;

    quint16 hour = tm.hour();
    d[18] = hour & 0xff;
    d[19] = hour >> 8;

    quint16 min = tm.minute();
    d[20] = min & 0xff;
    d[21] = min >> 8;

    quint16 sec = tm.second();
    d[22] = sec & 0xff;
    d[23] = sec >> 8;

    quint16 msec = tm.msec();
    d[24] = msec & 0xff;
    d[25] = msec >> 8;
}

quint16 TodVobcTelegram::RSN() const
{
    const char *d = data();
    return ((quint8)d[27] << 8) | (quint8)d[26];
}

void TodVobcTelegram::setRSN(quint16 RSN)
{
    char *d = data();
    d[26] = RSN & 0xff;                                 // 把该16位的数分高低字节分别存进数组d的第26个元素和27个元素；
    d[27] = RSN >> 8;
}

quint16 TodVobcTelegram::TSN() const
{
    const char *d = data();
    return ((quint8)d[29] << 8) | (quint8)d[28];
}

void TodVobcTelegram::setTSN(quint16 TSN)
{
    char *d = data();
    d[28] = TSN & 0xff;
    d[29] = TSN >> 8;
}

quint16 TodVobcTelegram::appDataLength() const
{
    const char *d = data();
    return ((quint8)d[31] << 8) | (quint8)d[30];
}

void TodVobcTelegram::setAppDataLength(quint16 appDataLength)
{
    char *d = data();
    d[30] = appDataLength & 0xff;
    d[31] = appDataLength >> 8;
}

const char *TodVobcTelegram::appData() const
{
    return data() + 32;
}

void TodVobcTelegram::setAppData(const QByteArray &srceData)
{
    char *d = data();
    int length = appDataLength();
    int size = srceData.size();
    memcpy(d + 32, srceData.data(), length > size ? size : length);
}

void TodVobcTelegram::TodVobcTelegram::updateCRC()
{
    char *d = data();

    int sz = size();

    quint16 crcCalc = MyCrc::calculate16(d, sz - 2);
    d[sz - 2] = crcCalc & 0xff;
    d[sz - 1] = crcCalc >> 8;
}


TodVobcTelegram TodVobcTelegram::createTelegram(quint16 appDataLength)
{
    TodVobcTelegram tg(appDataLength + 34);
    tg.setAppDataLength(appDataLength);
    return tg;
}

TodVobcTelegram TodVobcTelegram::createTelegram(const QByteArray &appData)
{
    TodVobcTelegram tg(appData.size() + 34);
    tg.setAppDataLength(appData.size());
    tg.setAppData(appData);
    return tg;
}

#include "Todvobctelegram.h"

#include <QDateTime>

static unsigned short crc16_table[256] = {
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
    0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
    0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
    0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
    0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
    0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
    0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
    0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
    0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
    0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
    0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
    0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
    0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
    0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
    0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
    0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
    0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
    0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
    0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
    0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
    0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
    0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
    0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

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

    quint16 crcCalc = calculate(d, sz - 2);
    d[sz - 2] = crcCalc & 0xff;
    d[sz - 1] = crcCalc >> 8;
}

unsigned short TodVobcTelegram::calculate(const char *data, int len)
{
    unsigned short crc = 0;

    while (len-- > 0)
        crc = (crc >> 8) ^ crc16_table[(crc ^ *(unsigned char *)data++) & 0xff];

    return crc;
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

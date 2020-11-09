#ifndef TODTCMSTELEGRAM_H
#define TODTCMSTELEGRAM_H

#include <QByteArray>

class TodTcmsTelegram : public QByteArray
{
  public:
    TodTcmsTelegram(int size);
    TodTcmsTelegram(const QByteArray &datagram);

    void setTelegramHeader(quint16 header);
    void setTelegramTail(quint8 tail);

    void setAppDataDeviceAddress(quint8 addr);
    quint8 appDataDeviceAddress();

    void setAppDataCommType(quint8 type);
    quint8 appDataCommType();

    void updateCRC();

    QByteArray appData(int pos);
    void setAppData(const QByteArray &srceData, int pos);

    static TodTcmsTelegram createTelegram(const QByteArray &appData, quint8 size, int appDataPos);

    QByteArray encoderPacket();
    static  TodTcmsTelegram decoderPacket(QByteArray &baTelegram);

};


#endif

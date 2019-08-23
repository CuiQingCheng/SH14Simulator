#ifndef TODVOBCTELEGRAM_H
#define TODVOBCTELEGRAM_H

#include <QByteArray>


class QDateTime;

class TodVobcTelegram : public QByteArray
{
public:
    enum TelegramType {
        RFC = 1,
        Poll = 4,
        Response = 5
    };

    TodVobcTelegram(int size);
    TodVobcTelegram(const QByteArray &datagram);

    quint16 interfaceType() const;
    void setInterfaceType(quint16 interfaceType);

    quint8 interfaceVersion() const;
    void setInterfaceVersion(quint8 interfaceVersion);

    quint8 telegramType() const;
    void setTelegramType(quint8 telegramType);

    quint16 receiverClass() const;
    void setReceiverClass(quint16 receiverClass);

    quint16 receiverID() const;
    void setReceiverId(quint16 receiverId);

    quint16 transmitterClass() const;
    void setTransmitterClass(quint16 transmitterClass);

    quint16 transmitterId() const;
    void setTransmitterId(quint16 transmitterId);

    QDateTime dateTime() const;
    void setDateTime(QDateTime dateTime);

    quint16 RSN() const;
    void setRSN(quint16 RSN);

    quint16 TSN() const;
    void setTSN(quint16 TSN);

    quint16 appDataLength() const;
    void setAppDataLength(quint16 appDataLength);

    const char *appData() const;
    void setAppData(const QByteArray &srceData);

    void updateCRC();
    static unsigned short calculate(const char *data, int len);

    static TodVobcTelegram createTelegram(quint16 appDataLength);
    static TodVobcTelegram createTelegram(const QByteArray &appDate);
};

#endif // TODVOBCTELEGRAM_H

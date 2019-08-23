#ifndef TELEGRAM_H
#define TELEGRAM_H

#include "Signal/SignalValue.h"
class Telegram
{
public:
    Telegram();
    ~Telegram();
    void setSendSignalMap(SignalValue* signalValue);
    void setReceiveSignalMap(SignalValue* signalValue);

    SignalValue* getSendSignalValue(QString& key);
    SignalValue* getReceiveSignalValue(QString& key);

    void clear();

private:
    SignalMap m_sendSignalMap;
    SignalMap m_receiveSignalMap;

    QByteArray m_sendAppData{113, 0};
};

#endif // TELEGRAM_H

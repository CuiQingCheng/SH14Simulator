#include "Telegram.h"
#include <QDebug>
Telegram::Telegram()
{

}

Telegram::~Telegram()
{
    qDebug() << "Telegram::~Telegram()";
    SignalMap::iterator itS = m_sendSignalMap.begin();
    while(itS != m_sendSignalMap.end())
    {
        SignalValue* signalValue = itS.value();
        if(signalValue != NULL)
        {
            delete signalValue;
            signalValue = NULL;
        }
        ++itS;
    }

    SignalMap::iterator itR = m_receiveSignalMap.begin();
    while(itR != m_receiveSignalMap.end())
    {
        SignalValue* signalValue = itR.value();
        if(signalValue != NULL)
        {
            delete signalValue;
            signalValue = NULL;
        }
        ++itR;
    }
}

void Telegram::setSendSignalMap(SignalValue* signalValue)
{
    if(signalValue != NULL)
    {
        if(!m_sendSignalMap.contains(signalValue->getName()))
        {
            m_sendSignalMap.insert(signalValue->getName(), signalValue);
        }
        else
        {
            qDebug("m_SendSignalMap Duplicated signal name : %s", signalValue->getName().toLatin1().data());
        }
    }
}

void Telegram::setReceiveSignalMap(SignalValue* signalValue)
{
    if(signalValue != NULL)
    {
        if(!m_receiveSignalMap.contains(signalValue->getName()))
        {
            m_receiveSignalMap.insert(signalValue->getName(), signalValue);
        }
        else
        {
            qDebug("m_SendSignalMap Duplicated signal name : %s", signalValue->getName().toLatin1().data());
        }
    }
}

SignalValue *Telegram::getSendSignalValue(QString &key)
{
    if(m_sendSignalMap.contains(key))
        return m_sendSignalMap.value(key);
    else
        return NULL;
}

SignalValue *Telegram::getReceiveSignalValue(QString &key)
{
    if(m_receiveSignalMap.contains(key))
        return m_receiveSignalMap.value(key);
    else
        return NULL;
}

void Telegram::clear()
{


    SignalMap::iterator itS = m_sendSignalMap.begin();

    for(; itS != m_sendSignalMap.end(); )
    {
        itS = m_sendSignalMap.erase(itS++);
    }

    qDebug() << "check empty:" << m_sendSignalMap.isEmpty();
    SignalMap::iterator itR = m_receiveSignalMap.begin();
    for(; itR != m_receiveSignalMap.end(); )
    {
        itR = m_receiveSignalMap.erase(itR++);
    }
}

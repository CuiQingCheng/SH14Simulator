#include "SignalValue.h"
#include <QDebug>
SignalValue::SignalValue(const QString& signalName, const QString& type, quint16 byteOffset, quint8 bitOffset, quint32 defaultValue, const QString& valueType, const QString& property) :
    m_isSignalValid(false),
    m_byteSize(1),
    m_bitMask(0x01),
    m_isValueChanged(false),
    m_value(0),
    m_valueType(valueType),
    m_dataType(type),
    m_isQueued(false)
{
    // Basic information copy.
    m_name = signalName;
    m_byteOffset = byteOffset;
    m_bitOffset = bitOffset;
    m_value = defaultValue;

    if("queued" == property)
    {
        m_isQueued = true;
        m_queuedValue.append(0);
    }

    // Handling signal type, calculate bitMask for signal.
    QStringList strLst = type.split(".");

    if(strLst.size() != 2)
    {
        m_isSignalValid = false;
        qDebug() << signalName << " parse failed !!! " << type << " should split by '.'";
        return;
    }

    if(type.startsWith("UNSIGNED"))
    {
        m_byteSize = strLst.at(1).toInt() / 8;
        m_bitSize = 8;
        m_bitMask = 0xFF;
    }
    else if(type.startsWith("BOOLEAN"))
    {
        m_bitSize = strLst.at(1).toInt();
        m_byteSize = 1;

        // Currently only support BOOLEAN.1
        for(int i = 0; i < m_bitSize; ++i)
        {
            m_bitMask |= 1 << 0;
        }
    }

    m_isSignalValid = true;
}

SignalValue::SignalValue(const SignalValue &rhs)
{
    m_name = rhs.m_name;
    m_byteOffset = rhs.m_byteOffset;
    m_bitOffset = rhs.m_bitOffset;
    m_byteSize = rhs.m_byteSize;
    m_bitSize = rhs.m_bitSize;
    m_bitMask = rhs.m_bitMask;
    m_value = rhs.m_value;
    m_isQueued = rhs.m_isQueued;
    m_isValueChanged = rhs.m_isValueChanged;
    m_isSignalValid = rhs.m_isSignalValid;
    m_queuedValue = rhs.m_queuedValue;
    m_valueType = rhs.m_valueType;
    m_dataType = rhs.m_dataType;
}

SignalValue &SignalValue::operator=(const SignalValue &rhs)
{
    if(this != &rhs)
    {
        m_name = rhs.m_name;
        m_byteOffset = rhs.m_byteOffset;
        m_bitOffset = rhs.m_bitOffset;
        m_byteSize = rhs.m_byteSize;
        m_bitSize = rhs.m_bitSize;
        m_bitMask = rhs.m_bitMask;
        m_value = rhs.m_value;
        m_isQueued = rhs.m_isQueued;
        m_isValueChanged = rhs.m_isValueChanged;
        m_isSignalValid = rhs.m_isSignalValid;
        m_queuedValue = rhs.m_queuedValue;
        m_valueType = rhs.m_valueType;
        m_dataType = rhs.m_dataType;
    }

    return *this;
}

SignalValue::SignalValue(const QString& signalName)
{
    m_name = signalName;
    m_byteOffset = 0;
    m_bitOffset = 0;
    m_byteSize = 0;
    m_bitSize = 0;
    m_bitMask = 0;
    m_value = 0;
    m_isQueued = false;
    m_isValueChanged = false;
    m_isSignalValid = false;
    m_valueType = "hexadecimal";
    m_dataType = "UNSIGNED.8";
}

SignalValue::SignalValue()
{
    m_name = "";
    m_byteOffset = 0;
    m_bitOffset = 0;
    m_byteSize = 0;
    m_bitSize = 0;
    m_bitMask = 0;
    m_value = 0;
    m_isQueued = false;
    m_isValueChanged = false;
    m_isSignalValid = false;
    m_valueType = "hexadecimal";
    m_dataType = "UNSIGNED.8";
}

void SignalValue::setName(const QString& name)
{
    m_name = name;
}

QString SignalValue::getName() const
{
    return m_name;
}

quint32 SignalValue::getValue()
{
    QMutexLocker locker(&m_mutex);
    m_isValueChanged = false;

    if(m_isQueued)
    {
        getQueuedValue();
    }

    return m_value;
}

void SignalValue::getQueuedValue()
{
    m_value = m_queuedValue.last();

    if(m_queuedValue.size() == 1)
    {
        m_queuedValue[0] = 0;
    }
    else
    {
        m_queuedValue.removeLast();
    }
}

void SignalValue::setValue(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);

    if((m_byteOffset + m_byteSize) > data.size())
    {
        qDebug("Signal out of range : %s, byteOffset : %d, byteSize : %d, incoming data size : %d", m_name.toLatin1().data(), m_byteOffset, m_byteSize, data.size());
        return;
    }

    quint32 newValue = 0;

    for(int i = 0, j = m_byteOffset; i < m_byteSize; ++i, ++j)
    {
        newValue |= ((static_cast<quint32>(data[j]) >> m_bitOffset) << (i * 8)) & (static_cast<quint32>(m_bitMask) << (i * 8));
    }

    if(m_value != newValue)
    {
        m_value = newValue;
        m_isValueChanged = true;
        //LOG_DEBUG("Signal(%s) gets new value : %d.", m_name.toLatin1().data(), m_value);
    }
    else
    {
        m_isValueChanged = false;
    }
}

void SignalValue::setValue(const quint32& value)
{
    if(m_isQueued)
    {
        m_queuedValue.append(value);
    }
    else
    {
        m_value = value;
    }
}

/**
 * @brief setData put the signal value into data set with its predefined offset and value type.
 * @param data
 */
void SignalValue::setData(QByteArray& data)
{
    if((m_byteOffset + m_byteSize) > data.size())
    {
        qDebug("Signal out of range : %s, byteOffset : %d, byteSize : %d, outgoing data size : %d", m_name.toLatin1().data(), m_byteOffset, m_byteSize, data.size());
        return;
    }

    if(m_isQueued)
    {
        getQueuedValue();
    }

    for(int i = 0, j = m_byteOffset; i < m_byteSize; ++i, ++j)
    {
        data[j] = data[j] | (static_cast<char>((m_value >> (i * 8)) << m_bitOffset));
    }
}

bool SignalValue::isValid()
{
    return m_isSignalValid;
}

bool SignalValue::isChanged()
{
    return m_isValueChanged;
}

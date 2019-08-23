#ifndef SIGNALVALUE_H
#define SIGNALVALUE_H

#include <QString>
#include <QStringList>
#include <QMutex>
#include <QVector>
#include <QList>
#include <QByteArray>
#include <QMap>

typedef QList<quint32> QueuedValue;

class SignalValue
{
public:
    SignalValue(const QString& signalName, const QString& type, quint16 byteOffset, quint8 bitOffset, quint32 defaultValue,const QString& valueType, const QString& property="null");

    SignalValue(const SignalValue& rhs);

    SignalValue& operator=(const SignalValue& rhs);

    SignalValue(const QString& signalName);

    SignalValue();

    void setName(const QString& name);

    QString getName() const;

    quint32 getValue();
    void getQueuedValue();

    void setValue(const QByteArray& data);
    void setValue(const quint32& value);

    /**
     * @brief setData put the signal value into data set with its predefined offset and value type.
     * @param data
     */
    void setData(QByteArray& data);

    bool isValid();
    bool isChanged();

    inline quint16 getByteOffset()
    {
        return m_byteOffset;
    }

    inline quint8 getBitOffset()
    {
        return m_bitOffset;
    }

    inline QString getValueType()
    {
        return m_valueType;
    }

    inline QString getDataType()
    {
        return m_dataType;
    }


private:

    QString m_name;
    QMutex  m_mutex;
    quint16 m_byteOffset;
    quint8  m_bitOffset;
    bool    m_isSignalValid;
    int     m_byteSize;
    int     m_bitSize;
    quint8  m_bitMask;
    bool    m_isValueChanged;
    quint32  m_value;
    QueuedValue m_queuedValue;
    bool    m_isQueued;
    QString m_valueType;
    QString m_dataType;
};

typedef QMap<QString, SignalValue*> SignalMap;



#endif // SIGNALVALUE_H

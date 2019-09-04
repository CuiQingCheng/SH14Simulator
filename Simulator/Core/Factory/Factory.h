#ifndef FACTORY_H
#define FACTORY_H

#include "Core/Handler/AutoTestHandler.h"
#include "Core/Handler/WidgetHandler.h"
#include "../../Telegram/Telegram.h"


#include <QDebug>
#include <QString>

class Factory
{
public:
    Factory();
    ~Factory();

    template<typename T>
    T* get(const QString& object)
    {
        T* obj = NULL;

        if("AutoTestHandler" == object)
        {
            if(m_autoTestHandler == NULL)
            {
                m_autoTestHandler = new AutoTestHandler;
                qDebug("Creating object : %s", object.toLatin1().data());
            }

            obj = dynamic_cast<T*>(m_autoTestHandler);
        }
        else if("WidgetHandler" == object)
        {
            if(m_widgetHandler == NULL)
            {
                m_widgetHandler = new WidgetHandler;
                qDebug("Creating object : %s", object.toLatin1().data());
            }

            obj = dynamic_cast<T*>(m_widgetHandler);
        }
        else if("Telegram" == object)
        {
            if(m_telegram == NULL)
            {
                m_telegram = new Telegram;
                qDebug("Creating object : %s", object.toLatin1().data());
            }
            obj = dynamic_cast<T*>(m_telegram);
        }

        if(NULL == obj)
        {
            qDebug("Get object failed. Object name : %s", object.toLatin1().data());
        }

        return obj;
    }


private:
    AutoTestHandler* m_autoTestHandler;
    WidgetHandler* m_widgetHandler;
    Telegram* m_telegram;

};

#endif

#include "Factory.h"

Factory::Factory()
    : m_autoTestHandler(NULL)
    , m_widgetHandler(NULL)
    , m_telegram(NULL)
{

}

Factory::~Factory()
{

    if(m_autoTestHandler != NULL)
    {
        delete m_autoTestHandler;
        m_autoTestHandler = NULL;
    }

    if(m_widgetHandler != NULL)
    {
        delete m_widgetHandler;
        m_widgetHandler = NULL;
    }

    if(m_telegram != NULL)
    {
        delete m_telegram;
        m_telegram = NULL;
    }
}

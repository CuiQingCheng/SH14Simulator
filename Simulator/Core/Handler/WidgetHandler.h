#ifndef WIDGETHANDLER_H
#define WIDGETHANDLER_H

#include "Gui/TextEdit.h"
#include "Gui/CheckBox.h"
#include "Gui/LineEdit.h"

#include <QObject>
#include <QStringList>
#include <QString>
#include <QWidget>
#include <QTableWidget>
#include <QTabWidget>

typedef QMap<QString, QWidget*> WidgetMap;

class WidgetHandler : public QObject
{
    Q_OBJECT
public:
    WidgetHandler();
    ~WidgetHandler();
    void addEleNameList(QString name, int type);

    void addWidget( QString& key, QWidget* widget);
    WidgetMap* getWidgetMap();

    enum SignalType{
        Send_Signal = 0,
        Receiver_Signal = 1,
        Info = 2,
        Fault =3
    };

private:
    QStringList m_sendSignalList;
    QStringList m_receiveSignalList;
    QStringList m_infoList;
    QStringList m_faultList;

    WidgetMap m_widgetMap;
};

#endif

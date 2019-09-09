#ifndef WIDGETHANDLER_H
#define WIDGETHANDLER_H

#include "Gui/TextEdit.h"
#include "Gui/CheckBox.h"
#include "Gui/LineEdit.h"
#include "Telegram/Telegram.h"

#include <QObject>
#include <QStringList>
#include <QString>
#include <QWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QList>
#include <QGridLayout>
#include <QMap>

typedef QMap<QString, QWidget*> WidgetMap;

typedef struct Tcms_Item{
    static const int ROW_COUNT = 6;
    static const int COLUMN_COUNT = 16;
    static const int ITEM_W = 70;
    static const int ITEM_H = 28;
}tcmsItem;

class WidgetHandler : public QObject
{
    Q_OBJECT
public:
    WidgetHandler();
    ~WidgetHandler();
    enum SignalType{
        Send_Signal = 0,
        Receiver_Signal = 1,
        Info = 2,
        Fault =3
    };

    enum TrainNumber{
        Shanghai_14 = 0,
        Wuhan_11 = 1
    };

    static const int RADIO_W = 225;
    static const int RADIO_H = 22;
    static const int LINECOUNT = 5;


    void addEleNameList(QString name, int type);

    void addWidget( QString key, QWidget* widget);
    WidgetMap* getWidgetMap();

    void setTelegram(Telegram* telegram);
    void clear();

    QList<int> getInfoFaultIdLst(bool isInfo);
    void getTcmsValueLst(QStringList& lst);

    void setTrainNumber(TrainNumber num)
    {
        m_currentTrainNumber = num;
    }
    TrainNumber getTrainNumber()
    {
        return m_currentTrainNumber;
    }

signals:
    void sendTelegramUpdated();

public slots:
    void showGuiDefData();
    void updateInfoFaultId(int state);
    void updateSignalValue(QString signal, QString value);

protected:
    void drawTableWidget();
    void drawInfoFaultCheckTab();
    void drawTcmsTableWidget();

private:
    QList<CheckBox*> m_infoCheckBoxList;
    QList<CheckBox*> m_faultCheckBoxList;


    QStringList m_sendSignalList;
    QStringList m_receiveSignalList;
    QStringList m_infoList;
    QStringList m_faultList;
    QList<int> m_infoIdLst;
    QList<int> m_faultIdLst;

    int m_variableSize;

    Telegram* m_telegram;
    WidgetMap m_widgetMap;

    QGridLayout* m_infoLayout;
    QGridLayout* m_faultLayout;

    QStringList m_autoTestSignalList;
    QMap<QString , int > m_signalIndexMap;

    TrainNumber m_currentTrainNumber;
};

#endif

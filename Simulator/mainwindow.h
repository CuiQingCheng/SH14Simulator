#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Signal/SignalValue.h"
#include "Telegram/Telegram.h"
#include "Gui/CheckBox.h"
#include "Gui/TextEdit.h"
#include "todvobccomm/TodVobcChannel.h"
#include "Core/Parser.h"
#include "Core/Factory/Factory.h"
#include "Core/Handler/WidgetHandler.h"

#include <QtXml>
#include <QDomDocument>
#include <QFile>

#include <QRegExpValidator>
#include <QRegExp>

class QTimer;

namespace Ui {
class MainWindow;
}

typedef enum sendType
{
    Auto = 0,
    Manual =1
}SendTelegramType;

typedef enum Optional_Frame_Id
{
    Fault_Frame_Id = 1,
    Info_Frame_Id = 2
}Optional_Frame_Id;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static const int FIX_REC_DATASIZE = 10;
    static const int REC_TCMS_DATASIZE = 208;

signals:
    void sendTelegramUpdated();

protected:
    bool openConfigurationFile();
    void parseConfigurationFile();
    void sendTableWidgetValue();
    void setInputFrameEnable(bool enable);


private slots:
    void on_action_Open_triggered();

    void on_setBtn_clicked();

    void sendRTFTimeout();

    void on_connectBtn_clicked();

    void checkConnectState();

    void sendPoolData();

    void receiveData();

    void checkConnectAndSendPoolData();

private:
    void initDefaultConfig();
    void modifyDefaultConfig();
    void setConfig();

private:
    Ui::MainWindow *ui;
    QString m_configFileName;

    SignalMap m_SendSignalMap;
    SignalMap m_receiveSignalMap;
    SignalMap m_telegramHeaderMap;

    TodVobcChannel *m_todChannel;

    int m_sendTelegramType;
    int m_sendCycle;

    QTimer* m_sendRFCTimer;
    QTimer* m_checkConnectState;
    QTimer* m_sendPoolDataTimer;
    QTimer* m_receiveDataTimer;


    QSettings *m_theSettingsPtr;
    Parser* m_parserPtr;
    Factory* m_factory;
    WidgetHandler* m_widgetHandler;

};
#endif // MAINWINDOW_H

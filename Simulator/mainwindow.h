#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Signal/SignalValue.h"
#include "Telegram/Telegram.h"
#include "Gui/CheckBox.h"
#include "Gui/TextEdit.h"
#include "todvobccomm/TodVobcChannel.h"
#include <QtXml>
#include <QDomDocument>
#include <QFile>

#include <QGridLayout>
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

typedef struct Tcms_Item{
    static const int ROW_COUNT = 6;
    static const int COLUMN_COUNT = 16;
    static const int ITEM_W = 70;
    static const int ITEM_H = 28;
}tcmsItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static const int RADIO_W = 225;
    static const int RADIO_H = 22;
    static const int LINECOUNT = 5;
    static const int FIX_REC_DATASIZE = 10;
    static const int REC_TCMS_DATASIZE = 208;

protected:
    bool openConfigurationFile();
    bool parseConfigurationFile();
    void drawTableWidget();
    void drawInfoFaultCheckTab();
    void drawTcmsTableWidget();
    void sendTableWidgetValue();
    void setInputFrameEnable(bool enable);
    void getTcmsValueLst(QStringList& lst);

private slots:
    void on_action_Open_triggered();

    void on_setBtn_clicked();

    void sendRTFTimeout();

    void on_connectBtn_clicked();

    void checkConnectState();

    void sendPoolData();

    void receiveData();

    void updateInfoFaultId(int state);

private:
    void clear();
    void initDefaultConfig();
    void modifyDefaultConfig();
    void setConfig();

private:
    Ui::MainWindow *ui;
    QString m_configFileName;

    SignalMap m_SendSignalMap;
    SignalMap m_receiveSignalMap;
    SignalMap m_telegramHeaderMap;

    QList<QString> m_SendSignalList;
    QList<QString> m_receiveSignalList;

    QList<QString> m_infoList;
    QList<QString> m_faultList;

    QList<CheckBox*> m_infoCheckBoxList;
    QList<CheckBox*> m_faultCheckBoxList;

    Telegram *m_telegram;
    TodVobcChannel *m_todChannel;

    int m_sendTelegramType;
    int m_sendCycle;

    QTimer* m_sendRFCTimer;
    QTimer* m_checkConnectState;
    QTimer* m_sendPoolDataTimer;
    QTimer* m_receiveDataTimer;
    QGridLayout* m_infoLayout;
    QGridLayout* m_faultLayout;
    QList<int> m_infoIdLst;
    QList<int> m_faultIdLst;

    int m_variableSize;

    QSettings *m_theSettingsPtr;
};
#endif // MAINWINDOW_H

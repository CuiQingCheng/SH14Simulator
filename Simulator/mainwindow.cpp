#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "math.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_isExecAutoTest(false)
{
    ui->setupUi(this);
    m_todChannel = new TodVobcChannel();
    m_sendRFCTimer = new QTimer(this);
    m_checkConnectState =new QTimer(this);
    m_sendPoolDataTimer = new QTimer(this);
    m_receiveDataTimer = new QTimer(this);

    connect(m_sendRFCTimer, SIGNAL(timeout()), this, SLOT(sendRTFTimeout()));
    connect(m_checkConnectState, SIGNAL(timeout()), this, SLOT(checkConnectState()));
    connect(m_sendPoolDataTimer, SIGNAL(timeout()), this, SLOT(sendPoolData()));
    connect(m_receiveDataTimer, SIGNAL(timeout()), this, SLOT(receiveData()));

    m_factory = new Factory;
    m_parserPtr = new Parser;

    m_widgetHandler = m_factory->get<WidgetHandler>("WidgetHandler");

    m_widgetHandler->addWidget(QString("sendTableWidget"), ui->sendTableWidget);
    m_widgetHandler->addWidget(QString("infoTab"), ui->infoTab);
    m_widgetHandler->addWidget(QString("faultTab"), ui->faultTab);
    m_widgetHandler->addWidget(QString("sendTcmsTableWidget"), ui->tcmsTableWidget);
    m_widgetHandler->addWidget(QString("receiveTableWidget"), ui->receiveTableWidget);
    m_widgetHandler->addWidget(QString("recTCMSData_TextEdit"), ui->recTCMSData_TextEdit);

    m_autoTestHandler = m_factory->get<AutoTestHandler>("AutoTestHandler");
    connect(m_parserPtr, SIGNAL(parseFinished()), m_widgetHandler, SLOT(showGuiDefData()));
    connect(m_autoTestHandler, SIGNAL(signalValueUpdated(QString, QString)), m_widgetHandler, SLOT(updateSignalValue(QString,QString)), Qt::QueuedConnection);
    connect(m_widgetHandler, SIGNAL(sendTelegramUpdated()), this, SLOT(checkConnectAndSendPoolData()));

    initDefaultConfig();

    this->setWindowTitle(QString("VOBC Simulator"));
    ui->autotestBtn->setDisabled(true);
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow()";
    delete ui;


    if(m_todChannel != NULL)
    {
        delete m_todChannel;
        m_todChannel = NULL;
    }

    if(m_factory != NULL)
    {
        delete m_factory;
        m_factory = NULL;
    }

    if(m_parserPtr != NULL)
    {
        delete m_parserPtr;
        m_parserPtr = NULL;
    }
}

void MainWindow::on_setBtn_clicked()
{
    setConfig();

    modifyDefaultConfig();
}

void MainWindow::setConfig()
{
    int cycle = ui->sendCycleLineEdit->text().toInt();
    if(cycle >= 3000)
    {
        QMessageBox::warning(this, "Warning", "cycle time should be less than 3000.", QMessageBox::Ok);
        return;
    }
    m_todChannel->setTodPort(ui->todPortLineEdit->text().toInt());
    m_todChannel->setTodAdress(ui->todIpLineEdit->text());
    m_todChannel->setTodId(ui->todIdLineEdit->text().toInt());
    m_todChannel->setVobcPort(ui->vobcPortLineEdit->text().toInt());

    if(ui->autoRadioBtn->isChecked())
    {
        m_sendTelegramType = SendTelegramType::Auto;
    }
    else
        m_sendTelegramType = SendTelegramType::Manual;
    m_sendCycle = cycle;
}

void MainWindow::on_connectBtn_clicked()
{
    if(ui->connectBtn->text() == "Connect")
    {
        m_todChannel->open();
        m_todChannel->sendDataTelegramForRFC();
        m_sendRFCTimer->start(2000);
        m_checkConnectState->start(3000);

        ui->connectBtn->setText("End");
        setInputFrameEnable(false);
    }
    else if(ui->connectBtn->text() == "End")
    {
        m_todChannel->close();

        m_sendRFCTimer->stop();
        m_checkConnectState->stop();
        m_sendPoolDataTimer->stop();
        m_receiveDataTimer->stop();
        m_autoTestHandler->stop();
        ui->connectBtn->setText("Connect");
        ui->tipsLabel->setText("End.");
        ui->tipsLabel->setStyleSheet("color:green; font:bold");
        ui->autotestBtn->setDisabled(true);
        setInputFrameEnable(true);
    }
}

void MainWindow::sendRTFTimeout()
{
    if(m_todChannel->getComConnectState())
    {
        m_sendRFCTimer->stop();

        ui->tipsLabel->setText("Connect Success.");
        ui->tipsLabel->setStyleSheet("color:green; font:bold");
        ui->autotestBtn->setEnabled(true);

        if(!m_sendPoolDataTimer->isActive())
        {
            m_sendPoolDataTimer->start(m_sendCycle);
        }
        if(!m_receiveDataTimer->isActive())
        {
            m_receiveDataTimer->start(m_sendCycle);
        }
    }
    else
    {
        m_todChannel->sendDataTelegramForRFC();
    }
}

void MainWindow::checkConnectState()
{
    if(!m_todChannel->getComConnectState())
    {
        if(!m_sendRFCTimer->isActive())
            m_sendRFCTimer->start(2000);

        ui->tipsLabel->setText("Disconnect !!!      Trying to reconnect ...");
        ui->tipsLabel->setStyleSheet("color:red; font:bold");
        ui->autotestBtn->setDisabled(true);
    }
}

void MainWindow::sendPoolData()
{
    sendTableWidgetValue();
    int offset = 0;
    QStringList lst;

    int optionalTag = 0;

    QList<int> infoIdLst = m_widgetHandler->getInfoFaultIdLst(true);
    QList<int> faultIdLst = m_widgetHandler->getInfoFaultIdLst(false);

    if(infoIdLst.size() > 0)
    {
        m_todChannel->setDataByInfoFault(infoIdLst, 0, Info_Frame_Id);
        offset = (infoIdLst.size() + 2);
        ++optionalTag;
    }
    if(faultIdLst.size() > 0)
    {
        m_todChannel->setDataByInfoFault(faultIdLst, offset, Fault_Frame_Id);
        offset += (faultIdLst.size() + 2);
        ++optionalTag;
    }

    if(optionalTag)
    {
        m_todChannel->setOptionTag(optionalTag);
    }
    m_widgetHandler->getTcmsValueLst(lst);

    m_todChannel->setDataByTCMS(lst, offset);

    m_todChannel->sendDataTelegram();
}

void MainWindow::receiveData()
{
    QByteArray receiveAppData = m_todChannel->receiveData();
    if(receiveAppData == NULL || receiveAppData.isNull() || (0 == receiveAppData.size()))
    {
        qDebug() << "Error: the receiving polling telegram is NULL!";
        return;
    }

    quint8 rowCount = ui->receiveTableWidget->rowCount();

    // the size of receiveAppData is looked as the all the lenth of the bytes.
    // start to fill the table by every line.
    QString valueStr = "";
    quint8 valueNum = 0;
    quint16 valueTwoByteNum = 0;
    quint16 valueFourByteNum = 0;
    quint8 valueBoolNum = 0;
    quint8 valueNum1 = 0;
    quint8 valueNum2 = 0;
    quint8 valueNum3 = 0;
    quint8 valueNum4 = 0;

    for(int i=0; i < rowCount; i++)
    {
        QString filedNameStr = ui->receiveTableWidget->item(i, 0)->text();
        quint8 byteOffsetNum = ui->receiveTableWidget->item(i, 1)->text().toInt();
        quint8 bitOffsetNum = ui->receiveTableWidget->item(i, 2)->text().toInt();
        QString typeStr = ui->receiveTableWidget->item(i, 3)->text();
        QString NumTypeStr = ui->receiveTableWidget->item(i, 4)->text();

        if(filedNameStr.contains("Reserved", Qt::CaseInsensitive) || filedNameStr.contains("Not Used", Qt::CaseInsensitive))        {
            continue;
        }

        if(typeStr == "UNSIGNED.8")
        {
            valueNum = (quint8)(receiveAppData.at(byteOffsetNum));
            if(NumTypeStr == "hexadecimal")
            {
                valueStr = QString::number(valueNum, 16);
            }
            else if(NumTypeStr == "Dec")
            {
                valueStr = QString::number(valueNum, 10);
            }
        }
        else if(typeStr == "BOOLEAN.1")
        {
            valueNum = (quint8)(receiveAppData.at(byteOffsetNum));
            valueBoolNum = (valueNum & (0x01 << bitOffsetNum));
            valueStr = QString::number(valueBoolNum, 10);
        }
        else if(typeStr == "UNSIGNED.16")
        {
            valueNum1 = (quint8)receiveAppData.at(byteOffsetNum);
            valueNum2 = (quint8)receiveAppData.at(byteOffsetNum + 1);

            if(NumTypeStr == "hexadecimal")
            {
                valueTwoByteNum = (quint16)(valueNum1 | (valueNum2 << 8));
                valueStr = QString::number(valueTwoByteNum, 16);
            }
            else if(NumTypeStr == "Dec")
            {
                valueTwoByteNum = (quint16)(valueNum1 + valueNum2*256);
                valueStr = QString::number(valueTwoByteNum, 10);
            }
        }
        else if(typeStr == "UNSIGNED.32")
        {
            valueNum1 = (quint8)receiveAppData.at(byteOffsetNum);
            valueNum2 = (quint8)receiveAppData.at(byteOffsetNum + 1);
            valueNum3 = (quint8)receiveAppData.at(byteOffsetNum + 2);
            valueNum4 = (quint8)receiveAppData.at(byteOffsetNum + 3);

            if(NumTypeStr == "hexadecimal")
            {
                valueFourByteNum = (quint16)(valueNum1 | (valueNum2 << 8) | (valueNum3 << 16) | (valueNum4 << 24));
                valueStr = QString::number(valueFourByteNum, 16);
            }
            else if(NumTypeStr == "Dec")
            {
                valueFourByteNum = (quint32)(valueNum1 + valueNum2*(pow(2, 8)) + valueNum3*(pow(2, 16)) + valueNum4*(pow(2, 24)));
                valueStr = QString::number(valueFourByteNum, 10);
            }
        }

        QTableWidgetItem *lastValueItem = ui->receiveTableWidget->item(i, 5);
        lastValueItem->setText(valueStr);
    }

    QByteArray tmcsData = receiveAppData.mid(FIX_REC_DATASIZE, REC_TCMS_DATASIZE);

    TextEdit* textEdit= ui->recTCMSData_TextEdit;
    textEdit->setShowData(tmcsData);

    m_checkConnectState->start(3000);
}

void MainWindow::checkConnectAndSendPoolData()
{
    if(m_todChannel->getComConnectState())
    {
        sendPoolData();
    }
}


void MainWindow::initDefaultConfig()
{
    QRegExp rx("^((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)$");

    ui->todIpLineEdit->setValidator(new QRegExpValidator(rx, this));
    ui->todIpLineEdit->setPlaceholderText("000.000.000.000");
    m_theSettingsPtr = new QSettings("../Simulator.conf", QSettings::IniFormat, this);
    QString ipStr = m_theSettingsPtr->value("IP", QString("192.168.30.110")).toString();
    ui->todIpLineEdit->setText(ipStr);

    QString idstr = m_theSettingsPtr->value("ID", 0).toString();
    ui->todIdLineEdit->setText(idstr);
    ui->todIdLineEdit->setValidator(new QIntValidator(0, 65535, this));

    QString portStr = m_theSettingsPtr->value("PORT", 0).toString();
    ui->todPortLineEdit->setText(portStr);
    ui->todPortLineEdit->setValidator(new QIntValidator(0, 65535, this));

    QString vobcPortStr = m_theSettingsPtr->value("VOBC_PORT", 0).toString();
    ui->vobcPortLineEdit->setText(vobcPortStr);
    ui->vobcPortLineEdit->setValidator(new QIntValidator(0, 65535, this));

    QString cycleStr = m_theSettingsPtr->value("Send_CYCLE", 0).toString();
    ui->sendCycleLineEdit->setText(cycleStr);
    ui->sendCycleLineEdit->setValidator(new QIntValidator(0, 3000, this));
    ui->sendCycleLineEdit->setPlaceholderText("< 3000");

    m_todChannel->setTodPort(portStr.toInt());
    m_todChannel->setTodAdress(ipStr);
    m_todChannel->setTodId(idstr.toInt());
    m_todChannel->setVobcPort(vobcPortStr.toInt());

    if(ui->autoRadioBtn->isChecked())
    {
        m_sendTelegramType = SendTelegramType::Auto;
    }
    else
        m_sendTelegramType = SendTelegramType::Manual;

    m_sendCycle = cycleStr.toInt();

    m_configFileName = m_theSettingsPtr->value("SOURCE", "").toString();

    if(!m_configFileName.isEmpty())
    {
        parseConfigurationFile();
    }
}

void MainWindow::modifyDefaultConfig()
{
    m_theSettingsPtr->setValue("IP", ui->todIpLineEdit->text());
    m_theSettingsPtr->setValue("PORT", ui->todPortLineEdit->text());
    m_theSettingsPtr->setValue("ID", ui->todIdLineEdit->text());
    m_theSettingsPtr->setValue("VOBC_PORT", ui->vobcPortLineEdit->text());
    m_theSettingsPtr->setValue("Send_CYCLE", ui->sendCycleLineEdit->text());
}


void MainWindow::on_action_Open_triggered()
{
    if (openConfigurationFile())
    {
        parseConfigurationFile();
    }
}

bool MainWindow::openConfigurationFile()
{
    QString configFileName = QFileDialog::getOpenFileName(this,tr("open file"),NULL,tr("xmlFile(*.xml)"));

    if(configFileName.isEmpty())
    {
        qDebug() << "Config file is not exist !!!";
        return false;
    }

    m_theSettingsPtr->setValue("SOURCE", configFileName);
    m_configFileName = configFileName;

    return true;
}

void MainWindow::parseConfigurationFile()
{
    m_widgetHandler->clear();
    m_autoTestHandler->clear();
    (m_factory->get<Telegram>("Telegram"))->clear();
    m_widgetHandler->setTelegram( (m_factory->get<Telegram>("Telegram")));
    m_parserPtr->setConfigFile(m_configFileName);
    int ret = m_parserPtr->parse(m_factory);

    if(ret != 0)
    {
        qDebug("Failed to parse configuration file, application abort.");
        exit(1);
    }

    QStringList Path = m_configFileName.split("/");
    QString fileName = Path.at(Path.size() - 1);
    this->setStatusTip(fileName);
}

void MainWindow::sendTableWidgetValue()
{
    QString sendSignalName;
    QString sendDataByteOffset;
    QString sendDataBitOffset;
    QString sendDataType;
    QString sendDataNumType;
    QString sendDataValue;

    QStringList speedNameLst = {"ActualSpeed", "TargetSpeed", "PermittedSpeed", "EbTriggerSpeed"};

    // clear the the_pucData before send at next time.
    m_todChannel->clear();

    for(int i = 0;i < ui->sendTableWidget->rowCount();++i)
    {
        QTableWidgetItem *itemInputPanel = ui->sendTableWidget->item(i, 3);
        if((itemInputPanel->flags() & Qt::ItemIsEnabled))
        {
            sendSignalName = ui->sendTableWidget->item(i, 0)->text();
            sendDataByteOffset = ui->sendTableWidget->item(i, 1)->text();
            sendDataBitOffset = ui->sendTableWidget->item(i, 2)->text();
            sendDataType = ui->sendTableWidget->item(i, 3)->text();
            sendDataNumType = ui->sendTableWidget->item(i, 4)->text();
            sendDataValue = ui->sendTableWidget->item(i, 5)->text();

            QStringList strLst = sendDataType.split(".");
            if(strLst.size() != 2)
            {
                qDebug() << sendSignalName << " send failed !!! " << sendDataType << " should split by '.'";
                continue;
            }

            if( speedNameLst.contains(sendSignalName) )
            {
                int value = sendDataValue.toInt();

                value = qRound (value * pow(10.0, 6) / 3600);

                sendDataValue = QString::number(value);
            }

            m_todChannel->setDataByByteSize(sendDataByteOffset, sendDataBitOffset, sendDataType, sendDataNumType, sendDataValue);
        }
    }
}

void MainWindow::setInputFrameEnable(bool enable)
{
    ui->todIpLineEdit->setEnabled(enable);
    ui->todPortLineEdit->setEnabled(enable);
    ui->todIdLineEdit->setEnabled(enable);
    ui->vobcPortLineEdit->setEnabled(enable);
    ui->sendCycleLineEdit->setEnabled(enable);
    ui->setBtn->setEnabled(enable);
}


void MainWindow::on_autotestBtn_clicked()
{
    static bool firstclicked = true;

    if(firstclicked)
    {
        m_autoTestHandler->initDefSignalValue();
        firstclicked = true;
    }

    if(m_isExecAutoTest)
    {
        m_autoTestHandler->stop();
        m_isExecAutoTest = false;
    }
    else
    {
        m_autoTestHandler->start();
        m_isExecAutoTest = true;
    }
}

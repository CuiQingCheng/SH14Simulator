#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "math.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>

const int SHL14_FIXOFFSET = 79;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_isExecAutoTest(false),
    m_serialStatus(false),
    m_baudRate(0)
{
    ui->setupUi(this);
    m_todChannel = new TodCommChannel();
    m_sendRFCTimer = new QTimer(this);
    m_checkConnectState =new QTimer(this);
    m_sendPoolDataTimer = new QTimer(this);
    m_receiveDataTimer = new QTimer(this);
    m_choseComnMode = Udp;
    m_currentComnMode = Empty;
    connect(m_sendRFCTimer, SIGNAL(timeout()), this, SLOT(sendRTFTimeout()));
    connect(m_checkConnectState, SIGNAL(timeout()), this, SLOT(checkConnectState()));
    connect(m_sendPoolDataTimer, SIGNAL(timeout()), this, SLOT(sendPoolData()));
    connect(m_receiveDataTimer, SIGNAL(timeout()), this, SLOT(receiveData()));

    this->setWindowTitle(QString("Communication Simulator"));
    m_factory = new Factory;
    m_parserPtr = new Parser;

    m_widgetHandler = m_factory->get<WidgetHandler>("WidgetHandler");

    m_widgetHandler->addWidget(QString("sendTableWidget"), ui->sendTableWidget);
    m_widgetHandler->addWidget(QString("infoTab"), ui->infoTab);
    m_widgetHandler->addWidget(QString("faultTab"), ui->faultTab);
    m_widgetHandler->addWidget(QString("sendTcmsTableWidget"), ui->tcmsTableWidget);
    m_widgetHandler->addWidget(QString("receiveTableWidget"), ui->receiveTableWidget);
    m_widgetHandler->addWidget(QString("recTCMSData_TextEdit"), ui->recTCMSData_TextEdit);
    m_widgetHandler->addWidget(QString("checkBoxWidget"), ui->tabWidget_2);
    m_widgetHandler->addWidget(QString("sendTcmsWidget"), ui->sendTcmsWidget);
    m_widgetHandler->addWidget(QString("recTcmsWidget"),ui->receiveTcmsWidget);

    m_autoTestHandler = m_factory->get<AutoTestHandler>("AutoTestHandler");
    connect(m_parserPtr, SIGNAL(parseFinished()), m_widgetHandler, SLOT(showGuiDefData()));
    connect(m_autoTestHandler, SIGNAL(signalValueUpdated(QString, QString)), m_widgetHandler, SLOT(updateSignalValue(QString,QString)), Qt::QueuedConnection);
    connect(m_todChannel, SIGNAL(updateRecvWidgetData()),this, SLOT(receiveData()));
    connect(m_widgetHandler, SIGNAL(sendTelegramUpdated()), this, SLOT(checkConnectAndSendPoolData()));
    connect(ui->openRadioButton, SIGNAL(clicked(bool)),this, SLOT(openSerialCom(bool)));

    initDefaultConfig();
    initSerialPort();

    ui->autotestBtn->setDisabled(true);
}

MainWindow::~MainWindow()
{
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

    m_sendCycle = ui->sendCycleLineEdit->text().toInt();
}

void MainWindow::setConfig()
{
    int cycle = ui->sendCycleLineEdit->text().toInt();
    if(cycle >= 4000)
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
    {
        m_sendTelegramType = SendTelegramType::Manual;
    }

    m_sendCycle = cycle;
}

void MainWindow::initSerialPort()
{
    ui->comComboBox->clear();
    QStringList list;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        list << info.portName();
    }
    qSort(list.begin(), list.end());
    ui->comComboBox->addItems(list);
}

void MainWindow::on_connectBtn_clicked()
{

    m_sendCycle = ui->sendCycleLineEdit->text().toInt();
    if(ui->connectBtn->text() == "Connect")
    {
        ui->connectBtn->setText("End");
        m_sendRFCTimer->start(2000);
        m_checkConnectState->start(3000);

        if(m_choseComnMode == Udp && m_currentComnMode == Empty)
        {
            m_todChannel->openUdpCom();

            m_currentComnMode = Udp;
            ui->connectType->setStyleSheet("color:green; background-color: none;");
            ui->connectType->setText("Current communication is UDP!");
            m_todChannel->sendDataTelegramForRFC("Udp");
        }
        else if(m_choseComnMode == Serial && m_currentComnMode == Empty)
        {
            if(!m_todChannel->isSerialPortOpen())
            {
                return;
            }
            m_currentComnMode = Serial;
            ui->connectType->setStyleSheet("color:green; background-color: none;");
            ui->connectType->setText("Current communication is Serial!");
            m_todChannel->sendDataTelegramForRFC("Serial");
        }

        setInputFrameEnable(false);
    }
    else if(ui->connectBtn->text() == "End")
    {
        m_sendRFCTimer->stop();
        m_checkConnectState->stop();
        m_sendPoolDataTimer->stop();
        m_receiveDataTimer->stop();
        m_autoTestHandler->stop();

        if(m_currentComnMode == Udp)
        {
            m_todChannel->closeUdpCom();
        }

        ui->connectType->setText("");
        ui->connectBtn->setText("Connect");
        ui->tipsLabel->setText("End.");
        ui->tipsLabel->setStyleSheet("color:green; font:bold");
        ui->autotestBtn->setDisabled(true);
        setInputFrameEnable(true);
        m_currentComnMode = Empty;
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
            qDebug() << "send Cycle:"<< m_sendCycle;
            m_sendPoolDataTimer->start(m_sendCycle);
        }
        if(m_currentComnMode == Udp)
        {
            if(!m_receiveDataTimer->isActive())
            {
                m_receiveDataTimer->start(m_sendCycle);
            }
        }
    }
    else
    {
        if(m_currentComnMode == Udp)
        {
            m_todChannel->sendDataTelegramForRFC("Udp");
        }
        else if(m_currentComnMode == Serial)
        {
            m_todChannel->sendDataTelegramForRFC("Serial");
        }

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

    int optionalTag = 0;
    bool checkWidgethidden = m_widgetHandler->getWidgetMap()->value("checkBoxWidget")->isHidden();

    if(!checkWidgethidden)
    {
        QList<int> infoIdLst = m_widgetHandler->getInfoFaultIdLst(true);
        QList<int> faultIdLst = m_widgetHandler->getInfoFaultIdLst(false);

        if(infoIdLst.size() > 0)
        {
            m_todChannel->setDataByInfoFault(infoIdLst, SHL14_FIXOFFSET, Info_Frame_Id);
            offset = (infoIdLst.size() + 2);
            ++optionalTag;
        }
        if(faultIdLst.size() > 0)
        {
            m_todChannel->setDataByInfoFault(faultIdLst, SHL14_FIXOFFSET + offset, Fault_Frame_Id);
            offset += (faultIdLst.size() + 2);
            ++optionalTag;
        }
    }

    bool sendTcmsWidgethidden = m_widgetHandler->getWidgetMap()->value("checkBoxWidget")->isHidden();
    if(!sendTcmsWidgethidden)
    {
        QStringList lst;
        if(optionalTag)
        {
            m_todChannel->setOptionTag(SHL14_FIXOFFSET - 1 , optionalTag);
        }
        m_widgetHandler->getTcmsValueLst(lst);

        m_todChannel->setDataByTCMS(lst, SHL14_FIXOFFSET + offset);
    }

    if(m_currentComnMode == Udp)
    {
        m_todChannel->sendDataTelegram("Udp");
    }
    else if(m_currentComnMode == Serial)
    {
        m_todChannel->sendDataTelegram("Serial");
    }

}

void MainWindow::receiveData()
{

    QByteArray receiveAppData = m_todChannel->receiveData();
    int recvFixedSize = m_parserPtr->getFixSignalSize("Response");

    if(receiveAppData == NULL || (0 == receiveAppData.size()) )
    {
        qDebug() << "Error: the receiving polling telegram is NULL!";
        return;
    }

    if(receiveAppData.size() + 5 < recvFixedSize)
    {
        qDebug() << "AppData size < right Size";
        return;
    }

    quint8 rowCount = ui->receiveTableWidget->rowCount();

    // the size of receiveAppData is looked as the all the lenth of the bytes.
    // start to fill the table by every line.
    QString valueStr = "";
    int valueNum = 0;
    quint16 valueTwoByteNum = 0;
    quint16 valueFourByteNum = 0;
    quint8 valueBoolNum = 0;
    quint8 valueNum1 = 0;
    quint8 valueNum2 = 0;
    quint8 valueNum3 = 0;
    quint8 valueNum4 = 0;
    quint8 fixRecSize = 0;
    for(int i=0; i < rowCount; i++)
    {
        QString filedNameStr = ui->receiveTableWidget->item(i, 0)->text();
        quint8 byteOffsetNum = ui->receiveTableWidget->item(i, 1)->text().toInt();
        quint8 bitOffsetNum = ui->receiveTableWidget->item(i, 2)->text().toInt();
        QString typeStr = ui->receiveTableWidget->item(i, 3)->text();
        QString NumTypeStr = ui->receiveTableWidget->item(i, 4)->text();
        if(i + 1 == rowCount)
        {
            fixRecSize = byteOffsetNum + 1;
            if(typeStr == "UNSIGNED.16")
            {
                fixRecSize += 1;
            }

            else if(typeStr == "UNSIGNED.32")
            {
                fixRecSize += 3;
            }
        }

        if(filedNameStr.contains("Reserved", Qt::CaseInsensitive) || filedNameStr.contains("Not Used", Qt::CaseInsensitive))
        {
            continue;
        }

        if(typeStr == "UNSIGNED.8")
        {
            valueNum = (quint8)(receiveAppData.at(byteOffsetNum));
            if(NumTypeStr == "Hex")
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

            if(NumTypeStr == "Hex")
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

            if(NumTypeStr == "Hex")
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
    QStringList tcmsPortLst = m_widgetHandler->getTcmsPort("recvTcms");
    if(tcmsPortLst.size() > 0)
    {
        QByteArray tmcsData;

        TextEdit* textEdit= ui->recTCMSData_TextEdit;

        int tcmsDataSize = 0;
        for(int i = 0; i < tcmsPortLst.size(); ++i)
        {
            QStringList strList =  tcmsPortLst.at(i).split(":");
            tcmsDataSize += strList[1].toInt();
        }
        tmcsData = receiveAppData.mid(fixRecSize, tcmsDataSize);
        textEdit->setShowData(tmcsData, tcmsPortLst);
    }

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
    ui->sendCycleLineEdit->setValidator(new QIntValidator(0, 4000, this));
    ui->sendCycleLineEdit->setPlaceholderText("< 4000");

    QString baud_Rate_Str = m_theSettingsPtr->value("Baud_RATE", 0).toString();
    ui->rateLineEdit->setText(baud_Rate_Str);

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


void MainWindow::on_action_open_triggered()
{
    if (openConfigurationFile())
    {
        parseConfigurationFile();
    }
}

bool MainWindow::openConfigurationFile()
{
    QString configFileName = QFileDialog::getOpenFileName(this,tr("open file"),NULL,tr("xmlFile(*.xml)"));

    if(m_todChannel->getComConnectState())
    {
        QMessageBox::warning(this, "Warning", "The simulator is connecting, disconnect it and replace the script file", QMessageBox::Ok);
        return false;
    }

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
    m_widgetHandler->setTelegram((m_factory->get<Telegram>("Telegram")));
    m_parserPtr->setConfigFile(m_configFileName);
    int ret = m_parserPtr->parse(m_factory);

    QString deviceName = m_parserPtr->getDeviceName();
    QString winTitle = deviceName + QString(" Simulator");
    this->setWindowTitle(winTitle);

    int defFixedPollSignalSize = m_parserPtr->getFixSignalSize("poll");
    int defFixedRecvSignalSize = m_parserPtr->getFixSignalSize("Response");
    m_todChannel->setDefPollDataSize(defFixedPollSignalSize);
    m_todChannel->setDefRecvDataSize(defFixedRecvSignalSize);
    QString protocolType = m_parserPtr->getProtocolType();

    if(protocolType == QString("udpSocket"))
    {
        ui->stackedWidget->setCurrentIndex(Udp);
        m_choseComnMode = Udp;
    }
    else if(protocolType == QString("serial"))
    {
        ui->stackedWidget->setCurrentIndex(Serial);
        m_choseComnMode = Serial;
    }
    m_todChannel->setjudgeInterval(protocolType);

    if(ret != 0)
    {
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
    ui->sendBtn->setEnabled(enable);
}


void MainWindow::on_autotestBtn_clicked()
{
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

void MainWindow::on_action_udp_triggered()
{
    if(Udp != ui->stackedWidget->currentIndex())
    {
        ui->stackedWidget->setCurrentIndex(Udp);
        m_choseComnMode = Udp;
        m_todChannel->setjudgeInterval("udp");
    }
}

void MainWindow::on_action_Serial_triggered()
{
    if(Serial != ui->stackedWidget->currentIndex())
    {
        ui->stackedWidget->setCurrentIndex(Serial);
        m_choseComnMode = Serial;
        m_todChannel->setjudgeInterval("serial");
    }
}


void MainWindow::on_action_about_triggered()
{
    QMessageBox::information(this, "About", "The emulator supports UDP and Serial communication!", QMessageBox::Ok);
}

void MainWindow::openSerialCom(bool isOpen)
{
    m_comName = ui->comComboBox->currentText();
    m_baudRate = ui->rateLineEdit->text().toInt();
    m_theSettingsPtr->setValue("Baud_RATE", m_baudRate);
    m_todChannel->setSerialPortName(m_comName);
    if(isOpen)
    {
        if(m_todChannel->openSerialPort(m_baudRate))
        {
            m_serialStatus = true;
            ui->comStatusLabel->setStyleSheet("color:green; background-color: none;");
            ui->comStatusLabel->setText("Open success!");
        }
        else
        {
            m_serialStatus = false;
            ui->comStatusLabel->setStyleSheet("color:red; background-color:none;");
            QMessageBox::warning(NULL, QString(tr("error")), QString(tr("open faild!")));
            ui->comStatusLabel->setText("Open faild!");
        }
    }
    else
    {
        if(m_todChannel->isSerialPortOpen())
        {
            m_todChannel->closeSerialPort();
        }
        m_serialStatus = false;
        ui->comStatusLabel->setText("");
    }
}

void MainWindow::on_sendBtn_clicked()
{
    if(!m_todChannel->isSerialPortOpen())
    {
        return;
    }
    m_currentComnMode = m_choseComnMode;
    sendPoolData();
    m_currentComnMode = Empty;
}



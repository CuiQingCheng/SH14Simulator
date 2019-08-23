#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_variableSize(0)
{
    ui->setupUi(this);
    m_todChannel = new TodVobcChannel();
    m_telegram = new Telegram();
    m_sendRFCTimer = new QTimer(this);

    initDefaultConfig();

    connect(m_sendRFCTimer, SIGNAL(timeout()), this, SLOT(sendRTFTimeout()));

    m_checkConnectState =new QTimer(this);
    connect(m_checkConnectState, SIGNAL(timeout()), this, SLOT(checkConnectState()));

    m_sendPoolDataTimer = new QTimer(this);
    connect(m_sendPoolDataTimer, SIGNAL(timeout()), this, SLOT(sendPoolData()));

    m_receiveDataTimer = new QTimer(this);
    connect(m_receiveDataTimer, SIGNAL(timeout()), this, SLOT(receiveData()));

    m_infoLayout = new QGridLayout(ui->infoTab);
    m_faultLayout = new QGridLayout(ui->faultTab);
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow()";
    delete ui;
    if(m_telegram != NULL)
    {
        delete m_telegram;
        m_telegram = NULL;
    }
    if(m_todChannel != NULL)
    {
        delete m_todChannel;
        m_todChannel = NULL;
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

        ui->connectBtn->setText("Connect");
        ui->tipsLabel->setText("End.");
        ui->tipsLabel->setStyleSheet("color:green; font:bold");
        setInputFrameEnable(true);
    }
}

void MainWindow::sendRTFTimeout()
{
//    qDebug() << "sendRTFTimeout() -> getComConnectState() : "<< m_todChannel->getComConnectState();
    if(m_todChannel->getComConnectState())
    {
        m_sendRFCTimer->stop();

        ui->tipsLabel->setText("Connect Success.");
        ui->tipsLabel->setStyleSheet("color:green; font:bold");

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
    }
}

void MainWindow::sendPoolData()
{
    sendTableWidgetValue();
    int offset = 0;
    QStringList lst;

    int optionalTag = 0;

    if(m_infoIdLst.size() > 0)
    {
        m_todChannel->setDataByInfoFault(m_infoIdLst, 0, Info_Frame_Id);
        offset = (m_infoIdLst.size() + 2);
        ++optionalTag;
    }
    if(m_faultIdLst.size() > 0)
    {
        m_todChannel->setDataByInfoFault(m_faultIdLst, offset, Fault_Frame_Id);
        offset += (m_faultIdLst.size() + 2);
        ++optionalTag;
    }

    if(optionalTag)
    {
        m_todChannel->setOptionTag(optionalTag);
    }
    getTcmsValueLst(lst);

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
    qDebug() << "receive app data size:"<< receiveAppData.size();
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

//    qDebug() << "receive tmcs size:" <<tmcsData.size();
    TextEdit* textEdit= ui->recTCMSData_TextEdit;
    textEdit->setShowData(tmcsData);

    m_checkConnectState->start(3000);
}

void MainWindow::updateInfoFaultId(int state)
{
    CheckBox* checkbox = qobject_cast<CheckBox *>(QObject::sender());
    int id = checkbox->getId();
    int type = checkbox->getCheckBoxType();

    if(state == Qt::Checked)
    {
        if(m_variableSize + 1 > TextEdit::MAX_SEND_VARIABLE_SIZE)
        {
            checkbox->setCheck(false);
            return;
        }
        else
        {
            checkbox->setCheck(true);
            ++m_variableSize;
        }
        if(0 == type)
        {
            if(!m_infoIdLst.contains(id))
            {
                m_infoIdLst.append(id);
            }
        }
        else if(1 == type)
        {
            if(!m_faultIdLst.contains(id))
            {
                m_faultIdLst.append(id);
            }
        }
    }
    else if(state == Qt::Unchecked)
    {
        if(0 == type)
        {
            if(m_infoIdLst.contains(id))
            {
                m_infoIdLst.removeOne(id);
            }
        }
        else if(1 == type)
        {
            if(m_faultIdLst.contains(id))
            {
                m_faultIdLst.removeOne(id);
            }
        }
        checkbox->setChecked(false);
        --m_variableSize;
    }

}

void MainWindow::clear()
{
    if(m_SendSignalList.size() > 0)
    {
        m_SendSignalList.clear();
    }

    if(m_receiveSignalList.size() > 0)
    {
        m_receiveSignalList.clear();
    }

    if(m_infoList.size() > 0)
    {
        m_infoList.clear();
    }

    if(m_faultList.size() > 0)
    {
        m_faultList.clear();
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
        if(parseConfigurationFile())
        {
            drawTableWidget();

            drawInfoFaultRadioTab();

            drawTcmsTableWidget();
        }
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

    qDebug() << "fileName = " << configFileName;
    m_configFileName = configFileName;

    return true;
}

bool MainWindow::parseConfigurationFile()
{
    QFile file(m_configFileName);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("Can't open configuration file : %s", m_configFileName.toLatin1().data());
        return false;
    }

    QDomDocument doc;

    QString error;
    int row = 0, column = 0;

    if(!doc.setContent(&file, false, &error, &row, &column))
    {
        qDebug("Configuration error.");
        qDebug("xml error : %s; row : %d, column : %d.", error.toLatin1().data(), row, column);
        file.close();
        return false;
    }
    clear();
    m_telegram->clear();
    QDomElement root = doc.documentElement();
    QDomElement node = root.firstChildElement();

    QString fieldName;
    QString dataType;
    quint16 byteOffset;
    quint8 bitOffset;
    quint32 defaultValue;
    QString valueType;
    QString element;
    QString elementText;
    QString elementId;

    while(!node.isNull())
    {
        if(node.tagName() == "communication" && node.hasAttribute("processType") && node.hasAttribute("sendType"))
        {
            QString sendType = node.attributeNode("sendType").value();
            QDomElement objNode = node.firstChildElement();

            while(!objNode.isNull())
            {
                if(objNode.hasAttribute("fieldName"))
                    fieldName = objNode.attributeNode("fieldName").value();
                if(objNode.hasAttribute("type"))
                    dataType = objNode.attributeNode("type").value();
                if(objNode.hasAttribute("byteOffset"))
                    byteOffset = objNode.attributeNode("byteOffset").value().toUInt();
                if(objNode.hasAttribute("bitOffset"))
                    bitOffset = objNode.attributeNode("bitOffset").value().toUInt();
                if(objNode.hasAttribute("defaultValue"))
                    defaultValue = objNode.attributeNode("defaultValue").value().toUInt();
                if(objNode.hasAttribute("valueType"))
                {
                    valueType = objNode.attributeNode("valueType").value();
                }
                else
                {
                    valueType = "hexadecimal";
                }

                SignalValue* signalValue = new SignalValue(fieldName, dataType, byteOffset, bitOffset, defaultValue, valueType);

                if(sendType == "vobc-tod")
                {
                    m_telegram->setSendSignalMap(signalValue);
                    if(!m_SendSignalList.contains(signalValue->getName()))
                    {
                        m_SendSignalList.append(signalValue->getName());
                    }
                }
                if(sendType == "tod-vobc")
                {
                    m_telegram->setReceiveSignalMap(signalValue);
                    if(!m_receiveSignalList.contains(signalValue->getName()))
                    {
                        m_receiveSignalList.append(signalValue->getName());
                    }
                }

                objNode = objNode.nextSiblingElement();
            }
        }
        if(node.tagName() == "checkboxList"&& node.hasAttribute("elemtype") && node.hasAttribute("id"))
        {
            QString id = node.attributeNode("id").value();
            QDomElement objNode = node.firstChildElement();

            while(!objNode.isNull())
            {
                if(objNode.hasAttribute("name"))
                   elementText = objNode.attributeNode("name").value();
                if(objNode.hasAttribute("id"))
                   elementId = objNode.attributeNode("id").value();


                element = elementText + ":" + elementId;
                if(id == "InfoModule")
                {
                    m_infoList.append(element);
                }

                if(id == "FaultModule")
                {
                    m_faultList.append(element);
                }
                objNode = objNode.nextSiblingElement();
            }
        }
        node = node.nextSiblingElement();
    }
    return true;
}

void MainWindow::drawTableWidget()
{
    /*** Initialize SendDataPage TableWidget***/
    QList<QString> rowHeaderList;
    rowHeaderList.append("FieldName");
    rowHeaderList.append("ByteOffset");
    rowHeaderList.append("BitOffset");
    rowHeaderList.append("DataType");
    rowHeaderList.append("ValueType");
    rowHeaderList.append("Value");

    // the FiledName's num plus 1 is equal to the all of line num
    quint16 sendDataRowCount = m_SendSignalList.count();
    quint8 sendDataColumnCount = rowHeaderList.count();

    ui->sendTableWidget->setRowCount(sendDataRowCount);
    ui->sendTableWidget->setColumnCount(sendDataColumnCount);
    ui->sendTableWidget->setColumnWidth(0,300);
    ui->sendTableWidget->setColumnWidth(3,150);
    ui->sendTableWidget->setColumnWidth(4,150);

    ui->sendTableWidget->setHorizontalHeaderLabels(rowHeaderList);

    for (int i = 0; i < ui->sendTableWidget->rowCount(); ++i)
    {
        for (int j = 0; j < ui->sendTableWidget->columnCount(); ++j)
        {
            QTableWidgetItem* newItem = new QTableWidgetItem("");
            ui->sendTableWidget->setItem(i,j,newItem);

            ui->sendTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);

            if ((j == (sendDataColumnCount-1)))
            {
                ui->sendTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            }
            else
            {
                ui->sendTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled);
                ui->sendTableWidget->item(i,j)->setBackgroundColor(Qt::gray);
            }
            // SignalValue* signal = m_SendSignalMap.value(m_SendSignalList[i]);
            SignalValue* signal = m_telegram->getSendSignalValue(m_SendSignalList[i]);
            if(NULL == signal)
            {
                qDebug() << "get Send Signal Value by key: " << m_SendSignalList[i] << " failed !!!";
                return;
            }
            if (j == 0)    // FieldName
            {
                ui->sendTableWidget->item(i,j)->setText(signal->getName());
            }
            else if (j == 1)    // ByteOffset
            {
                ui->sendTableWidget->item(i,j)->setText(QString::number(signal->getByteOffset()));
            }
            else if (j == 2)    // BitOffset
            {
                ui->sendTableWidget->item(i,j)->setText(QString::number(signal->getBitOffset()));
            }
            else if (j == 3)    // DataType
            {
                ui->sendTableWidget->item(i,j)->setText(signal->getDataType());
            }
            else if (j == 4)    // ValueType
            {
                ui->sendTableWidget->item(i,j)->setText(signal->getValueType());
            }
        }
    }

    for (int i = 0; i < ui->sendTableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *itemFirstColumn = ui->sendTableWidget->item(i,0);
        QString FirstColumnStr =  itemFirstColumn->text();
        if( FirstColumnStr.contains("Reserved") || FirstColumnStr.contains("Not Used"))
        {
            for(int j = 0;j < ui->sendTableWidget->columnCount(); ++j)
            {
                QTableWidgetItem *itemNeedDisable = ui->sendTableWidget->item(i,j);
                itemNeedDisable->setFlags(itemNeedDisable->flags() & (~Qt::ItemIsEnabled));
                itemNeedDisable->setBackgroundColor(Qt::gray);
            }
        }
    }

    ui->sendTableWidget->resizeRowsToContents();             // adjust to many lines to show when a line can't show all the content.

    /*** Initialize ReceivedDataPage TableWidget***/
    quint16 receivedDataRowCount = m_receiveSignalList.count();
    quint8 receivedDataColumnCount = rowHeaderList.count();

    ui->receiveTableWidget->setRowCount(receivedDataRowCount);
    ui->receiveTableWidget->setColumnCount(receivedDataColumnCount);
    ui->receiveTableWidget->setColumnWidth(0,300);
    ui->receiveTableWidget->setColumnWidth(3,150);
    ui->receiveTableWidget->setColumnWidth(4,150);
    ui->receiveTableWidget->setHorizontalHeaderLabels(rowHeaderList);

    for (int i = 0; i < ui->receiveTableWidget->rowCount(); ++i)
    {
        for (int j = 0; j < ui->receiveTableWidget->columnCount(); ++j)
        {
            QTableWidgetItem* newItem = new QTableWidgetItem("");
            ui->receiveTableWidget->setItem(i,j,newItem);

            ui->receiveTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);

            if ((j == (receivedDataColumnCount-1)))
            {
                ui->receiveTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable );
            }
            else
            {
                ui->receiveTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled);
                ui->receiveTableWidget->item(i,j)->setBackgroundColor(Qt::gray);
            }

            // SignalValue* signal = m_receiveSignalMap.value(m_receiveSignalList[i]);
            SignalValue* signal = m_telegram->getReceiveSignalValue(m_receiveSignalList[i]);
            if(NULL == signal)
            {
                qDebug() << "get Receive Signal Value by key: " << m_receiveSignalList[i] << " failed !!!";
                return;
            }
            if (j == 0)    // FieldName
            {
                ui->receiveTableWidget->item(i,j)->setText(signal->getName());
            }
            else if (j == 1)    // ByteOffset
            {
                ui->receiveTableWidget->item(i,j)->setText(QString::number(signal->getByteOffset()));
            }
            else if (j == 2)    // BitOffset
            {
                ui->receiveTableWidget->item(i,j)->setText(QString::number(signal->getBitOffset()));
            }
            else if (j == 3)    // DataType
            {
                ui->receiveTableWidget->item(i,j)->setText(signal->getDataType());
            }
            else if (j == 4)    // ValueType
            {
                ui->receiveTableWidget->item(i,j)->setText(signal->getValueType());
            }
        }
    }

    for (int i = 0; i < ui->receiveTableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *itemFirstColumn = ui->receiveTableWidget->item(i,0);
        QString FirstColumnStr =  itemFirstColumn->text();
        if( FirstColumnStr.contains("Reserved") || FirstColumnStr.contains("Not Used"))
        {
            for(int j = 0;j < ui->receiveTableWidget->columnCount(); ++j)
            {
                QTableWidgetItem *itemNeedDisable = ui->receiveTableWidget->item(i,j);
                itemNeedDisable->setFlags(itemNeedDisable->flags() & (~Qt::ItemIsEnabled));
                itemNeedDisable->setBackgroundColor(Qt::gray);
            }
        }
    }

    ui->receiveTableWidget->resizeRowsToContents();
}

void MainWindow::drawInfoFaultRadioTab()
{
    int infoCount = m_infoList.size();
    int infoBoxCount = m_infoCheckBoxList.size();

    int faultCount = m_faultList.size();
    int faultBoxCount = m_faultCheckBoxList.size();
    int addBox = 0;
    if(infoCount > infoBoxCount)
    {
        addBox = infoCount - infoBoxCount;
    }
    if(faultCount > faultBoxCount)
    {
        addBox += (faultCount - faultBoxCount);
    }

    for(int i = 0; i < addBox; ++i)
    {
        CheckBox* checkbox = new CheckBox(RADIO_W, RADIO_H);
        QObject::connect(checkbox, SIGNAL(stateChanged(int)), this, SLOT(updateInfoFaultId(int)));
        if(i < (infoCount - infoBoxCount))
        {
            checkbox->setCheckBoxType(CheckBox::boxType::INFO);
            m_infoCheckBoxList.append(checkbox);
        }

        if((faultCount > faultBoxCount) && (i >= (infoCount - infoBoxCount)))
        {
            checkbox->setCheckBoxType(CheckBox::boxType::FAULT);
            m_faultCheckBoxList.append(checkbox);
        }
    }

    if(infoCount < infoBoxCount)
    {
        for(int i = 0; i< infoBoxCount - infoCount; ++i)
        {
            CheckBox* checkbox = m_infoCheckBoxList[infoBoxCount - (i+1)];
            m_infoCheckBoxList.removeLast();
            delete checkbox;
            checkbox = NULL;
        }
    }

    if(faultCount < faultBoxCount)
    {
        for(int i = 0; i < faultBoxCount - faultCount; ++i)
        {
            CheckBox* checkbox = m_faultCheckBoxList[faultBoxCount - (i+1)];
            m_faultCheckBoxList.removeLast();
            delete checkbox;
            checkbox = NULL;
        }
    }


    for(int i = 0; i < m_infoList.size(); ++i)
    {
        QStringList info = m_infoList[i].split(":");
        if(info.size() != 2)
            return;

        m_infoCheckBoxList[i]->setId((info[1]).toInt());
        m_infoCheckBoxList[i]->setLabelText(info[0]);

        m_infoLayout->addWidget(m_infoCheckBoxList[i], i / LINECOUNT, i % LINECOUNT, 1, 1);
    }
    m_infoLayout->setHorizontalSpacing(5);
    m_infoLayout->setVerticalSpacing(2);
    m_infoLayout->setContentsMargins(10, 10, 10, 10);
    ui->infoTab->setLayout(m_infoLayout);

    for(int i = 0; i < m_faultList.size(); i++)
    {
        QStringList falut = m_faultList[i].split(":");
        if(falut.size() != 2)
            return;

        m_faultCheckBoxList[i]->setId((falut[1]).toInt());
        m_faultCheckBoxList[i]->setLabelText(falut[0]);

        m_faultLayout->addWidget(m_faultCheckBoxList[i], i / LINECOUNT, i % LINECOUNT, 1, 1);
    }
    m_faultLayout->setHorizontalSpacing(5);
    m_faultLayout->setVerticalSpacing(2);
    m_faultLayout->setContentsMargins(10, 10, 10, 10);
    ui->faultTab->setLayout(m_faultLayout);
}

void MainWindow::drawTcmsTableWidget()
{
    int itemWidth = ui->tcmsTableWidget->width() / 16;
    int itemHeight = ui->tcmsTableWidget->height() / 6;
    ui->tcmsTableWidget->setColumnCount(16);
    ui->tcmsTableWidget->setRowCount(6);
    for(int i = 0; i < 16; i++)
    {
        ui->tcmsTableWidget->setColumnWidth(i, itemWidth);
    }
    for (int i = 0; i < ui->tcmsTableWidget->rowCount(); ++i)
    {
        ui->tcmsTableWidget->setRowHeight(i, itemHeight);

        for (int j = 0; j < ui->tcmsTableWidget->columnCount(); ++j)
        {
            QTableWidgetItem* newItem = new QTableWidgetItem("0");
            ui->tcmsTableWidget->setItem(i,j,newItem);

            ui->tcmsTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);

            ui->tcmsTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        }
    }
}

void MainWindow::sendTableWidgetValue()
{
    QString sendSignalName;
    QString sendDataByteOffset;
    QString sendDataBitOffset;
    QString sendDataType;
    QString sendDataNumType;
    QString sendDataValue;

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

void MainWindow::getTcmsValueLst(QStringList& lst)
{
    QString itemValue;

    for(int i = 0; i < ui->tcmsTableWidget->rowCount(); ++i)
    {
        for(int j = 0; j < ui->tcmsTableWidget->columnCount(); ++j)
        {
            itemValue = ui->tcmsTableWidget->item(i, j)->text();

            lst << (itemValue);
        }
    }
}

#include "WidgetHandler.h"
#include "../../Gui/TextEdit.h"

#include <QDebug>

WidgetHandler::WidgetHandler()
    : m_variableSize(0)
{
    m_infoLayout  = new QGridLayout();
    m_faultLayout = new QGridLayout();

    m_autoTestSignalList << "StationSkipStatus" << "StationHoldStatus" << "StationDwell" << "NextPlatformDoorSide"
                         << "DistanceToStoppingPoint" << "TrainDoorsEnableLeftStatus" << "TrainDoorsEnableRightStatus"
                         << "LeftTrainDoorsClosedStatus" << "RightTrainDoorsClosedStatus" << "LeftDoorCloseCommandStatus"
                         << "RightDoorCloseCommandStatus" << "LeftPSDClosedAndLockedStatus" << "RightPSDClosedAndLockedStatus"
                         << "NextPlatformID" << "TerminusPlatformID" <<"ActualSpeed" << "TargetSpeed" << "PermittedSpeed"
                         << "EbTriggerSpeed" << "DepartureStatus" << "DockingStatus" << "CurrentOperatingMode" << "NumberOfCars"
                         << "FAMModeAvailability" << "ATOModeAvaliability" << "ATPMModeAvaliability" << "RunTypeIndication"
                         << "NumberOfAvailableVOBCs" << "ActiveCab" << "MotorBrakingStatus" << "TrainDoorsClosedAndLockedStatus"
                         << "TrainDoorsEnableLeftStatus" << "PlatformDoorEnableStatus" << "PlatformDoorOpenStatus";
}

WidgetHandler::~WidgetHandler()
{

}

void WidgetHandler::addEleNameList(QString name, int type)
{
    if(Send_Signal == type)
    {
        if(!m_sendSignalList.contains(name))
        {
            m_sendSignalList.append(name);
        }
    }
    else if(Receiver_Signal == type)
    {
        if(!m_receiveSignalList.contains(name))
        {
            m_receiveSignalList.append(name);
        }
    }
    else if(Info == type)
    {
        if(!m_infoList.contains(name))
        {
            m_infoList.append(name);
        }
    }
    else if(Fault == type)
    {
        if(!m_faultList.contains(name))
        {
            m_faultList.append(name);
        }
    }

}

void WidgetHandler::addWidget(QString key, QWidget* widget)
{

    if(m_widgetMap.contains(key))
    {
        return ;
    }

    if( widget != NULL )
    {
        m_widgetMap.insert(key, widget);
    }
}

WidgetMap* WidgetHandler::getWidgetMap()
{
    return &m_widgetMap;
}

void WidgetHandler::setTelegram(Telegram* telegram)
{
    m_telegram = telegram;
}

void WidgetHandler::clear()
{
    WidgetMap::Iterator iter = m_widgetMap.begin();

    while(iter != m_widgetMap.end())
    {
       if((iter.key()).contains("TableWidget"))
       {
           QTableWidget* tableWidget = static_cast<QTableWidget*> (iter.value());
           tableWidget->clear();
       }

       if((iter.key()).contains("TextEdit"))
       {
            TextEdit* textEdit = static_cast<TextEdit*> (iter.value());
            textEdit->clear();
       }

       ++iter;
    }

    m_sendSignalList.clear();
    m_receiveSignalList.clear();
    m_infoIdLst.clear();
    m_faultIdLst.clear();
}

QList<int> WidgetHandler::getInfoFaultIdLst(bool isInfo)
{
    if(isInfo)
    {
        return m_infoIdLst;
    }
    return m_faultIdLst;
}

void WidgetHandler::getTcmsValueLst(QStringList &lst)
{
    QString itemValue;
    QTableWidget* tcmsTableWidget = static_cast<QTableWidget*> (m_widgetMap["sendTcmsTableWidget"]);

    for(int i = 0; i < tcmsTableWidget->rowCount(); ++i)
    {
        for(int j = 0; j < tcmsTableWidget->columnCount(); ++j)
        {
            itemValue = tcmsTableWidget->item(i, j)->text();

            lst << (itemValue);
        }
    }
}

void WidgetHandler::showGuiDefData()
{
    if(m_widgetMap.isEmpty())
    {
        qDebug()<<"widget map is empty";
        exit(1);
    }
    drawTableWidget();
    drawInfoFaultCheckTab();
    drawTcmsTableWidget();
}

void WidgetHandler::updateInfoFaultId(int state)
{
    CheckBox* checkbox = qobject_cast<CheckBox *>(QObject::sender());
    int id = checkbox->getId();
    int type = checkbox->getCheckBoxType();

    if(state == Qt::Checked)
    {
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
        checkbox->setCheck(true);
        ++m_variableSize;
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

void WidgetHandler::updateSignalValue(QString signal, QString value)
{
//    qDebug( "[WidgetHandler::updateSignalValue] signal: %s value: %s", signal, value);

    if(m_autoTestSignalList.contains(signal))
    {
        QTableWidget* sendTableWidget = static_cast<QTableWidget*> (m_widgetMap["sendTableWidget"]);
        int row = m_signalIndexMap.value(signal);
        sendTableWidget->item(row, 5)->setText(value);
        emit sendTelegramUpdated();
    }
}

void WidgetHandler::drawTableWidget()
{
    QTableWidget* sendTableWidget = static_cast<QTableWidget*> (m_widgetMap["sendTableWidget"]);
    QTableWidget* receiverTableWidget = static_cast<QTableWidget*> (m_widgetMap["receiveTableWidget"]);

    if(sendTableWidget == NULL)
    {
        qDebug() << "SendTableWidget is NULL";
        return;
    }

    if(receiverTableWidget == NULL)
    {
        qDebug() << "receiverTableWidget is NULL";
        return;
    }

    QList<QString> rowHeaderList;
    rowHeaderList.append("FieldName");
    rowHeaderList.append("ByteOffset");
    rowHeaderList.append("BitOffset");
    rowHeaderList.append("DataType");
    rowHeaderList.append("ValueType");
    rowHeaderList.append("Value");

    // the FiledName's num plus 1 is equal to the all of line num
    quint16 sendDataRowCount = m_sendSignalList.count();
    quint8 sendDataColumnCount = rowHeaderList.count();

    sendTableWidget->setRowCount(sendDataRowCount);
    sendTableWidget->setColumnCount(sendDataColumnCount);
    sendTableWidget->setColumnWidth(0,300);
    sendTableWidget->setColumnWidth(3,150);
    sendTableWidget->setColumnWidth(4,150);

    sendTableWidget->setHorizontalHeaderLabels(rowHeaderList);

    for (int i = 0; i < sendTableWidget->rowCount(); ++i)
    {
        for (int j = 0; j < sendTableWidget->columnCount(); ++j)
        {
            QTableWidgetItem* newItem = new QTableWidgetItem("");
            sendTableWidget->setItem(i,j,newItem);
            sendTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);

            if ((j == (sendDataColumnCount-1)))
            {
                sendTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            }
            else
            {
                sendTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled);
                sendTableWidget->item(i,j)->setBackgroundColor(Qt::gray);
            }

            SignalValue* signal = m_telegram->getSendSignalValue(m_sendSignalList[i]);
            if(NULL == signal)
            {
                qDebug() << "get Send Signal Value by key: " << m_sendSignalList[i] << " failed !!!";
                return;
            }

            if (j == 0)    // FieldName
            {
                if(m_autoTestSignalList.contains(signal->getName()))
                {
                    if(!m_signalIndexMap.contains(signal->getName()))
                    {
                        m_signalIndexMap.insert(signal->getName(), i);
                    }
                }
                sendTableWidget->item(i,j)->setText(signal->getName());
            }
            else if (j == 1)    // ByteOffset
            {
                sendTableWidget->item(i,j)->setText(QString::number(signal->getByteOffset()));
            }
            else if (j == 2)    // BitOffset
            {
                sendTableWidget->item(i,j)->setText(QString::number(signal->getBitOffset()));
            }
            else if (j == 3)    // DataType
            {
                sendTableWidget->item(i,j)->setText(signal->getDataType());
            }
            else if (j == 4)    // ValueType
            {
                sendTableWidget->item(i,j)->setText(signal->getValueType());
            }
            else if (j == 5)    // Value
            {
                sendTableWidget->item(i,j)->setText(QString::number(signal->getValue()));
            }
        }
    }

    for (int i = 0; i < sendTableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *itemFirstColumn = sendTableWidget->item(i,0);
        QString FirstColumnStr =  itemFirstColumn->text();
        if( FirstColumnStr.contains("Reserved") || FirstColumnStr.contains("Not Used"))
        {
            for(int j = 0;j < sendTableWidget->columnCount(); ++j)
            {
                QTableWidgetItem *itemNeedDisable = sendTableWidget->item(i,j);
                itemNeedDisable->setFlags(itemNeedDisable->flags() & (~Qt::ItemIsEnabled));
                itemNeedDisable->setBackgroundColor(Qt::gray);
            }
        }
    }

    sendTableWidget->resizeRowsToContents();             // adjust to many lines to show when a line can't show all the content.

    quint16 receivedDataRowCount = m_receiveSignalList.count();
    quint8 receivedDataColumnCount = rowHeaderList.count();

    receiverTableWidget->setRowCount(receivedDataRowCount);
    receiverTableWidget->setColumnCount(receivedDataColumnCount);
    receiverTableWidget->setColumnWidth(0,300);
    receiverTableWidget->setColumnWidth(3,150);
    receiverTableWidget->setColumnWidth(4,150);
    receiverTableWidget->setHorizontalHeaderLabels(rowHeaderList);

    for (int i = 0; i < receiverTableWidget->rowCount(); ++i)
    {
        for (int j = 0; j < receiverTableWidget->columnCount(); ++j)
        {
            QTableWidgetItem* newItem = new QTableWidgetItem("");
            receiverTableWidget->setItem(i,j,newItem);

            receiverTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);

            if ((j == (receivedDataColumnCount-1)))
            {
                receiverTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable );
            }
            else
            {
                receiverTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled);
                receiverTableWidget->item(i,j)->setBackgroundColor(Qt::gray);
            }

            SignalValue* signal = m_telegram->getReceiveSignalValue(m_receiveSignalList[i]);
            if(NULL == signal)
            {
                qDebug() << "get Receive Signal Value by key: " << m_receiveSignalList[i] << " failed !!!";
                return;
            }
            if (j == 0)    // FieldName
            {
                receiverTableWidget->item(i,j)->setText(signal->getName());
            }
            else if (j == 1)    // ByteOffset
            {
                receiverTableWidget->item(i,j)->setText(QString::number(signal->getByteOffset()));
            }
            else if (j == 2)    // BitOffset
            {
                receiverTableWidget->item(i,j)->setText(QString::number(signal->getBitOffset()));
            }
            else if (j == 3)    // DataType
            {
                receiverTableWidget->item(i,j)->setText(signal->getDataType());
            }
            else if (j == 4)    // ValueType
            {
                receiverTableWidget->item(i,j)->setText(signal->getValueType());
            }
            else if (j == 5)    // Value
            {
                receiverTableWidget->item(i,j)->setText(QString::number(signal->getValue()));
            }
        }
    }

    for (int i = 0; i < receiverTableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *itemFirstColumn = receiverTableWidget->item(i,0);
        QString FirstColumnStr =  itemFirstColumn->text();
        if( FirstColumnStr.contains("Reserved") || FirstColumnStr.contains("Not Used"))
        {
            for(int j = 0;j < receiverTableWidget->columnCount(); ++j)
            {
                QTableWidgetItem *itemNeedDisable = receiverTableWidget->item(i,j);
                itemNeedDisable->setFlags(itemNeedDisable->flags() & (~Qt::ItemIsEnabled));
                itemNeedDisable->setBackgroundColor(Qt::gray);
            }
        }
    }

    receiverTableWidget->resizeRowsToContents();
}

void WidgetHandler::drawInfoFaultCheckTab()
{
    QWidget* infoWidget = m_widgetMap["infoTab"];
    QWidget* faultWidget = m_widgetMap["faultTab"];

    if(infoWidget == NULL)
    {
        qDebug() << "infoWidget is NULL";
        return;
    }


    if(faultWidget == NULL)
    {
        qDebug() << "faultWidget is NULL";
        return;
    }

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
        m_infoLayout ->addWidget(m_infoCheckBoxList[i], i / LINECOUNT, i % LINECOUNT, 1, 1);
    }

    m_infoLayout ->setHorizontalSpacing(5);
    m_infoLayout ->setVerticalSpacing(2);
    m_infoLayout ->setContentsMargins(10, 10, 10, 10);
    (m_widgetMap["infoTab"])->setLayout(m_infoLayout );

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
     (m_widgetMap["faultTab"])->setLayout(m_faultLayout);
}

void WidgetHandler::drawTcmsTableWidget()
{
    QTableWidget* tcmsTableWidget = static_cast<QTableWidget*> (m_widgetMap["sendTcmsTableWidget"]);
    tcmsTableWidget->setColumnCount(tcmsItem::COLUMN_COUNT);
    tcmsTableWidget->setRowCount(tcmsItem::ROW_COUNT);

    for(int i = 0; i < 16; i++)
    {
        tcmsTableWidget->setColumnWidth(i, tcmsItem::ITEM_W);
    }
    for (int i = 0; i < tcmsTableWidget->rowCount(); ++i)
    {
        tcmsTableWidget->setRowHeight(i, tcmsItem::ITEM_H);

        for (int j = 0; j < tcmsTableWidget->columnCount(); ++j)
        {
            QTableWidgetItem* newItem = new QTableWidgetItem("0");
            tcmsTableWidget->setItem(i,j,newItem);

            tcmsTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);

            tcmsTableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        }
    }
}




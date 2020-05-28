#include "TextEdit.h"
#include <QDebug>

TextEdit::TextEdit(QWidget *parent):QTextEdit(parent)
  , m_showStr("")
  , m_editCount(0)
{
}

TextEdit::~TextEdit()
{

}

void TextEdit::setShowData(QByteArray &input,QStringList& tcmsPortLst)
{
    if(input.size() > MAX_REC_TCMS_SIZE)
    {
        qDebug() << "receive data is too large";
        return;
    }
    quint8 valueNum = 0;
    QString valueStr = "";
    QString tempStr = "";
    quint8 portNum = tcmsPortLst.size();
    QString str = tcmsPortLst.at(0);
    quint8 linecount = str.toInt();

    for(int i = 0, j = 0; (j < portNum)&&(i < input.size()); i++)
    {
        valueNum = (quint8)(input.at(i));
        valueStr = QString::number(valueNum, 16);
        tempStr += valueStr;
        if(i + 1 < input.size())
        {
            if((i + 1) == linecount)
            {
                tempStr += ";";
                tempStr += "\n";
                j++;
                QString str = tcmsPortLst.at(0);
                linecount += str.toInt();
            }
            else
                tempStr += ",";
        }
        else
        {
            tempStr += ";";
            tempStr += "\n";
        }
    }


    if(0 != tempStr.compare(m_showStr))
    {
        m_showStr = tempStr;
        this->setText(m_showStr);
    }

}

QString TextEdit::getShowData()
{
    return m_showStr;
}

void TextEdit::setEditCount(int count)
{
    m_editCount = count;
}

int TextEdit::getEditCount()
{
    return m_editCount;
}


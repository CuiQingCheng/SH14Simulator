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

void TextEdit::setShowData(QByteArray &input)
{
    if(input.size() > MAX_REC_TCMS_SIZE)
    {
        qDebug() << "receive data is too large";
        return;
    }
    quint8 valueNum = 0;
    QString valueStr = "";
    QString tempStr = "";
    for(int i = 0; i < input.size(); i++)
    {
        valueNum = (quint8)(input.at(i));
        valueStr = QString::number(valueNum, 16);
        tempStr += valueStr;
        if(i + 1 < input.size())
        {
            if((i - 15) % 32 == 0)
            {
                tempStr += ";";
                tempStr += "\n";
            }
            else
                tempStr += ",";
        }
        else
        {
            tempStr += ";";
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


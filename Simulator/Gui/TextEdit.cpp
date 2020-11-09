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
    quint8 valueNum = 0;
    QString valueStr = "";
    quint8 portNum = tcmsPortLst.size();
    QStringList lst0 = tcmsPortLst.at(0).split(":");
    int costlineNum = (lst0.at(1)).toInt();
    QString tempStr = lst0.at(0) + "\n";
    int linecount = costlineNum;
    int portcount = costlineNum;
    int i =0, j = 0;
    for(; (j < portNum)&&(i < input.size()); i++)
    {
        valueNum = (quint8)(input.at(i));
        valueStr = QString::number(valueNum, 16);//Hex
        tempStr += valueStr;
        if(i + 1 < input.size())
        {
            if((i + 1) == linecount)
            {
                tempStr += ";";
                tempStr += "\n";
                linecount += costlineNum;
                if(i + 1 == portcount)
                {
                    tempStr += "\n";
                    ++j;
                    QStringList lstJ = tcmsPortLst.at(j).split(":");
                    tempStr += lstJ.at(0);
                    tempStr += "\n";
                    portcount += lstJ.at(1).toInt();
                }
            }
            else
            {
                tempStr += ",";
            }

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


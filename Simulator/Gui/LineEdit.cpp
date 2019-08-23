#include "LineEdit.h"

LineEdit::LineEdit(QWidget *parent):QLineEdit(parent)
{
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(showTextChanged(QString)));
}

LineEdit::~LineEdit()
{

}

QString LineEdit::showTextChanged(QString text)
{
    if(m_text.compare(text) != 0)
    {
        m_text = text;
    }
}

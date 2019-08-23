#include "CheckBox.h"
#include <QDebug>

CheckBox::CheckBox(int width, int height):
    m_width(width)
  , m_height(height)
  , m_type(boxType::INFO)
{
    this->setFixedWidth(m_width);
    this->setFixedHeight(m_height);

    setStyleSheet(QString("font-size : 12px"));

//    QObject::connect(this, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
}

CheckBox::~CheckBox()
{

}

void CheckBox::setId(int id)
{
    m_id = id;
}

int CheckBox::getId()
{
    return m_id;
}

void CheckBox::setCheck(bool check)
{
    this->setChecked(check);
//    emit checkStateUpdate(check);
}

bool CheckBox::getIsChecked()
{
    return this->isChecked();
}

void CheckBox::setCheckBoxType(int type)
{
    m_type = type;
}

int CheckBox::getCheckBoxType()
{
    return m_type;
}

void CheckBox::setLabelText(QString text)
{
    m_text = text;
    this->setText(text);
    this->setToolTip(text);
}

QString CheckBox::getLabelText()
{
    return m_text;
}

//void CheckBox::onStateChanged(int status)
//{
//     qDebug() << "onStateChanged";
//    if(status == Qt::Unchecked)
//    {
//        setCheck(false);
//    }
//    else if (status == Qt::Checked)
//    {
//        setCheck(true);
//    }
//}


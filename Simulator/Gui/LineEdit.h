#ifndef LINEEDIT_H
#define LINEEDIT_H

#include<QLineEdit>
#include<QString>

class LineEdit:public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget *parent=0);
    ~LineEdit();

public slots:
    QString showTextChanged(QString text);

private:
    QString m_text;
};

#endif

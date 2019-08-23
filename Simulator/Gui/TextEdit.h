#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include<QTextEdit>
#include<QByteArray>
#include<QString>

class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    TextEdit(QWidget *parent=0);
    ~TextEdit();
    static const int MAX_REC_TCMS_SIZE = 502;
    static const int MAX_SEND_VARIABLE_SIZE = 178;

    void setShowData(QByteArray& input);
    QString getShowData();

    void setEditCount(int count);
    int getEditCount();

private:
    QString m_showStr;
    int     m_editCount;
};

#endif

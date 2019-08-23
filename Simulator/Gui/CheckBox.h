#ifndef CHECKBOX_H
#define CHECKBOX_H

#include<QCheckBox>
#include<QString>

class CheckBox:public QCheckBox
{
    Q_OBJECT
public:
    CheckBox(int width, int height);
    ~CheckBox();
    enum boxType{
        INFO = 0,
        FAULT
    };
    void setId(int id);
    int getId();

    void setCheck(bool check);
    bool getIsChecked();

    void setCheckBoxType(int type);
    int getCheckBoxType();

    void setLabelText(QString text);
    QString getLabelText();

signals:
//    void checkStateUpdate(bool state);

public slots:
//    void onStateChanged(int status);
private:
    QString m_text;
    int m_width;
    int m_height; 
    int m_id;
    int m_type;
};

#endif

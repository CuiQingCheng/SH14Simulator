#ifndef AUTOTESTHANDLER_H
#define AUTOTESTHANDLER_H

#include<QObject>

class AutoTestHandler:public QObject
{
    Q_OBJECT
public:
    AutoTestHandler();
    ~AutoTestHandler();
};

#endif

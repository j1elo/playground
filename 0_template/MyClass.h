#ifndef MYCLASS_H
#define MYCLASS_H

#include <QWidget>

class MyClass : public QWidget
{
    Q_OBJECT
    
public:
    explicit MyClass(QWidget* parent = 0);
    ~MyClass();
};

#endif // MYCLASS_H

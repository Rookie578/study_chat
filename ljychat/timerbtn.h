#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
public:
    TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();

    void mouseReleaseEvent(QMouseEvent *e) override; //override 重写（覆盖）基类虚函数标记

private:
    //计时器
    QTimer *_timer;
    //计数
    int _counter;
};

#endif // TIMERBTN_H

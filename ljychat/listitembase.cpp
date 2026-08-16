#include "listitembase.h"

#include <QStyleOption>
#include <QPainter>

ListItemBase::ListItemBase(QWidget *parent):QWidget(parent)
{

}

void ListItemBase::SetItemType(ListItemType itemType)
{
    _itemType = itemType;
}

ListItemType ListItemBase::GetItemType()
{
    return _itemType;
}

//重写paintEvent，让ListItemBase这个控件能够正确响应样式表（QSS）的设置，显示背景、边框等基本外观。
void ListItemBase::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

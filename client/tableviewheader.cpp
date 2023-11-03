#include "tableviewheader.h"
#include <QPainter>
#include <QCheckBox>

TableViewHeader::TableViewHeader(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    //设置Section可点击，若不设置则不能发出sectionClicked信号
    this->setSectionsClickable(true);
    //当发出sectionClicked就进入匿名函数
    connect(this, &TableViewHeader::sectionClicked, [=](int logicalIndex)
            {
                //判断map容器是否包含当前点击列，包含则更新并发出columnSectionClicked
                if(m_columpnCheckedMap.contains(logicalIndex))
                {
                    //更新当前值
                    m_columpnCheckedMap[logicalIndex] = !m_columpnCheckedMap[logicalIndex];
                    //发出信号
                    emit columnSectionClicked(logicalIndex, m_columpnCheckedMap[logicalIndex]);
                }
            });

}

void TableViewHeader::setColumnCheckable(int column, bool checkable)
{
    //当可选值为true
    if(checkable)
    {
        //将指定列添加到map容器中
        m_columpnCheckedMap[column] = false;
    }
    else if(m_columpnCheckedMap.contains(column))   //当可选值为false，且map容器包含指定列
    {
        //移除指定列
        m_columpnCheckedMap.remove(column);
    }
}

void TableViewHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();    //保存当前画笔状态
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore(); //恢复保存的画笔状态

    //当map容器包含当前列才绘制复选框
    if(m_columpnCheckedMap.contains(logicalIndex))
    {
        //创建样式对象并设置区域
        QStyleOptionButton styleOption;
        styleOption.rect = rect.adjusted(3, 0, 0, 0);
        styleOption.state = QStyle::State_Enabled;

        //根据map中的值设置状态
        if(m_columpnCheckedMap[logicalIndex])
        {
            styleOption.state |= QStyle::State_On;
        }
        else
        {
            styleOption.state |= QStyle::State_Off;
        }

        //调用this的style对象绘制复选框
        this->style()->drawControl(QStyle::CE_CheckBox, &styleOption, painter);
    }
}

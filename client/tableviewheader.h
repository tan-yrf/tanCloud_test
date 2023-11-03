#ifndef TABLEVIEWHEADER_H
#define TABLEVIEWHEADER_H

#include <QObject>
#include <QHeaderView>
#include <QMap>



class TableViewHeader : public QHeaderView
{
    Q_OBJECT
public:
    /**
     * @brief CHeaderView 构造函数
     * @param orientation 方向
     * @param parent 父类对象
     */
    TableViewHeader(Qt::Orientation orientation, QWidget *parent = nullptr);

    /**
     * @brief setColumnCheckable 设置指定列是否可选
     * @param column 指定列
     * @param checkable 可选值
     */
    void setColumnCheckable(int column, bool checkable);

signals:
    /**
     * @brief columnSectionClicked Section点击信号
     * @param logicalIndex 点击位置
     * @param checked 选中值
     */
    void columnSectionClicked(int logicalIndex, bool checked);

    // QHeaderView interface
protected:
    /**
     * @brief paintSection 绘制复选框
     * @param painter 绘制对象
     * @param rect 绘制区域
     * @param logicalIndex 当前索引位置
     */
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;

private:
    QMap<int, bool>    m_columpnCheckedMap; //包含复选框列的map容器

};

#endif // TABLEVIEWHEADER_H

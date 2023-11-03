/*
 * @description: 自定义的Qt数据模型类，继承自QAbstractTableModel。
 * @todo: 1. 初始化和数据设置：
 *          MyDataModel(QObject *parent = nullptr)： 构造函数，初始化数据模型。
 *          void setData(const QVector<QStringList> &data)： 设置数据模型的数据，同时初始化复选框状态和数据列数。
 *
 *        2. 数据行数和列数：
 *          int rowCount(const QModelIndex &parent = QModelIndex()) const： 返回数据行数，根据输入的 parent 参数可以实现更复杂的模型结构，但在此模型中未使用。
 *          int columnCount(const QModelIndex &parent = QModelIndex()) const： 返回数据列数，它是根据数据的第一行的列数来确定的，确保数据行的列数相同。
 *
 *        3. 数据获取：
 *          QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const： 获取特定索引的数据，根据索引和角色返回对应的数据，包括复选框状态、图标、普通数据。
 *
 *        4. 水平表头设置：
 *          QVariant headerData(int section, Qt::Orientation orientation, int role) const： 设置水平表头标题，根据角色和水平/垂直方向返回合适的标题。
 *
 *        5. 用户交互和复选框处理：
 *          Qt::ItemFlags flags(const QModelIndex &index) const： 设置Item的标志，允许用户操作复选框。
 *          bool setData(const QModelIndex &index, const QVariant &value, int role)： 处理复选框状态的变化，根据用户交互更新数据模型和通知视图更新。
 *
 *        6. 辅助函数：
 *          QVector<int> getSelectedRows() const： 返回数据模型中被用户选中的行号，用于获取用户选中的数据行。
 *          QStringList getRowData(int row) const： 根据行号返回数据模型中的数据，用于获取特定行的数据。
 *
 * @date: 2023/11/2
*/
#ifndef MYDATAMODEL_H
#define MYDATAMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>

class MyDataModel : public QAbstractTableModel {
    Q_OBJECT

public:
    static const int CHECKBOX_COLUMN = 0;

    MyDataModel(QObject *parent = nullptr);

    void setData(const QVector<QStringList> &data);                                                 //设置模型数据
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;                         //返回数据行数
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;                      //返回数据列数
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;             //返回特定索引的数据
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;         //设置水平表头
    Qt::ItemFlags flags(const QModelIndex &index) const override;                                   //设置Item的标志，允许用户操作复选框
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;               //处理复选框状态的变化
    QVector<int> getSelectedRows() const;                                                           //返回数据模型中被用户选中的行号
    QStringList getRowData(int row) const;                                                          //根据行号返回数据模型中的数据
private:
    QVector<QStringList> tableData;         //储存模型数据
    QVector<bool> checkboxStates;           //储存复选框状态
};

#endif // MYDATAMODEL_H

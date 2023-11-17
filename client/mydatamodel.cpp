#include "mydatamodel.h"
#include "qicon.h"

MyDataModel::MyDataModel(QObject *parent) : QAbstractTableModel(parent) {}

void MyDataModel::setData(const QVector<QStringList> &data)
{
    // 重置模型数据
    beginResetModel();
    tableData = data;
    // 初始化复选框状态的 QVector，确保其大小与数据行数一致
    checkboxStates.resize(rowCount());
    endResetModel();
}

int MyDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // 返回数据行数
    return tableData.size();
}

int MyDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // 如果数据为空，返回0，否则返回第一行的列数（每行的列数应该相同）
    if (tableData.isEmpty()) {
        return 0;
    }
    return tableData.first().size();
}

QVariant MyDataModel::data(const QModelIndex &index, int role) const
{
    // 检查索引的有效性
    if (!index.isValid() || index.row() >= tableData.size() || index.column() >= tableData.first().size())
    {
        return QVariant();
    }

    // 如果是第一列并且角色为 Qt::CheckStateRole，返回复选框的状态
    if (index.column() == 0 && role == Qt::CheckStateRole) {
        return checkboxStates[index.row()] ? Qt::Checked : Qt::Unchecked;
    }

    // 如果是第一列并且角色为 Qt::DecorationRole，返回图标类型
    if (index.column() == 0 && role == Qt::DecorationRole)
    {
        QString fileType = tableData[index.row()][2];
        if (fileType == "folder") {
            // 设置文件夹图标
            return QIcon(":/images/icon/folder.svg");
        } else {
            // 设置文件图标
            return QIcon(":/images/icon/file.svg");
        }
    }

    // 如果是第一列以外的其他列，并且角色为 Qt::DisplayRole，返回普通数据
    if (role == Qt::DisplayRole) {
        return tableData[index.row()][index.column()];
    }

    // 其他情况，返回空的 QVariant
    return QVariant();
}

QVariant MyDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // 设置水平表头的标题
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal && section >= 0 && section < tableData.first().size()) {
        QStringList headers = {"Name", "Upload Time", "Type", "Size", "Path"};
        return headers.at(section);
    }

    // 其他情况，返回空的 QVariant
    return QVariant();
}

Qt::ItemFlags MyDataModel::flags(const QModelIndex &index) const
{
    // 设置Item的标志，允许用户操作复选框
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == CHECKBOX_COLUMN) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

bool MyDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // 处理复选框状态的变化
    if (index.isValid() && index.column() == CHECKBOX_COLUMN && role == Qt::CheckStateRole) {
        checkboxStates[index.row()] = (value.toInt() == Qt::Checked);
        // 发送数据变化信号，通知视图更新
        emit dataChanged(index, index);
        return true;
    }
    return false;
}


/*返回数据模型中被用户选中的行号*/
QVector<int> MyDataModel::getSelectedRows() const
{
    QVector<int> selectedRows;
    for (int i = 0; i < checkboxStates.size(); ++i) {
        if (checkboxStates.at(i)) {
            selectedRows.append(i);
        }
    }
    return selectedRows;
}


/*根据行号返回数据模型中的数据*/
QStringList MyDataModel::getRowData(int row) const
{
    if(row >= 0 && row < tableData.size())
    {
        return tableData.at(row);
    }
    return QStringList();
}

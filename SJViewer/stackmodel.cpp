#include "stackmodel.h"

StackModel::StackModel(QStack<QString> &s,QObject *parent)
    : QAbstractListModel(parent)
{
    stack = &s;
}

QVariant StackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return "";
}

int StackModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return stack->size();
}

QVariant StackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() > stack->count())
            return QVariant();
    const QString & ct = (*stack)[index.row()];
    if (!ct.isEmpty())
        return ct;

   return QVariant();
}

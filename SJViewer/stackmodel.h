#ifndef STACKMODEL_H
#define STACKMODEL_H

#include <QAbstractListModel>
#include <QStack>
#include <QString>

class StackModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit StackModel(QStack<QString> &s, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QStack<QString> *stack;
};

#endif // STACKMODEL_H

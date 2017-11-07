
#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>

class TreeItem;

class TreeModel : public QAbstractItemModel
{

   Q_OBJECT

public:
    explicit TreeModel(QString **data, int sizeOfData, QObject *parent = 0);
    ~TreeModel();
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    TreeItem *rootItem;
private:
    void setupModelData(QString **data, int sizeOfData, TreeItem *parent);

};

class TreeItem
{
public:
    explicit TreeItem(QList<QVariant> &data, TreeItem *parent = 0);
    ~TreeItem();
    void appendChild(TreeItem *child);
    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();
    void changeData(QString);
private:
    QList<TreeItem*> childItems;
    QList<QVariant> itemData;
    TreeItem *parentItem;
};

#endif // TREEMODEL_H

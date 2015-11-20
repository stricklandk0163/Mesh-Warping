#ifndef HIERARCHICALMESHMODEL_H
#define HIERARCHICALMESHMODEL_H

#include <QAbstractItemModel>
#include <ObjectNode.h>

class HierarchicalMeshModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    HierarchicalMeshModel(ObjectNode* pRoot, QObject *parent = 0);
    ~HierarchicalMeshModel();

    // Implementing the QAbstractModelView Interface
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

protected:
    ObjectNode* mRoot;
};

#endif // HIERARCHICALMESHMODEL_H

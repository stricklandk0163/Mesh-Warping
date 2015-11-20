#include "HierarchicalMeshModel.h"

// Index for each column
#define COLUMN_NAME     0
#define COLUMN_VERTS    1
#define COLUMN_FACES    2

HierarchicalMeshModel::HierarchicalMeshModel(ObjectNode *pRoot, QObject *parent) :
    QAbstractItemModel(parent)
{
    mRoot = new ObjectNode();
    mRoot->addChild(pRoot);
}

HierarchicalMeshModel::~HierarchicalMeshModel()
{
    // We don't own the root so don't delete it!
}

int HierarchicalMeshModel::columnCount(const QModelIndex &parent) const { Q_UNUSED(parent); return 3; }

QVariant HierarchicalMeshModel::data(const QModelIndex &index, int role) const
{
    // Invalid index or something other than 'display'
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();


    ObjectNode *mesh = static_cast<ObjectNode*>(index.internalPointer());
    switch(index.column())
    {
        case COLUMN_NAME: return QVariant(mesh->getName()); break;
        case COLUMN_VERTS: return mesh->getVertexCount(); break;
        case COLUMN_FACES: return mesh->getFaceCount(); break;
        default: return QVariant(); break;
    }
}

Qt::ItemFlags HierarchicalMeshModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;
    return QAbstractItemModel::flags(index);
}

QVariant HierarchicalMeshModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch(section)
        {
            case COLUMN_NAME: return QString("Name"); break;
            case COLUMN_VERTS: return QString("Verts"); break;
            case COLUMN_FACES: return QString("Faces"); break;
            default: return QVariant(); break;
        }
    }

    return QVariant();
}

QModelIndex HierarchicalMeshModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if(!hasIndex(row, column, parent)) return QModelIndex();

    // Get the data for the parent (root item if none given)
    ObjectNode* parentMesh;
    if(!parent.isValid()) parentMesh = mRoot;
    else parentMesh = static_cast<ObjectNode*>(parent.internalPointer());

    // Get the indicated child (row index)
    ObjectNode *childMesh = parentMesh->getChild(row);
    if(childMesh != NULL) return createIndex(row, column, childMesh);
    else return QModelIndex();
}

QModelIndex HierarchicalMeshModel::parent(const QModelIndex &index) const
{
    if(!index.isValid()) return QModelIndex();

    ObjectNode *childMesh = static_cast<ObjectNode*>(index.internalPointer());
    ObjectNode *parentMesh = childMesh->parentMesh();

    if(parentMesh == mRoot) return QModelIndex();
    return createIndex(parentMesh->childIndex(), 0, parentMesh);
}

int HierarchicalMeshModel::rowCount(const QModelIndex &parent) const
{
    ObjectNode *parentItem;
    if(parent.column() > 0) return 0;
    if (!parent.isValid()) parentItem = mRoot;
    else parentItem = static_cast<ObjectNode*>(parent.internalPointer());

    return parentItem->getChildCount();
}

//void HierarchicalMeshModel::setupModelData(const QStringList &lines, TreeItem *parent)
//{
//    QList<TreeItem*> parents;
//    QList<int> indentations;
//    parents << parent;
//    indentations << 0;

//    int number = 0;

//    while (number < lines.count()) {
//        int position = 0;
//        while (position < lines[number].length()) {
//            if (lines[number].mid(position, 1) != " ")
//                break;
//            position++;
//        }

//        QString lineData = lines[number].mid(position).trimmed();

//        if (!lineData.isEmpty()) {
//            // Read the column data from the rest of the line.
//            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
//            QList<QVariant> columnData;
//            for (int column = 0; column < columnStrings.count(); ++column)
//                columnData << columnStrings[column];

//            if (position > indentations.last()) {
//                // The last child of the current parent is now the new parent
//                // unless the current parent has no children.

//                if (parents.last()->childCount() > 0) {
//                    parents << parents.last()->child(parents.last()->childCount()-1);
//                    indentations << position;
//                }
//            } else {
//                while (position < indentations.last() && parents.count() > 0) {
//                    parents.pop_back();
//                    indentations.pop_back();
//                }
//            }

//            // Append a new item to the current parent's list of children.
//            parents.last()->appendChild(new TreeItem(columnData, parents.last()));
//        }

//        ++number;
//    }
//}

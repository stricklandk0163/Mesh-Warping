#ifndef MODELAGENCYMAINWINDOW_H
#define MODELAGENCYMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class ModelAgencyMainWindow;
}

class ObjectNode;
class MaterialProperties;

class ModelAgencyMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ModelAgencyMainWindow(QWidget *parent = 0);
    ~ModelAgencyMainWindow();

protected slots:
    // Loading and saving of object files
    void on_actionLoad_Object_from_File_triggered();
    void on_actionSave_Object_to_File_triggered();

    // Quit the application
    void on_actionQuit_triggered();

    // Create/Load various objects
    void on_actionMake_Cube_triggered();
    void on_actionMake_Cylinder_triggered();
    void on_actionMake_Sphere_triggered();

    void on_actionMake_Sculpture_triggered();
    void on_actionMake_Humanoid_triggered();

    void on_actionExtra_Credit_1_triggered();
    void on_actionExtra_Credit_2_triggered();

    // Change appearance of OpenGL renderer
    void on_actionNormal_Rendering_Mode_toggled(bool checked);
    void on_actionVisualize_Normals_as_Color_toggled(bool checked);
    void on_actionVisualize_Texture_Coordinates_as_Color_toggled(bool checked);
    void on_actionSolid_Color_Only_no_shade_toggled(bool checked);

    // Respond to the 'use parent' materials checkbox
    void on_materialUseParentCheckBox_toggled(bool checked);

    // Sync values from the GUI back to the object
    void syncMaterials();
    void syncTransformation();

    // Respond to changed selection in the TreeView
    void treeViewSelectionChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void processNewObject(ObjectNode* pNewObject);

    void clearObjectUIData();
    void setMaterialValues(MaterialProperties *pMtrl);
    void setTransformationValues(ObjectNode *pCurObj);
    void setMaterialUIState(bool checked);

    bool mBlockSyncSlots, mAllowUncheck;
    Ui::ModelAgencyMainWindow *ui;
};

#endif // MODELAGENCYMAINWINDOW_H

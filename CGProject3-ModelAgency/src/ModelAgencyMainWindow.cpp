#include "ModelAgencyMainWindow.h"
#include "ui_ModelAgencyMainWindow.h"
#include "ui_SphereDialog.h"
#include "ui_CylinderDialog.h"
#include "ui_GenericDialog.h"

#include "ObjectFactory.h"
#include "ObjectNode.h"
#include "HierarchicalMeshModel.h"
#include "MaterialProperties.h"

#include <QFileDialog>
#include <QMessageBox>

ModelAgencyMainWindow::ModelAgencyMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ModelAgencyMainWindow)
{
    // Setup the UI form
    ui->setupUi(this);
    ui->actionQuit->setShortcut(QKeySequence::Quit);

    // Connect actions to GLWidget
    connect(ui->actionShow_Bounding_Box_and_Axies, SIGNAL(toggled(bool)), ui->mainGLWidget, SLOT(enableAxies(bool)));
    connect(ui->actionShow_Vertex_Normals, SIGNAL(toggled(bool)), ui->mainGLWidget, SLOT(enableNormals(bool)));
    connect(ui->actionCycle_Wireframe_Mode, SIGNAL(triggered()), ui->mainGLWidget, SLOT(cycleWireframeMode()));
    connect(ui->actionCompute_Per_face_Normals, SIGNAL(triggered()), ui->mainGLWidget, SLOT(computePerFaceNormals()));
    connect(ui->actionSmooth_all_Normals, SIGNAL(triggered()), ui->mainGLWidget, SLOT(smoothAllNormals()));

    mBlockSyncSlots = false;
    mAllowUncheck = false;
}

ModelAgencyMainWindow::~ModelAgencyMainWindow()
{
    delete ui;
}

void ModelAgencyMainWindow::syncMaterials()
{
    if(mBlockSyncSlots) return;

    QModelIndex lCurrent = ui->treeView->currentIndex();
    if(!lCurrent.isValid()) return;

    ObjectNode* lCurObj = static_cast<ObjectNode*>(lCurrent.internalPointer());
    if(lCurObj == NULL) return;

    MaterialProperties *lMat = lCurObj->getMaterials();
    if(lMat != NULL)
    {
        lMat->setAmbient(ui->ambRSpinBox->value(), ui->ambGSpinBox->value(), ui->ambBSpinBox->value());
        lMat->setDiffuse(ui->diffRSpinBox->value(), ui->diffGSpinBox->value(), ui->diffBSpinBox->value());
        lMat->setSpecular(ui->specRSpinBox->value(), ui->specGSpinBox->value(), ui->specBSpinBox->value());
        lMat->setSpecularExp(ui->shineSpinBox->value());
    }

    ui->mainGLWidget->updateGL();
}

void ModelAgencyMainWindow::syncTransformation()
{
    if(mBlockSyncSlots) return;

    QModelIndex lCurrent = ui->treeView->currentIndex();
    if(!lCurrent.isValid()) return;

    ObjectNode* lCurObj = static_cast<ObjectNode*>(lCurrent.internalPointer());
    if(lCurObj == NULL) return;

    lCurObj->setTranslate(ui->translationXSpinBox->value(), ui->translationYSpinBox->value(), ui->translationZSpinBox->value());
    lCurObj->setCenterOfRotation(ui->centerRXSpinBox->value(), ui->centerRYSpinBox->value(), ui->centerRZSpinBox->value());
    lCurObj->setRotate(ui->rotationXSpinBox->value(), ui->rotationYSpinBox->value(), ui->rotationZSpinBox->value());
    lCurObj->setScale(ui->scaleXSpinBox->value(), ui->scaleYSpinBox->value(), ui->scaleZSpinBox->value());

    ui->mainGLWidget->updateGL();
}

void ModelAgencyMainWindow::on_materialUseParentCheckBox_toggled(bool checked)
{
    if(checked)
    {
        int result = QMessageBox::warning(this, "Delete properties", "You will lose the material properties for this node.  Are you sure?",
                             QMessageBox::Cancel, QMessageBox::Yes);
        if(result == QMessageBox::Cancel)
        {
            ui->materialUseParentCheckBox->blockSignals(true);
            ui->materialUseParentCheckBox->setChecked(false);
            ui->materialUseParentCheckBox->blockSignals(false);
            return;
        }
    }

    QModelIndex lIndex = ui->treeView->currentIndex();
    if(!lIndex.isValid()) return;
    ObjectNode *lCurObj = static_cast<ObjectNode*>(lIndex.internalPointer());
    if(lCurObj == NULL) return;

    if(checked)
    {
        delete lCurObj->getMaterials();
        lCurObj->setMaterials(NULL);
    }
    else
    {
        MaterialProperties *lNewMat;
        if(lCurObj->parentMesh() != NULL && lCurObj->parentMesh()->getMaterials() != NULL)
        {
            lNewMat = new MaterialProperties(*lCurObj->parentMesh()->getMaterials());
        }
        else lNewMat = new MaterialProperties();

        setMaterialValues(lNewMat);
        lCurObj->setMaterials(lNewMat);
    }

    setMaterialUIState(checked);
}

inline void ModelAgencyMainWindow::setMaterialUIState(bool checked)
{
    ui->ambRSpinBox->setDisabled(checked);
    ui->ambGSpinBox->setDisabled(checked);
    ui->ambBSpinBox->setDisabled(checked);

    ui->diffRSpinBox->setDisabled(checked);
    ui->diffGSpinBox->setDisabled(checked);
    ui->diffBSpinBox->setDisabled(checked);

    ui->specRSpinBox->setDisabled(checked);
    ui->specGSpinBox->setDisabled(checked);
    ui->specBSpinBox->setDisabled(checked);

    ui->shineSpinBox->setDisabled(checked);
}

inline void ModelAgencyMainWindow::clearObjectUIData()
{
    ui->ambRSpinBox->setValue(0);
    ui->ambGSpinBox->setValue(0);
    ui->ambBSpinBox->setValue(0);

    ui->diffRSpinBox->setValue(0);
    ui->diffGSpinBox->setValue(0);
    ui->diffBSpinBox->setValue(0);

    ui->specRSpinBox->setValue(0);
    ui->specGSpinBox->setValue(0);
    ui->specBSpinBox->setValue(0);
    ui->shineSpinBox->setValue(0);

    ui->appearanceGroupBox->setEnabled(false);

    ui->translationXSpinBox->setValue(0);
    ui->translationYSpinBox->setValue(0);
    ui->translationZSpinBox->setValue(0);

    ui->scaleXSpinBox->setValue(1);
    ui->scaleYSpinBox->setValue(1);
    ui->scaleZSpinBox->setValue(1);

    ui->centerRXSpinBox->setValue(0);
    ui->centerRYSpinBox->setValue(0);
    ui->centerRZSpinBox->setValue(0);

    ui->rotationXSpinBox->setValue(0);
    ui->rotationYSpinBox->setValue(0);
    ui->rotationZSpinBox->setValue(0);

    ui->transformationGroupBox->setEnabled(false);
}

void ModelAgencyMainWindow::treeViewSelectionChanged(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);

    mBlockSyncSlots = true;
    if(!current.isValid())
    {
        clearObjectUIData();
        mBlockSyncSlots = false;
        return;
    }

    ObjectNode* lCurObj = static_cast<ObjectNode*>(current.internalPointer());
    if(lCurObj == NULL)
    {
        clearObjectUIData();
        mBlockSyncSlots = false;
        return;
    }

    // Appearance data (coming soon)
    MaterialProperties *lMtrl = lCurObj->getMaterials();
    setMaterialValues(lMtrl);

    // Transformation data
    setTransformationValues(lCurObj);

    mBlockSyncSlots = false;
}

inline void ModelAgencyMainWindow::setMaterialValues(MaterialProperties *pMtrl)
{
    ui->materialUseParentCheckBox->blockSignals(true);

    if(pMtrl == NULL)
    {
        ui->materialUseParentCheckBox->setChecked(true);
        setMaterialUIState(true);
    }
    else
    {
        ui->materialUseParentCheckBox->setChecked(false);
        setMaterialUIState(false);

        const QColor &lAmb = pMtrl->getAmbient();
        ui->ambRSpinBox->setValue(lAmb.red());
        ui->ambGSpinBox->setValue(lAmb.green());
        ui->ambBSpinBox->setValue(lAmb.blue());

        const QColor &lDiff = pMtrl->getDiffuse();
        ui->diffRSpinBox->setValue(lDiff.red());
        ui->diffGSpinBox->setValue(lDiff.green());
        ui->diffBSpinBox->setValue(lDiff.blue());

        const QColor &lSpec = pMtrl->getSpecular();
        ui->specRSpinBox->setValue(lSpec.red());
        ui->specGSpinBox->setValue(lSpec.green());
        ui->specBSpinBox->setValue(lSpec.blue());

        ui->shineSpinBox->setValue(pMtrl->getSpecularExp());
    }

    ui->appearanceGroupBox->setEnabled(true);
    ui->materialUseParentCheckBox->blockSignals(false);
}

inline void ModelAgencyMainWindow::setTransformationValues(ObjectNode *pCurObj)
{
    float lTx, lTy, lTz;
    pCurObj->getTranslation(lTx, lTy, lTz);
    ui->translationXSpinBox->setValue(lTx);
    ui->translationYSpinBox->setValue(lTy);
    ui->translationZSpinBox->setValue(lTz);

    float lSx, lSy, lSz;
    pCurObj->getScale(lSx, lSy, lSz);
    ui->scaleXSpinBox->setValue(lSx);
    ui->scaleYSpinBox->setValue(lSy);
    ui->scaleZSpinBox->setValue(lSz);

    float lRx, lRy, lRz;
    pCurObj->getRotation(lRx, lRy, lRz);
    ui->rotationXSpinBox->setValue(lRx);
    ui->rotationYSpinBox->setValue(lRy);
    ui->rotationZSpinBox->setValue(lRz);

    float lCRx, lCRy, lCRz;
    pCurObj->getCenterOfRotation(lCRx, lCRy, lCRz);
    ui->centerRXSpinBox->setValue(lCRx);
    ui->centerRYSpinBox->setValue(lCRy);
    ui->centerRZSpinBox->setValue(lCRz);

    ui->transformationGroupBox->setEnabled(true);
}

inline void ModelAgencyMainWindow::processNewObject(ObjectNode* pNewObject)
{
    // Build model for the tree view
    HierarchicalMeshModel* lModel = new HierarchicalMeshModel(pNewObject, ui->treeView);
    ui->treeView->setModel(lModel);
    connect(ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(treeViewSelectionChanged(QModelIndex,QModelIndex)));

    // Cleanup the ui
    clearObjectUIData();

    // Some reasonable column widths
    ui->treeView->setColumnWidth(0, 200);
    ui->treeView->setColumnWidth(1, 60);
    ui->treeView->setColumnWidth(2, 60);

    // Give the object to the OpenGL widget
    ui->mainGLWidget->takeObject(pNewObject);
}

void ModelAgencyMainWindow::on_actionSave_Object_to_File_triggered()
{
    const ObjectNode* lObject = ui->mainGLWidget->getObject();
    if(lObject == NULL)
    {
        QMessageBox::warning(this, "Error saving", "Nothing to save.  Please create an object first.");
        return;
    }

    QString lFilename = QFileDialog::getSaveFileName(this, "Please specify a file name for the object", "", "Wavefront obj (*.obj)");
    if(lFilename.isEmpty()) return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    bool result = lObject->writeFile(lFilename.toStdString());
    QApplication::restoreOverrideCursor();

    if(!result)
    {
        QMessageBox::warning(this, "Error saving", "There was an error saving the mesh.  See standard output for more information.");
    }
}

void ModelAgencyMainWindow::on_actionLoad_Object_from_File_triggered()
{
    QString mFilename = QFileDialog::getOpenFileName(this, "Please select an object file to open", "", "Wavefront obj (*.obj);;All Files (*.*)");
    if(mFilename.isEmpty()) return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ObjectNode* lNewObject = ObjectFactory::loadMesh(mFilename);
    QApplication::restoreOverrideCursor();

    if(lNewObject != NULL) processNewObject(lNewObject);
    else
    {
        QMessageBox::warning(this, "Error loading", "There was an error reading the mesh.  See standard output for more information.");
    }
}

void ModelAgencyMainWindow::on_actionQuit_triggered()
{
    exit(0);
}

// Calls to make the basic shapes
void ModelAgencyMainWindow::on_actionMake_Cube_triggered()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ObjectNode* lNewObject = ObjectFactory::buildCube();
    QApplication::restoreOverrideCursor();

    if(lNewObject != NULL) processNewObject(lNewObject);
    else
    {
        QMessageBox::warning(this, "Error making cube", "There was an error constructing the cube mesh. See standard output for more information.");
    }
}

void ModelAgencyMainWindow::on_actionMake_Cylinder_triggered()
{
    QDialog *lCylinderDialog = new QDialog(this);
    Ui::CylinderDialog *lDialogUI = new Ui::CylinderDialog();

    lDialogUI->setupUi(lCylinderDialog);
    if(lCylinderDialog->exec() == QDialog::Rejected)
    {
        delete lDialogUI;
        delete lCylinderDialog;
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ObjectNode* lNewObject = ObjectFactory::buildCylinder(lDialogUI->slicesSpinBox->value());
    QApplication::restoreOverrideCursor();

    if(lNewObject != NULL) processNewObject(lNewObject);
    else
    {
        QMessageBox::warning(this, "Error making cylinder", "There was an error constructing the cylinder mesh. See standard output for more information.");
    }

    delete lDialogUI;
    delete lCylinderDialog;
}

void ModelAgencyMainWindow::on_actionMake_Sphere_triggered()
{
    QDialog *lSphereDialog = new QDialog(this);
    Ui::SphereDialog *lDialogUI = new Ui::SphereDialog();

    lDialogUI->setupUi(lSphereDialog);
    if(lSphereDialog->exec() == QDialog::Rejected)
    {
        delete lDialogUI;
        delete lSphereDialog;
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ObjectNode* lNewObject = ObjectFactory::buildSphere(lDialogUI->slicesSpinBox->value(), lDialogUI->stacksSpinBox->value());
    QApplication::restoreOverrideCursor();

    if(lNewObject != NULL) processNewObject(lNewObject);
    else
    {
        QMessageBox::warning(this, "Error making sphere", "There was an error constructing the sphere mesh. See standard output for more information.");
    }

    delete lDialogUI;
    delete lSphereDialog;
}

// Calls to make the sculpture
void ModelAgencyMainWindow::on_actionMake_Sculpture_triggered()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ObjectNode* lNewObject = ObjectFactory::buildSculpture();
    QApplication::restoreOverrideCursor();

    if(lNewObject != NULL) processNewObject(lNewObject);
    else
    {
        QMessageBox::warning(this, "Error making sculpture", "There was an error constructing the sculpture mesh. See standard output for more information.");
    }
}

// Calls to make the humanoid
void ModelAgencyMainWindow::on_actionMake_Humanoid_triggered()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ObjectNode* lNewObject = ObjectFactory::buildHumanoid();
    QApplication::restoreOverrideCursor();

    if(lNewObject != NULL) processNewObject(lNewObject);
    else
    {
        QMessageBox::warning(this, "Error making humanoid", "There was an error constructing the humanoid mesh. See standard output for more information.");
    }
}

// Extra calls for any extra credit you might want to attempt
void ModelAgencyMainWindow::on_actionExtra_Credit_1_triggered()
{
    QDialog *lGenericDialog = new QDialog(this);
    Ui::GenericDialog *lDialogUI = new Ui::GenericDialog();

    lDialogUI->setupUi(lGenericDialog);
    if(lGenericDialog->exec() == QDialog::Rejected)
    {
        delete lDialogUI;
        delete lGenericDialog;
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ObjectNode* lNewObject = ObjectFactory::buildExtraObject1(lDialogUI->parameter1SpinBox->value(),
                                                              lDialogUI->parameter2SpinBox->value(),
                                                              lDialogUI->parameter3SpinBox->value());
    QApplication::restoreOverrideCursor();

    if(lNewObject != NULL) processNewObject(lNewObject);
    else
    {
        QMessageBox::warning(this, "Error making extra object 1", "There was an error constructing the extra object 1 mesh. See standard output for more information.");
    }

    delete lDialogUI;
    delete lGenericDialog;
}

void ModelAgencyMainWindow::on_actionExtra_Credit_2_triggered()
{
    QDialog *lGenericDialog = new QDialog(this);
    Ui::GenericDialog *lDialogUI = new Ui::GenericDialog();

    lDialogUI->setupUi(lGenericDialog);
    if(lGenericDialog->exec() == QDialog::Rejected)
    {
        delete lDialogUI;
        delete lGenericDialog;
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ObjectNode* lNewObject = ObjectFactory::buildExtraObject2(lDialogUI->parameter1SpinBox->value(),
                                                              lDialogUI->parameter2SpinBox->value(),
                                                              lDialogUI->parameter3SpinBox->value());
    QApplication::restoreOverrideCursor();

    if(lNewObject != NULL) processNewObject(lNewObject);
    else
    {
        QMessageBox::warning(this, "Error making extra object 3", "There was an error constructing the extra object 3 mesh. See standard output for more information.");
    }

    delete lDialogUI;
    delete lGenericDialog;
}

void ModelAgencyMainWindow::on_actionNormal_Rendering_Mode_toggled(bool checked)
{
    if(checked)
    {
        mAllowUncheck = true;
        ui->mainGLWidget->changeRenderingMode(PHONG_MATERIAL_MODE_SHADED);
        ui->actionSolid_Color_Only_no_shade->setChecked(false);
        ui->actionVisualize_Normals_as_Color->setChecked(false);
        ui->actionVisualize_Texture_Coordinates_as_Color->setChecked(false);
        mAllowUncheck = false;
    }
    else if(!mAllowUncheck)
    {
        ui->actionNormal_Rendering_Mode->blockSignals(true);
        ui->actionNormal_Rendering_Mode->setChecked(true);
        ui->actionNormal_Rendering_Mode->blockSignals(false);
    }
}

void ModelAgencyMainWindow::on_actionVisualize_Normals_as_Color_toggled(bool checked)
{
    if(checked)
    {
        mAllowUncheck = true;
        ui->mainGLWidget->changeRenderingMode(PHONG_MATERIAL_MODE_NORMALS);
        ui->actionNormal_Rendering_Mode->setChecked(false);
        ui->actionSolid_Color_Only_no_shade->setChecked(false);
        ui->actionVisualize_Texture_Coordinates_as_Color->setChecked(false);
        mAllowUncheck = false;
    }
    else if(!mAllowUncheck)
    {
        ui->actionVisualize_Normals_as_Color->blockSignals(true);
        ui->actionVisualize_Normals_as_Color->setChecked(true);
        ui->actionVisualize_Normals_as_Color->blockSignals(false);
    }
}

void ModelAgencyMainWindow::on_actionVisualize_Texture_Coordinates_as_Color_toggled(bool checked)
{
    if(checked)
    {
        mAllowUncheck = true;
        ui->mainGLWidget->changeRenderingMode(PHONG_MATERIAL_MODE_TEX_CORDS);
        ui->actionNormal_Rendering_Mode->setChecked(false);
        ui->actionSolid_Color_Only_no_shade->setChecked(false);
        ui->actionVisualize_Normals_as_Color->setChecked(false);
        mAllowUncheck = false;
    }
    else if(!mAllowUncheck)
    {
        ui->actionVisualize_Texture_Coordinates_as_Color->blockSignals(true);
        ui->actionVisualize_Texture_Coordinates_as_Color->setChecked(true);
        ui->actionVisualize_Texture_Coordinates_as_Color->blockSignals(false);
    }
}

void ModelAgencyMainWindow::on_actionSolid_Color_Only_no_shade_toggled(bool checked)
{
    if(checked)
    {
        mAllowUncheck = true;
        ui->mainGLWidget->changeRenderingMode(PHONG_MATERIAL_MODE_SOLID_COLOR);
        ui->actionNormal_Rendering_Mode->setChecked(false);
        ui->actionVisualize_Normals_as_Color->setChecked(false);
        ui->actionVisualize_Texture_Coordinates_as_Color->setChecked(false);
        mAllowUncheck = false;
    }
    else if(!mAllowUncheck)
    {
        ui->actionSolid_Color_Only_no_shade->blockSignals(true);
        ui->actionSolid_Color_Only_no_shade->setChecked(true);
        ui->actionSolid_Color_Only_no_shade->blockSignals(false);
    }
}

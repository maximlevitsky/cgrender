/*
	This file is part of CG4.

	Copyright (c) Inbar Donag and Maxim Levitsky

    CG4 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CG4 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CG4.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "ui_about_dialog.h"

#include "engine/Engine.h"
#include "renderer/Renderer.h"

#include "AboutDialog.h"
#include "MouseSensivetyDialog.h"
#include "EnvironmentDialog.h"
#include "CameraPropertiesDialog.h"
#include "MaterialsDialog.h"

#include <QFileDialog>

MainWindow::MainWindow()
{
	/* settings*/
	rotationSensivety = 50;
	movementSensivety = 50;
	scaleSensivety = 50;

	/* Engine setup*/
	engine = new Engine();
	renderer = new Renderer();
	engine->setRenderer(renderer);

	/* GUI setup */
	setupUi(this);

	/* File menu */
	connect(actionLoad, SIGNAL(triggered()), this, SLOT(onModelLoad()));
	connect(actionResetModel, SIGNAL(triggered()), this, SLOT(onReset()));
	connect(actionLoad_debug_model, SIGNAL(triggered()), this, SLOT(onLoadDebugModel()));
	connect(actionSave_screenshot, SIGNAL(triggered()), this, SLOT(onSaveScreenShot()));


	/* scene menu */
	cameraPropertiesDialog = new CameraPropertiesDialog(this);
	environmentDialog = new EnvironmentDialog(this);
	materialDialog = new MaterialsDialog(this);
	connect(actionCamera_properties, SIGNAL(triggered()), this, SLOT(onCameraPropertiesDialog()));
	connect(actionEnvironment, SIGNAL(triggered()), this, SLOT(onEnvironmentDialog()));
	connect(actionMaterial, SIGNAL(triggered()), this, SLOT(onMaterialDialog()));

	/* view menu*/
	actionBounding_box->setChecked(engine->getDrawBoundingBox());
	actionAxes->setChecked(engine->getDrawAxes());
	actionNormals->setChecked(engine->getdrawVertexNormals());
	actionFaces->setChecked(engine->getdrawPolygonNormals());
	actionWireframe->setChecked(engine->getdrawWireFrame());
	actionLight_sources->setChecked(engine->getDrawLightSources());
	actionBack_face_culling->setChecked(engine->getBackfaceCulling());
	actionDepth_buffer_visualization->setChecked(engine->getDepthRendering());

	connect(actionBounding_box, SIGNAL(toggled(bool)), this, SLOT(onDrawBoundingBox()));
	connect(actionAxes, SIGNAL(toggled(bool)), this, SLOT(onDrawAxes()));
	connect(actionNormals, SIGNAL(toggled(bool)), this, SLOT(onDrawNormals()));
	connect(actionFaces, SIGNAL(toggled(bool)), this, SLOT(onDrawfaceNormals()));
	connect(actionWireframe, SIGNAL(toggled(bool)), this, SLOT(onDrawWireframe()));
	connect(actionLight_sources, SIGNAL(toggled(bool)), this, SLOT(onDrawLightSources()));
	connect(actionBack_face_culling, SIGNAL(toggled(bool)), this, SLOT(onBackFaceCulling()));
	connect(actionDepth_buffer_visualization, SIGNAL(toggled(bool)), this, SLOT(onDrawDepthbuffer()));

	/* Transformation menu */
	QActionGroup* transformGroup = new QActionGroup( this );
	actionCamera->setActionGroup(transformGroup);
	actionWorld->setActionGroup(transformGroup);
	actionWorld_separate_objects->setActionGroup(transformGroup);
	_transformMode = TRANSFORM_OBJECT;
	actionWorld->setChecked(true);

	connect(actionCamera, SIGNAL(toggled(bool)), this, SLOT(onCameraTransformMode()));
	connect(actionWorld, SIGNAL(toggled(bool)), this, SLOT(onWorldTransformationMode()));
	connect(actionWorld_separate_objects, SIGNAL(toggled(bool)), this, SLOT(onSeparateObjectsMode()));
	connect(actionTransformationsReset, SIGNAL(triggered()), this, SLOT(onTransformationsReset()));
	connect(actionMouse_sensivety, SIGNAL(triggered()), this, SLOT(onMouseSensivetyDialog()));
	mouseSensivetyDialog = new MouseSensivetyDialog(this);

	/* Shading menu*/
	QActionGroup* shadingGroup = new QActionGroup( this );
	actionFlat->setActionGroup(shadingGroup);
	actionGourald->setActionGroup(shadingGroup);
	actionPhong->setActionGroup(shadingGroup);
	actionPhong->setChecked(true);
	connect(actionFlat, SIGNAL(toggled(bool)), this, SLOT(onShadingFlat()));
	connect(actionGourald, SIGNAL(toggled(bool)), this, SLOT(onShadingGorald()));
	connect(actionPhong, SIGNAL(toggled(bool)), this, SLOT(onShadingPhong()));

	actionInvert_vertex_normals->setChecked(engine->getInvertedVertexNormals());
	actionInvert_faces->setChecked(engine->getInvertedPolygonNormals());
	actionDual_face_lighting->setChecked(engine->getLightBackFaces());
	actionAll_face_lighting->setChecked(engine->getForceAllFrontFaces());

	connect(actionInvert_vertex_normals, SIGNAL(toggled(bool)), this, SLOT(onInvertNormals()));
	connect(actionInvert_faces, SIGNAL(toggled(bool)), this, SLOT(onInvertFaces()));
	connect(actionDual_face_lighting, SIGNAL(toggled(bool)), this, SLOT(onDualfaceLighting()));
	connect(actionAll_face_lighting, SIGNAL(toggled(bool)), this, SLOT(onAllFaceLighting()));

	/* Help menu */
	connect(actionAbout, SIGNAL(triggered()), this, SLOT(onAboutDialog()));

}

MainWindow::~MainWindow()
{
	delete engine;
	delete renderer;
}



/***************************************************************************************/
/* FILE MENU */
/***************************************************************************************/

void MainWindow::onModelLoad() {
	QFileDialog *dlg  = new QFileDialog(this);
	dlg->setAcceptMode(QFileDialog::AcceptOpen);
	if (!dlg->exec())
		return;

	QStringList fileNames = dlg->selectedFiles();
	for (QString& filename : fileNames)
		 engine->loadSceneFromOBJ(filename.toStdString().c_str());

	drawArea->invalidateScene();
}

void MainWindow::onReset()
{
	engine->resetScene();
	drawArea->invalidateScene();

}

void MainWindow::onLoadDebugModel()
{
	engine->loadDebugScene();
	drawArea->invalidateScene();
}

void MainWindow::onSaveScreenShot()
{
	/* TODO */
}


/***************************************************************************************/
/* SCENE MENU */
/***************************************************************************************/

void MainWindow::onEnvironmentDialog() {
	environmentDialog->show();
}

void MainWindow::onMaterialDialog() {
	materialDialog->show();
}

void MainWindow::onCameraPropertiesDialog() {
	cameraPropertiesDialog->show();
}


/***************************************************************************************/
/* VIEW MENU */
/***************************************************************************************/

void MainWindow::onDrawBoundingBox()
{
	engine->setDrawBoundingBox(!engine->getDrawBoundingBox());
	drawArea->invalidateScene();
}

void MainWindow::onDrawAxes()
{
	engine->setDrawAxes(!engine->getDrawAxes());
	drawArea->invalidateScene();
}

void MainWindow::onDrawNormals()
{
	engine->setDrawVertexNormals(!engine->getdrawVertexNormals());
	drawArea->invalidateScene();
}
void MainWindow::onDrawfaceNormals()
{
	engine->setDrawPolygonNormals(!engine->getdrawPolygonNormals());
	drawArea->invalidateScene();
}
void MainWindow::onDrawWireframe()
{
	engine->setDrawWireFrame(!engine->getdrawWireFrame());
	drawArea->invalidateScene();
}
void MainWindow::onDrawLightSources()
{
	engine->setDrawLightSources(!engine->getDrawLightSources());
	drawArea->invalidateScene();
}
void MainWindow::onBackFaceCulling()
{
	engine->setBackFaceCulling(!engine->getBackfaceCulling());
	drawArea->invalidateScene();
}

void MainWindow::onDrawDepthbuffer()
{
	engine->setDepthRendering(!engine->getDepthRendering());
	drawArea->invalidateScene();
}


/***************************************************************************************/
/* SHADING MENU */
/***************************************************************************************/
void MainWindow::onShadingFlat()
{
	engine->setShadingMode(Engine::SHADING_FLAT);
	drawArea->invalidateScene();
}

void MainWindow::onShadingGorald()
{
	engine->setShadingMode(Engine::SHADING_GOURAD);
	drawArea->invalidateScene();
}

void MainWindow::onShadingPhong()
{
	engine->setShadingMode(Engine::SHADING_PHONG);
	drawArea->invalidateScene();
}

void MainWindow::onInvertNormals()
{
	engine->invertVertexNormals(!engine->getInvertedVertexNormals());
	drawArea->invalidateScene();
}

void MainWindow::onInvertFaces()
{
	engine->invertPolygonNormals(!engine->getInvertedPolygonNormals());
	drawArea->invalidateScene();
}

void MainWindow::onDualfaceLighting()
{
	engine->setLightBackFaces(!engine->getLightBackFaces());
	drawArea->invalidateScene();
}

void MainWindow::onAllFaceLighting()
{
	engine->setForceAllFrontFaces(!engine->getForceAllFrontFaces());
	drawArea->invalidateScene();
}

/***************************************************************************************/
/* TRANSOFORMATIONS MENU */
/***************************************************************************************/

void MainWindow::onMouseSensivetyDialog()
{
	mouseSensivetyDialog->show();
}

void MainWindow::onCameraTransformMode()
{
	_transformMode = TRANSFORM_CAMERA;
	engine->setDrawSeperateObjects(false);
	drawArea->invalidateScene();
}

void MainWindow::onWorldTransformationMode()
{
	_transformMode = TRANSFORM_OBJECT;
	engine->setDrawSeperateObjects(false);
	drawArea->invalidateScene();
}

void MainWindow::onSeparateObjectsMode()
{
	_transformMode = TRANSFORM_OBJECT;
	engine->setDrawSeperateObjects(true);
	drawArea->invalidateScene();
}

void MainWindow::onTransformationsReset()
{
	engine->resetTransformations();
	drawArea->invalidateScene();
}


/***************************************************************************************/
/* HELP MENU */
/***************************************************************************************/

void MainWindow::onAboutDialog()
{
	(new AboutDialog(this))->show();
}

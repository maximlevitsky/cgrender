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
#include "SidePanel.h"

#include <QFileDialog>
#include <QDockWidget>

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


	/* view menu*/
	sidePanel = new SidePanel(this);
	panelShown = true;
    this->addDockWidget(Qt::RightDockWidgetArea, sidePanel);
    actionSidePanel->setChecked(true);

	connect(actionSidePanel, SIGNAL(toggled(bool)), this, SLOT(onSidePanelShowHide(bool)));
    connect(actionBounding_box, SIGNAL(toggled(bool)), this, SLOT(onDrawBoundingBox(bool)));
	connect(actionAxes, SIGNAL(toggled(bool)), this, SLOT(onDrawAxes(bool)));
	connect(actionNormals, SIGNAL(toggled(bool)), this, SLOT(onDrawNormals(bool)));
	connect(actionFaces, SIGNAL(toggled(bool)), this, SLOT(onDrawfaceNormals(bool)));
	connect(actionWireframe, SIGNAL(toggled(bool)), this, SLOT(onDrawWireframe(bool)));
	connect(actionLight_sources, SIGNAL(toggled(bool)), this, SLOT(onDrawLightSources(bool)));
	connect(actionBack_face_culling, SIGNAL(toggled(bool)), this, SLOT(onBackFaceCulling(bool)));
	connect(actionDepth_buffer_visualization, SIGNAL(toggled(bool)), this, SLOT(onDrawDepthbuffer(bool)));

	/* Transformation menu */
	QActionGroup* transformGroup = new QActionGroup( this );
	actionCamera->setActionGroup(transformGroup);
	actionWorld->setActionGroup(transformGroup);
	actionWorld_separate_objects->setActionGroup(transformGroup);
	_transformMode = TRANSFORM_OBJECT;
	actionWorld->setChecked(true);
	connect(actionLeft_coordinate_system, SIGNAL(toggled(bool)), this, SLOT(onLeftCoordinateSystem(bool)));

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

	connect(actionFlat, SIGNAL(toggled(bool)), this, SLOT(onShadingFlat(bool)));
	connect(actionGourald, SIGNAL(toggled(bool)), this, SLOT(onShadingGorald(bool)));
	connect(actionPhong, SIGNAL(toggled(bool)), this, SLOT(onShadingPhong(bool)));

	connect(actionInvert_vertex_normals, SIGNAL(toggled(bool)), this, SLOT(onInvertNormals(bool)));
	connect(actionInvert_faces, SIGNAL(toggled(bool)), this, SLOT(onInvertFaces(bool)));
	connect(actionDual_face_lighting, SIGNAL(toggled(bool)), this, SLOT(onDualfaceLighting(bool)));
	connect(actionAll_face_lighting, SIGNAL(toggled(bool)), this, SLOT(onAllFaceLighting(bool)));

	/* Help menu */
	connect(actionAbout, SIGNAL(triggered()), this, SLOT(onAboutDialog()));

	updateStatus();

}

void MainWindow::updateStatus()
{
	/* scene menu*/
	actionLeft_coordinate_system->setChecked(engine->getInvertDepth());

	/* view menu*/
	actionBounding_box->setChecked(engine->getDrawBoundingBox());
	actionAxes->setChecked(engine->getDrawAxes());
	actionNormals->setChecked(engine->getdrawVertexNormals());
	actionFaces->setChecked(engine->getdrawPolygonNormals());
	actionWireframe->setChecked(engine->getdrawWireFrame());
	actionLight_sources->setChecked(engine->getDrawLightSources());
	actionBack_face_culling->setChecked(engine->getBackfaceCulling());
	actionDepth_buffer_visualization->setChecked(engine->getDepthRendering());

	/* Shading menu*/
	actionPhong->setChecked(engine->getShadingMode() == Engine::SHADING_PHONG);
	actionGourald->setChecked(engine->getShadingMode() == Engine::SHADING_GOURAD);
	actionFlat->setChecked(engine->getShadingMode() == Engine::SHADING_FLAT);

	actionInvert_vertex_normals->setChecked(engine->getInvertNormals());
	actionInvert_faces->setChecked(engine->getInvertFaces());
	actionDual_face_lighting->setChecked(engine->getLightBackFaces());
	actionAll_face_lighting->setChecked(engine->getLightAllFaces());

	/* TODO: update contents of all subdialogs here*/
}

MainWindow::~MainWindow()
{
	delete engine;
	delete renderer;
}



/***************************************************************************************/
/* FILE MENU */
/***************************************************************************************/

void MainWindow::onModelLoad()
{
	QFileDialog *dlg  = new QFileDialog(this);
	dlg->setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "OBJ files (*.obj)";
	filters << "All files (*.*)";

	dlg->setNameFilters(filters);
	if (!dlg->exec())
		return;

	QString file = dlg->selectedFiles().first();
	engine->loadSceneFromOBJ(file.toStdString().c_str());
	this->setWindowTitle(file);

	drawArea->invalidateScene();
	updateStatus();
}

void MainWindow::onReset()
{
	engine->resetScene();
	drawArea->invalidateScene();
	updateStatus();

}

void MainWindow::onLoadDebugModel()
{
	engine->loadDebugScene();
	drawArea->invalidateScene();
	updateStatus();
}

void MainWindow::onSaveScreenShot()
{
	/* TODO */
}

/***************************************************************************************/
/* VIEW MENU */
/***************************************************************************************/

void MainWindow::onSidePanelShowHide(bool checked)
{
	if (checked)
	{
		sidePanel->show();
	} else
		sidePanel->hide();
}

void MainWindow::onDrawBoundingBox(bool checked)
{
	engine->setDrawBoundingBox(checked);
	drawArea->invalidateScene();
}

void MainWindow::onDrawAxes(bool checked)
{
	engine->setDrawAxes(checked);
	drawArea->invalidateScene();
}

void MainWindow::onDrawNormals(bool checked)
{
	engine->setDrawVertexNormals(checked);
	drawArea->invalidateScene();
}
void MainWindow::onDrawfaceNormals(bool checked)
{
	engine->setDrawPolygonNormals(checked);
	drawArea->invalidateScene();
}
void MainWindow::onDrawWireframe(bool checked)
{
	engine->setDrawWireFrame(checked);
	drawArea->invalidateScene();
}
void MainWindow::onDrawLightSources(bool checked)
{
	engine->setDrawLightSources(checked);
	drawArea->invalidateScene();
}
void MainWindow::onBackFaceCulling(bool checked)
{
	engine->setBackFaceCulling(checked);
	drawArea->invalidateScene();
}

void MainWindow::onDrawDepthbuffer(bool checked)
{
	engine->setDepthRendering(checked);
	drawArea->invalidateScene();
}


/***************************************************************************************/
/* SHADING MENU */
/***************************************************************************************/
void MainWindow::onShadingFlat(bool checked)
{
	if (!checked)
		return;

	engine->setShadingMode(Engine::SHADING_FLAT);
	drawArea->invalidateScene();
}

void MainWindow::onShadingGorald(bool checked)
{
	if (!checked)
		return;

	engine->setShadingMode(Engine::SHADING_GOURAD);
	drawArea->invalidateScene();
}

void MainWindow::onShadingPhong(bool checked)
{
	if (!checked)
		return;

	engine->setShadingMode(Engine::SHADING_PHONG);
	drawArea->invalidateScene();
}

void MainWindow::onInvertNormals(bool checked)
{
	engine->setInvertNormals(checked);
	drawArea->invalidateScene();
}

void MainWindow::onInvertFaces(bool checked)
{
	engine->setInvertFaces(checked);
	drawArea->invalidateScene();
}

void MainWindow::onDualfaceLighting(bool checked)
{
	engine->setLightBackFaces(checked);
	drawArea->invalidateScene();
}

void MainWindow::onAllFaceLighting(bool checked)
{
	engine->setLightAllFaces(checked);
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

void MainWindow::onLeftCoordinateSystem(bool enable)
{
	engine->setInvertDepth(enable);
	drawArea->invalidateScene();
}


/***************************************************************************************/
/* HELP MENU */
/***************************************************************************************/

void MainWindow::onAboutDialog()
{
	(new AboutDialog(this))->show();
}

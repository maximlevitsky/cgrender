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


#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include "ui_mainwindow.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

enum TRANSFORM_MODES {
	TRANSFORM_CAMERA,
	TRANSFORM_OBJECT
};

class MouseSensivetyDialog;
class CameraPropertiesDialog;
class EnvironmentDialog;
class MaterialsDialog;
class Engine;


class MainWindow : public QMainWindow, Ui::MainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();
	Engine* getEngine() { return engine;}

	double movementSensivety;
	double scaleSensivety;
	double rotationSensivety;

private:
	Engine* engine;
	Renderer * renderer;
	QPoint startMousePos;
	TRANSFORM_MODES _transformMode;

private:
	void mouseMoveEvent (QMouseEvent * event);
	void mousePressEvent ( QMouseEvent * event );
	void wheelEvent (QWheelEvent * event );
	void keyPressEvent ( QKeyEvent * event );
	void keyReleaseEvent ( QKeyEvent * event );

public slots:

	/* model menu*/
	void onModelLoad();
	void onSaveScreenShot();
	void onReset();
	void onLoadDebugModel();

	/* scene menu */
	void onCameraPropertiesDialog();
	void onEnvironmentDialog();
	void onMaterialDialog();
	void onLeftCoordinateSystem(bool);

	/* view menu*/
	void onDrawBoundingBox(bool);
	void onDrawAxes(bool);
	void onDrawNormals(bool);
	void onDrawfaceNormals(bool);
	void onDrawWireframe(bool);
	void onDrawLightSources(bool);
	void onBackFaceCulling(bool);
	void onDrawDepthbuffer(bool);

	/* shading menu */
	void onShadingFlat(bool);
	void onShadingGorald(bool);
	void onShadingPhong(bool);
	void onInvertNormals(bool);
	void onInvertFaces(bool);
	void onDualfaceLighting(bool);
	void onAllFaceLighting(bool);



	/* transformation mode*/
	void onCameraTransformMode();
	void onWorldTransformationMode();
	void onSeparateObjectsMode();
	void onTransformationsReset();
	void onMouseSensivetyDialog();


	/* help menu */
	void onAboutDialog();

private:
	/* resident dialogs*/
	MouseSensivetyDialog *mouseSensivetyDialog;
	CameraPropertiesDialog *cameraPropertiesDialog;
	EnvironmentDialog *environmentDialog;
	MaterialsDialog *materialDialog;

	void updateStatus();
};

#endif /* MAINWINDOW_H_ */

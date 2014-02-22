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
class SidePanel;
class Engine;


class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();
	Engine* getEngine() { return engine;}

	void updateStatus();

	double movementSensivety;
	double scaleSensivety;
	double rotationSensivety;
	TRANSFORM_MODES _transformMode;

	bool panelShown;

private:
	Engine* engine;
	Renderer * renderer;

public slots:

	/* model menu*/
	void onModelLoad();
	void onSaveScreenShot();
	void onReset();
	void onLoadDebugModel();

	/* view menu*/
	void onSidePanelShowHide(bool);
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
	void onLeftCoordinateSystem(bool);

	/* help menu */
	void onAboutDialog();

private:
	/* resident dialogs*/
	MouseSensivetyDialog *mouseSensivetyDialog;
	SidePanel *sidePanel;

};

#endif /* MAINWINDOW_H_ */

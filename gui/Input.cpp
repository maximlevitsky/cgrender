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

static double translateSensivety(double value)
{
	double result;

	if (value >= 50)
		result =  1+((double)value - 50) / 5;
	else
		result =  1.0 / (1.0 + (50-(value-1))/5);

	return result;
}


/***************************************************************************************/
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
	QPoint pos = event->pos() - drawArea->pos();
	QPoint screenDist = pos - startMousePos;

	if (event->buttons() & Qt::RightButton)
	{
		Vector3 dist =
				engine->deviceToNDC(pos.x(), 0, 0) -
				engine->deviceToNDC(startMousePos.x(), 0, 0);

		switch(_transformMode)
		{
		case TRANSFORM_CAMERA:
			switch(QApplication::keyboardModifiers()) {
			case Qt::ShiftModifier:
				engine->moveCamera(2, dist.x() * translateSensivety(movementSensivety));
				break;
			default:
				engine->rotateCamera(2, screenDist.x() * translateSensivety(rotationSensivety));
				break;
			}
			break;
		case TRANSFORM_OBJECT:
			switch(QApplication::keyboardModifiers()) {
			case Qt::ShiftModifier:
				engine->moveObject(2, dist.x() * translateSensivety(movementSensivety));
				break;
			case Qt::ControlModifier:
				engine->scaleObject(2, dist.x() * translateSensivety(scaleSensivety));
				break;
			default:
				engine->rotateObject(2, screenDist.x()  * translateSensivety(rotationSensivety));
				break;
			}
			break;
		}

	} else
	{
		Vector3 dist =
				engine->deviceToNDC(pos.x(), pos.y(), 0) -
				engine->deviceToNDC(startMousePos.x(), startMousePos.y(), 0);

		switch(_transformMode)
		{
		case TRANSFORM_CAMERA:
			switch(QApplication::keyboardModifiers()) {
			case Qt::ShiftModifier:
				engine->moveCamera(0, dist.x() * translateSensivety(movementSensivety));
				engine->moveCamera(1, dist.y() * translateSensivety(movementSensivety));
				break;
			default:
				engine->rotateCamera(0, screenDist.y() * translateSensivety(rotationSensivety));
				engine->rotateCamera(1, -screenDist.x() * translateSensivety(rotationSensivety));
				break;
			}
			break;
		case TRANSFORM_OBJECT:
			switch(QApplication::keyboardModifiers()) {
			case Qt::ShiftModifier:
				engine->moveObject(0, dist.x() * translateSensivety(movementSensivety));
				engine->moveObject(1, dist.y() * translateSensivety(movementSensivety));
				break;
			case Qt::ControlModifier:
				engine->scaleObject(0, dist.x() * translateSensivety(scaleSensivety));
				engine->scaleObject(1, dist.y() * translateSensivety(scaleSensivety));
				break;
			default:
				engine->rotateObject(0, screenDist.y()  * translateSensivety(rotationSensivety));
				engine->rotateObject(1, -screenDist.x() * translateSensivety(rotationSensivety));
				break;
			}
			break;
		}

	}


	startMousePos = pos;
	drawArea->invalidateScene();

}

/***************************************************************************************/

void MainWindow::wheelEvent (QWheelEvent * event )
{
	int degrees = event->delta() / 8;

	Vector3 p1 = engine->deviceToNDC(0, 0, 0);
	Vector3 p2 = engine->deviceToNDC(0, 0, 0.05*(degrees));
	double scaleDist = p1.z() - p2.z();

	if (_transformMode == TRANSFORM_OBJECT) {
		engine->scaleObject(0, scaleDist * translateSensivety(scaleSensivety));
		engine->scaleObject(1, scaleDist * translateSensivety(scaleSensivety));
		engine->scaleObject(2, scaleDist * translateSensivety(scaleSensivety));
	}

	drawArea->invalidateScene();

}

/***************************************************************************************/


void MainWindow::mousePressEvent(QMouseEvent* event)
{
	startMousePos = event->pos() - drawArea->pos();

	if (engine->getDrawSeparateObjects() && (event->modifiers() == 0))
	{
		printf("selecting object at %i, %i\n", startMousePos.x(), startMousePos.y());

		if (engine->selectObject(startMousePos.x(), startMousePos.y())) {
			printf("selected some object...");
			drawArea->invalidateScene();
		}
	}
}

/***************************************************************************************/

void MainWindow::keyPressEvent ( QKeyEvent * event )
{
	switch (event->modifiers()) {
	case Qt::ShiftModifier:
		/* move */
		setCursor(Qt::ClosedHandCursor);
		break;
	case Qt::ControlModifier:
		/* resize */
		setCursor(Qt::SizeVerCursor);
		break;
	default:
		setCursor(Qt::ArrowCursor);
		break;
	}
}

/***************************************************************************************/

void MainWindow::keyReleaseEvent ( QKeyEvent * event )
{
	setCursor(Qt::ArrowCursor);
}

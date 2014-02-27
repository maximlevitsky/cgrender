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

#include <DrawArea.h>
#include <QPainter>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QString>
#include <sys/time.h>
#include <stdint.h>
#include "MainWindow.h"
#include <stdio.h>

#include <cmath>

/***************************************************************************************/
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

DrawArea::DrawArea(QWidget *parent) : QWidget(parent), _image(NULL), _outputTexture(NULL)
{
	mainWindow = dynamic_cast<MainWindow*>(parent->parent());
	engine = mainWindow->getEngine();
	sceneValid = false;
	renderingSuspended = false;

	setFocusPolicy(Qt::ClickFocus);
}
/***************************************************************************************/

DrawArea::~DrawArea()
{
	delete _outputTexture;
	delete _image;
}

/***************************************************************************************/

void DrawArea::invalidateScene(bool force)
{
	if (renderingSuspended && !force)
		return;

	sceneValid = false;
	repaint();
}

/***************************************************************************************/

void DrawArea::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	if (_outputTexture == NULL)
	 return;


	// make engine render
	if (!sceneValid) {

		// start measuring time
		struct timeval t,t2,t3;
		gettimeofday(&t, NULL);

		Color bkgcolor = engine->getBackgroundSettings().color * 255;

		engine->render();
		sceneValid = true;

		// measure time again
		gettimeofday(&t2, NULL);
		timersub(&t2,&t,&t3);
		int msec = t3.tv_sec * 1000 + t3.tv_usec / 1000;

		// draw time report
		QPainter imgpainter(_image);
		int w = this->geometry().width();
		QRect textRect(QPoint(w - 220, 0), QPoint(w,20));
		imgpainter.fillRect(textRect, QBrush(QColor(bkgcolor[0],bkgcolor[1],bkgcolor[2])));
		imgpainter.setPen(QColor(255,255,255));
		imgpainter.drawText(textRect.bottomLeft(),
		 QString("Rendering took %1 msec (%2 FPS)").
		 arg(QString::number(msec), QString::number(1000.0/msec)));

		//printf("Rendering took %i msec\n", msec);
	}

	// blit the _image
	painter.drawImage(QPoint(0,0), *_image);
}

/***************************************************************************************/

void DrawArea::resizeEvent (QResizeEvent * event)
{
	QSize newSize = event->size();

	// nothing to do if pixmap is of enough size
	if (!_outputTexture || _outputTexture->getHeight() < newSize.height()
			||_outputTexture->getWidth() < newSize.width()) {

		delete _image;
		delete _outputTexture;

		// create new output texture
		_outputTexture = new Texture(newSize.width(), newSize.height());

		// create image based on this texture and use it to init it (why not...)
		_image = new QImage(
				(uchar*)_outputTexture->getPointer(),
				_outputTexture->getWidth(),
				_outputTexture->getHeight(),
				_outputTexture->getWidth() * sizeof(DEVICE_PIXEL),
				QImage::Format_RGB32
		);

		_image->fill(QColor(qRgb(20,20,20)));
	}


	mainWindow->getEngine()->setOutput(_outputTexture, newSize.width(), newSize.height());
	invalidateScene(true);
}


/***************************************************************************************/
void DrawArea::mouseMoveEvent(QMouseEvent* event)
{
	QPoint pos = event->pos();
	QPoint screenDist = pos - startMousePos;

	if (renderingSuspended)
		return;

	Vector3 dist =
			engine->deviceToNDC(pos.x(), pos.y()) -
			engine->deviceToNDC(startMousePos.x(), startMousePos.y());

	switch(mainWindow->_transformMode)
	{
	case TRANSFORM_CAMERA:
		switch(QApplication::keyboardModifiers()) {
		case Qt::ShiftModifier:
			engine->moveCamera(0, dist.x() * translateSensivety(mainWindow->movementSensivety));
			engine->moveCamera(1, dist.y() * translateSensivety(mainWindow->movementSensivety));
			break;
		default:
			engine->rotateCamera(0, screenDist.y() * translateSensivety(mainWindow->rotationSensivety));
			engine->rotateCamera(1, -screenDist.x() * translateSensivety(mainWindow->rotationSensivety));
			break;
		}
		break;
	case TRANSFORM_OBJECT:
		switch(QApplication::keyboardModifiers()) {
		case Qt::ShiftModifier:
			engine->moveObject(0, dist.x() * translateSensivety(mainWindow->movementSensivety));
			engine->moveObject(1, dist.y() * translateSensivety(mainWindow->movementSensivety));
			break;
		case Qt::ControlModifier:
			engine->scaleObject(0, dist.x() * translateSensivety(mainWindow->scaleSensivety));
			engine->scaleObject(1, dist.y() * translateSensivety(mainWindow->scaleSensivety));
			break;
		default:
			engine->rotateObject(0, screenDist.y()  * translateSensivety(mainWindow->rotationSensivety));
			engine->rotateObject(1, -screenDist.x() * translateSensivety(mainWindow->rotationSensivety));
			break;
		}
		break;
	}



	startMousePos = pos;
	invalidateScene();

}

/***************************************************************************************/

void DrawArea::wheelEvent (QWheelEvent * event )
{
	int degrees = event->delta() / 8;
	double rotdist = degrees / 4;

	if (renderingSuspended)
		return;

	double dist = engine->getZStep() * degrees / 4;

	switch(mainWindow->_transformMode)
	{
	case TRANSFORM_CAMERA:
		switch(QApplication::keyboardModifiers()) {
		case Qt::ShiftModifier:
			engine->moveCamera(2, dist * translateSensivety(mainWindow->movementSensivety));
			break;
		default:
			engine->rotateCamera(2, rotdist * translateSensivety(mainWindow->rotationSensivety));
			break;
		}
		break;
	case TRANSFORM_OBJECT:
		switch(QApplication::keyboardModifiers()) {
		case Qt::ShiftModifier:
			engine->moveObject(2, dist * translateSensivety(mainWindow->movementSensivety));
			break;
		case Qt::ControlModifier:
			engine->scaleObject(0, dist * translateSensivety(mainWindow->scaleSensivety));
			engine->scaleObject(1, dist * translateSensivety(mainWindow->scaleSensivety));
			engine->scaleObject(2, dist * translateSensivety(mainWindow->scaleSensivety));
			break;
		default:
			engine->rotateObject(2, rotdist  * translateSensivety(mainWindow->rotationSensivety));
			break;
		}
		break;
	}

	invalidateScene();
	engine->commitRotation();

}

/***************************************************************************************/


void DrawArea::mousePressEvent(QMouseEvent* event)
{

	if (renderingSuspended)
		return;

	startMousePos = event->pos();

	if (engine->getDrawSeparateObjects() && (event->modifiers() == 0))
	{
		printf("selecting object at %i, %i\n", startMousePos.x(), startMousePos.y());

		if (engine->selectObject(startMousePos.x(), startMousePos.y())) {
			printf("selected some object...");
			invalidateScene();
		}
	}
}

void DrawArea::mouseReleaseEvent(QMouseEvent * event)
{
	engine->commitRotation();
}

/***************************************************************************************/

void DrawArea::keyPressEvent ( QKeyEvent * event )
{
	if (renderingSuspended)
		return;

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

void DrawArea::keyReleaseEvent ( QKeyEvent * event )
{
	setCursor(Qt::ArrowCursor);
}

void DrawArea::enterEvent(QEvent * event)
{
	setFocus();
}


void DrawArea::suspendRendering(bool suspend)
{
	renderingSuspended = suspend;
	if (!renderingSuspended)
		invalidateScene();
}


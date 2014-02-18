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
#include <math.h>
#include <QPainter>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QString>

#include <sys/time.h>
#include <stdint.h>

#include "MainWindow.h"

#include <stdio.h>

DrawArea::DrawArea(QMainWindow *parent) : QWidget(parent), _image(NULL), _outputTexture(NULL)
{
	mainWindow = dynamic_cast<MainWindow*>(parent);
	sceneValid = false;
}

DrawArea::~DrawArea()
{
	delete _outputTexture;
	delete _image;
}


void DrawArea::invalidateScene()
{
	sceneValid = false;
	repaint();
}


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

		Color bkgcolor = mainWindow->getEngine()->getBackgroundColor();


		mainWindow->getEngine()->render();
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
	invalidateScene();
}

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

#ifndef DRAWAREA_H_
#define DRAWAREA_H_

#include <QtGui/qwidget.h>
#include <stdint.h>
#include <QMainWindow>

#include "engine/Engine.h"
#include "renderer/Renderer.h"
#include "renderer/Texture.h"

class MainWindow;

class DrawArea: public QWidget
{
public:
	DrawArea(QMainWindow *parent);
	virtual ~DrawArea();

	void invalidateScene();
private:
	// events for painting
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent * event);

private:
	// data
	QImage *_image;
	Texture *_outputTexture;
	MainWindow *mainWindow;


	bool sceneValid;
};

#endif /* DRAWAREA_H_ */

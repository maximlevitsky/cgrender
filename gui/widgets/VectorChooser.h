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

#ifndef VECTORCHOOSER_H_
#define VECTORCHOOSER_H_

#include "ui_vector_chooser.h"
#include <QWidget>

class VectorChooser : public QWidget, Ui::VectorChooser
{
	Q_OBJECT
public:
	VectorChooser(QWidget *parent);
	void showAngleHelpBoxes(bool show);
};

#endif
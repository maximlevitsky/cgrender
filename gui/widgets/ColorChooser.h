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

#ifndef COLORCHOOSER_H_
#define COLORCHOOSER_H_

#include <qwidget.h>
#include "ui_ColorChooser.h"

#include "common/Vector3.h"

class ColorChooser: public QWidget, Ui_ColorChooser
{
	Q_OBJECT
public:
	ColorChooser(QWidget* parent);

	/* IO */
	Color getColor() { return value; }
	void setColor(Color c) { value = c ; updateUI(); }
	void setDefaultValue(Color c) { defaultValue = c;}

private slots:
	void onSpinBoxValueChanged();
	void onColorChooseCalled();
	void onColorResetCalled();

signals:
	void contentsChanged();

private:
	Color value;
	Color defaultValue;
	void updateUI();
};

#endif /* COLORCHOOSER_H_ */

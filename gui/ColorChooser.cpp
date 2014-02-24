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

#include <ColorChooser.h>
#include <QLineEdit>
#include <QColorDialog>

ColorChooser::ColorChooser(QWidget* parent) : QWidget(parent)
{
	setupUi(this);
	connect(redBox, SIGNAL(valueChanged(double)), this, SLOT(onSpinBoxValueChanged()), Qt::QueuedConnection);
	connect(greenBox, SIGNAL(valueChanged(double)), this, SLOT(onSpinBoxValueChanged()), Qt::QueuedConnection);
	connect(blueBox, SIGNAL(valueChanged(double)), this, SLOT(onSpinBoxValueChanged()), Qt::QueuedConnection);

	connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(onColorResetCalled()));
	connect(chooseButton, SIGNAL(clicked(bool)), this, SLOT(onColorChooseCalled()));
	updateUI();
}


void ColorChooser::onSpinBoxValueChanged()
{
	/* read new values from the user */
	value[0] = redBox->value();
	value[1] = greenBox->value();
	value[2] = blueBox->value();

	updateUI();
	emit contentsChanged();
}


void ColorChooser::onColorChooseCalled()
{
	QColor c = qRgb(round(value[0]*255),round(value[1]*255),round(value[2]*255));
	c = QColorDialog::getColor(c, this);

	value[0] = (double)c.red() / 255;
	value[1] = (double)c.green() / 255;
	value[2] = (double)c.blue() / 255;
	updateUI();
	emit contentsChanged();
}

void ColorChooser::onColorResetCalled()
{
	value = defaultValue;
	updateUI();
	emit contentsChanged();
}



void ColorChooser::updateUI()
{
	QPalette palette;
	QColor c = qRgb(round(value[0]*255),round(value[1]*255),round(value[2]*255));
	int textColor = (c.red() + c.green() + c.blue()) / 3 > 128 ? 0 : 255;

	palette.setColor(QPalette::Window,c);
	colorArea->setPalette(palette);

	QString stylesheet;
	stylesheet.sprintf("background-color: rgb(%d,%d,%d);\n color: rgb(%d,%d,%d);",
			c.red(),c.green(),c.blue(), textColor, textColor, textColor);

	redBox->setStyleSheet(stylesheet);
	greenBox->setStyleSheet(stylesheet);
	blueBox->setStyleSheet(stylesheet);

	redBox->setValue(value[0]);
	greenBox->setValue(value[1]);
	blueBox->setValue(value[2]);

}


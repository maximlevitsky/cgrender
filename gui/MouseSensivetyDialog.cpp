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

#include "MouseSensivetyDialog.h"
#include "MainWindow.h"

#include <QPushButton>


MouseSensivetyDialog::MouseSensivetyDialog(MainWindow *parent) : QDialog(parent)
{
	mainWindow = parent;

	setupUi(this);
	rotationSensivetySlider->setRange(0,100);
	rotationSensivetySlider->setSliderPosition(mainWindow->rotationSensivety);

	movementSensivetySlider->setRange(0,100);
	movementSensivetySlider->setSliderPosition(mainWindow->movementSensivety);

	scaleSensivetySlider->setRange(0,100);
	scaleSensivetySlider->setSliderPosition(mainWindow->scaleSensivety);

	connect(rotationSensivetySlider, SIGNAL(sliderMoved(int)), this, SLOT(onDialogUpdate()));
	connect(movementSensivetySlider, SIGNAL(sliderMoved(int)), this, SLOT(onDialogUpdate()));
	connect(scaleSensivetySlider, SIGNAL(sliderMoved(int)), this, SLOT(onDialogUpdate()));

	QPushButton *resetButton = buttonBox->button(QDialogButtonBox::RestoreDefaults);
	connect(resetButton, SIGNAL(clicked()), this, SLOT(onReset()));
}

MouseSensivetyDialog::~MouseSensivetyDialog()
{
	// TODO Auto-generated destructor stub
}


void MouseSensivetyDialog::onDialogUpdate()
{
	mainWindow->rotationSensivety = rotationSensivetySlider->value();
	mainWindow->movementSensivety = movementSensivetySlider->value();
	mainWindow->scaleSensivety = scaleSensivetySlider->value();
}


void MouseSensivetyDialog::onReset() {
	rotationSensivetySlider->setValue(50);
	movementSensivetySlider->setValue(50);
	scaleSensivetySlider->setValue(50);
	onDialogUpdate();
}

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

#include "SidePanel.h"
#include "MainWindow.h"
#include "engine/Engine.h"

SidePanel::SidePanel(MainWindow* parent) : QDockWidget(parent)
{
	mainWindow = parent;
	engine = mainWindow->getEngine();

	panel = new QWidget(this);
	setupUi(panel);
	setWidget(panel);
	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
	setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	setWidget(panel);
	lightDirectionChooser->showAngleHelpBoxes(true);
	cameraDirectionChooser->showAngleHelpBoxes(true);


	/* setup general tab/fog */
	connect(fogModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(fogPanelReadControls()));
	connect(fogStartDepthBox, SIGNAL(valueChanged(double)), this, SLOT(fogPanelReadControls()));
	connect(fogEndDepthBox, SIGNAL(valueChanged(double)), this, SLOT(fogPanelReadControls()));
	connect(fogDensityBox, SIGNAL(valueChanged(double)), this, SLOT(fogPanelReadControls()));
	connect(fogColorChooser, SIGNAL(contentsChanged()), this, SLOT(fogPanelReadControls()));
	connect(fogResetButton, SIGNAL(clicked(bool)), this, SLOT(fogReset()));
	FogParams params = engine->getFogParams();
	fogColorChooser->setDefaultValue(params.color);
	fogPanelWriteControls();

	/* setup general tab/background */
	backgroundTextureChooser->setDefaultFilename("");
	backgroundColorChooser->setDefaultValue(engine->getBackgroundSettings().color);
	backgroundPanelWriteControls();


	connect(backgroundColorChooser, SIGNAL(contentsChanged()), this, SLOT(backgroundPanelReadControls()));
	connect(backgroundTextureChooser, SIGNAL(contentsChanged()), this, SLOT(backgroundPanelReadControls()));
	connect(backgroundScalingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundPanelReadControls()));
	connect(backgroundResetButton, SIGNAL(clicked(bool)), this, SLOT(backgroundReset()));


	/* bottom bar*/
	connect(pauseRenderingButton, SIGNAL(clicked(bool)), this, SLOT(onSuspendRendering(bool)));
	connect(fastRenderingModeButton, SIGNAL(clicked(bool)), this, SLOT(onFastRendering(bool)));
}


/******************************************************************************************/
/* GENERAL TAB / FOG */
/******************************************************************************************/

void SidePanel::fogPanelReadControls()
{
	/* update program state when user changes something in fog control panel */
	int mode = fogModeComboBox->currentIndex();

	/* update engine*/
	FogParams params = engine->getFogParams();
	params.enabled = mode != 0;

	if (params.enabled)
		params.type = (FogParams::FogType)(mode - 1);

	params.startPoint = fogStartDepthBox->value();
	params.endPoint = fogEndDepthBox->value();
	params.density = fogDensityBox->value();
	params.color = fogColorChooser->getColor();
	engine->setFogParams(params);

	/* update visable controls */
	switch (fogModeComboBox->currentIndex())
	{
	case 0:
		/*disabled*/
		linearFogArea->hide();
		exponentialFogArea->hide();
		fogColorArea->hide();
		break;
	case 1:
		/* linear*/
		exponentialFogArea->hide();
		linearFogArea->show();
		fogColorArea->show();
		break;
	case 2:
	case 3:
		/* exponent / quard exponent*/
		linearFogArea->hide();
		exponentialFogArea->show();
		fogColorArea->show();
		break;
	default:
		assert(0);
	}

	mainWindow->drawArea->invalidateScene();
}

void SidePanel::fogPanelWriteControls()
{
	FogParams params = engine->getFogParams();

	if (params.enabled == false)
		fogModeComboBox->setCurrentIndex(0);
	else
		fogModeComboBox->setCurrentIndex(params.type + 1);

	fogStartDepthBox->setValue(params.startPoint);
	fogEndDepthBox->setValue(params.endPoint);
	fogDensityBox->setValue(params.density);
	fogColorChooser->setColor(params.color);
}

void SidePanel::fogReset()
{
	FogParams params = engine->getFogParams();
	params.reset();
	engine->setFogParams(params);
	fogPanelWriteControls();
	mainWindow->drawArea->invalidateScene();
}

/******************************************************************************************/
/* GENERAL TAB / BACKGROUND */
/******************************************************************************************/
void SidePanel::backgroundPanelReadControls()
{
	BackgroundParams params = engine->getBackgroundSettings();
	params.textureFile = backgroundTextureChooser->getFileName().toStdString();
	params.color = backgroundColorChooser->getColor();
	params.mode = backgroundTextureChooser->getFileName().isEmpty() ? BackgroundParams::COLOR : BackgroundParams::TEXTURE;
	params.textureScalingMode = (BackgroundParams::ScalingMode) backgroundScalingMode->currentIndex();
	engine->setBackGroundSettings(params);
	fogColorChooser->setDefaultValue(params.color);
	mainWindow->drawArea->invalidateScene();
}

void SidePanel::backgroundPanelWriteControls()
{
	BackgroundParams params = engine->getBackgroundSettings();
	backgroundTextureChooser->setFileName(QString::fromStdString(params.textureFile));
	backgroundColorChooser->setColor(params.color);
	backgroundScalingMode->setCurrentIndex(params.textureScalingMode);
}

void SidePanel::backgroundReset()
{
	engine->resetBackground();
	backgroundPanelWriteControls();
	mainWindow->drawArea->invalidateScene();
}



/***********************************************************************************/
/* BOTTOM BAR */
/***********************************************************************************/

void SidePanel::onSuspendRendering(bool checked)
{
	mainWindow->drawArea->suspendRendering(checked);
}

void SidePanel::onFastRendering(bool checked)
{
	/* set fast rendering flag for the engine*/
	mainWindow->drawArea->invalidateScene();
}


SidePanel::~SidePanel()
{
}

void SidePanel::closeEvent (QCloseEvent *event)
{
	mainWindow->actionSidePanel->setChecked(false);
}

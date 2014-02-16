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
#include "Material.h"

MaterialParams::MaterialParams() :
		base(NULL), _default(NULL) {
	reset();
}

void MaterialParams::setBase(MaterialParams* newbase)
{
	base = newbase;
}

void MaterialParams::setDefault(MaterialParams* newdefault)
{
	_default = newdefault;
}

void MaterialParams::reset()
{
	lightingValid = false;
	ambient = 0.2;
	diffuse = 0.5;
	spectular = 0.2;
	shineness = 32;
	resetObjectColor();
	resetTexture();
}

double MaterialParams::getAmbient() const
{
	if (lightingValid)
		return ambient;

	if (base && base->lightingValid)
		return base->ambient;

	if (_default && _default->lightingValid)
		return _default->ambient;

	return ambient;
}

double MaterialParams::getDiffuse() const
{
	if (lightingValid)
		return diffuse;

	if (base && base->lightingValid)
		return base->diffuse;

	if (_default && _default->lightingValid)
		return _default->diffuse;

	return diffuse;
}

double MaterialParams::getSpecular() const
{
	if (lightingValid)
		return spectular;

	if (base && base->lightingValid)
		return base->spectular;

	if (_default && _default->lightingValid)
		return _default->spectular;

	return spectular;
}

int MaterialParams::getShineness() const
{
	if (lightingValid)
		return shineness;

	if (base && base->lightingValid)
		return base->shineness;

	if (_default && _default->lightingValid)
		return _default->shineness;

	return shineness;
}

void MaterialParams::setAmbient(double value)
{
	ambient = clamp(value, 0.0, 1.0);
	lightingValid = true;
}

void MaterialParams::setDiffuse(double value)
{
	diffuse = clamp(value, 0.0, 1.0);
	lightingValid = true;
}

void MaterialParams::setSpecular(double value)
{
	spectular = clamp(value, 0.0, 1.0);
	lightingValid = true;
}

void MaterialParams::setShinenes(int value) {
	shineness = max(value, 0);
	lightingValid = true;
}

Color MaterialParams::getObjectColor() const
{
	if (colorValid)
		return objectColor;

	if (base && base->colorValid)
		return base->objectColor;

	if (_default && _default->colorValid)
		return _default->objectColor;

	return objectColor;
}

void MaterialParams::setObjectColor(Color color)
{
	objectColor = color;
	colorValid = true;
}

void MaterialParams::resetObjectColor() {
	colorValid = false;
	objectColor = Color(255, 255, 255);
}

std::string MaterialParams::getObjectTexture() const
{
	std::string empty;
	if (textureValid)
		return objectTexture;

	if (base && base->textureValid)
		return base->objectTexture;

	if (_default && _default->textureValid)
		return _default->objectTexture;

	return empty;
}

void MaterialParams::setObjectTexture(const std::string str, int scaleX,int scaleY)
{
	objectTexture = str;
	textureValid = true;
	_scaleX = scaleX;
	_scaleY = scaleY;
}

int MaterialParams::getscaleX()
{
	if (textureValid)
		return _scaleX;

	if (base && base->textureValid)
		return base->_scaleX;

	if (_default && _default->textureValid)
		return _default->_scaleX;

	return 1;
}

int MaterialParams::getscaleY()
{
	if (textureValid)
		return _scaleY;

	if (base && base->textureValid)
		return base->_scaleY;

	if (_default && _default->textureValid)
		return _default->_scaleY;

	return 1;
}

void MaterialParams::resetTexture()
{
	objectTexture.clear();
	_scaleX = 1;
	_scaleY = 1;
	textureValid = false;
}

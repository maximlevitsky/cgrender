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
#ifndef MATERIALS_H
#define MATERIALS_H

#include "common/Vector4.h"
#include <string>


struct MaterialParams
{
	MaterialParams();

	void setBase(MaterialParams* newbase);
	void setDefault(MaterialParams* newdefault);

	void reset();


	/* lighting settings */
	double getAmbient() const;
	double getDiffuse() const;
	double getSpecular() const;
	int getShineness() const;
	void setAmbient(double value);
	void setDiffuse(double value);
	void setSpecular(double value);
	void setShinenes(int value);

	/* object color */
	Color getObjectColor() const;
	void setObjectColor(Color color);
	void resetObjectColor();

	/* object texture */
	std::string getObjectTexture() const;
	void setObjectTexture(const std::string str, int scaleX, int scaleY);
	int getscaleX();
	int getscaleY();
	void resetTexture();

private:
	// lighting params
	double ambient;
	double diffuse;
	double spectular;
	int shineness;
	bool lightingValid;

	// object color
	Color objectColor;
	bool colorValid;

	// object texture
	std::string objectTexture;
	int _scaleX;
	int _scaleY;

	bool textureValid;
	MaterialParams *base;
	MaterialParams *_default;
};



#endif

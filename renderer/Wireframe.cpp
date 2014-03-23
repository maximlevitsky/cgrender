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
#include "Renderer.h"
#include "Texture.h"
#include "Samplers.h"

#include "common/Mat4.h"
#include "common/Vector4.h"
#include "common/Vector3.h"

#include <assert.h>

void Renderer::renderWireFrame( unsigned int* geometry, int count, Color c, bool colorValid )
{

	for (polygonIterator iter(geometry, count); iter.hasmore() ; iter.next())
	{
#if 0
		if ((_backFaceCulling || _frontFaceCulling) && iter.vertexCount() >= 3)
		{
			bool backface = determineBackface(iter);
			if ((backface && _backFaceCulling) || (!backface && _frontFaceCulling))
				continue;
		}

		for (int j=0, k=1; j<iter.vertexCount(); ++j, ++k)
		{
			int indexOfVertex1 = iter[j];
			int indexOfVertex2 = k == iter.vertexCount() ? iter[0] : iter[k];

			const Vector4 &v1 = _vertexBuffer.position(indexOfVertex1);
			const Vector4 &v2 = _vertexBuffer.position(indexOfVertex2);

			if (v1.w() > 0 && v2.w() > 0 && !fastClipLine(v1, v2))
				drawLine(NDC_to_DeviceSpace(&v1), NDC_to_DeviceSpace(&v2),
						colorValid ? c : _vertexBuffer[indexOfVertex1][0]);
		}
#endif
	}

}



void Renderer::drawLine( Vector4 p1, Vector4 p2, const Color &c )
{
	if (p1.isBadFP() || p2.isBadFP())
		return;

	int x1 = (int)p1.x(), x2 = (int)p2.x();
	int y1 = (int)p1.y(), y2 = (int)p2.y();

	// add small bias to Z so that wireframe is rendered above the model
	double z1 = p1.z() - 0.05, z2 = p2.z() - 0.05;

    int dx = (int)abs(x2 - x1);
	int dy = (int)abs(y2 - y1);
	double dz = (z2 - z1) / max(dx,dy);

	int sx = x1 < x2 ? 1 : -1;
	int sy = y1 < y2 ? 1 : -1;
	int d = dx - dy;

	while (1)
	{
		if (x1 >= 0 && x1 < _viewportSizeX && y1 >= 0 && y1 < _viewportSizeY
			&& (!_zBuffer || _zBuffer->zTest(x1,y1, z1)))
		{
			if (_zBuffer) _zBuffer->setPixelValue(x1,y1,z1);
			drawPixel(x1, y1, c);
		}

		if (x1 == x2 && y1 == y2)
			break;

		z1 += dz;
		int e2 = 2 * d;

		if (e2 > -dy) {
			d -= dy;
			x1 += sx;
		}

		if (x1 == x2 && y1 == y2)
			continue;

		if (e2 < dx) {
			d += dx;
			y1 += sy;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

bool Renderer::fastClipLine(const Vector4& v1, const Vector4 &v2)
{
	if ((v1.x() > v1.w() * clip_x) && (v2.x() > v2.w() * clip_x) )
		return true;

	if ((v1.x() < -v1.w() * clip_x) && (v2.x() < -v2.w() * clip_x) )
		return true;

	if ((v1.y() > v1.w() * clip_y) && (v2.y() > v2.w() * clip_y))
		return true;

	if ((v1.y() < -v1.w() * clip_y) && (v2.y() < -v2.w() * clip_y))
		return true;

	return false;
}

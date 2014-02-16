// ColorLabel.cpp : implementation file
//

/*
	This file is part of CG1.
    CG1 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CG1 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CG4.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "ColorLabel.h"


// CColorLabel

IMPLEMENT_DYNAMIC(CColorLabel, CStatic)

CColorLabel::CColorLabel()
{
	m_brush = new CBrush(RGB(0,0,0));
}

CColorLabel::~CColorLabel()
{
	delete m_brush;
}


BEGIN_MESSAGE_MAP(CColorLabel, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


HBRUSH CColorLabel::CtlColor(CDC* pDC, UINT nCtlColor)
{
	return *m_brush;
}


void CColorLabel::setColor(COLORREF color)
{
	delete m_brush;
	m_brush = new CBrush(color);
	Invalidate();
}

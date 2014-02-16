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
#pragma once

#include "StdAfx.h"
#include "Common\Vector4.h"

// CColorLabel

class CColorLabel : public CStatic
{
	DECLARE_DYNAMIC(CColorLabel)

public:
	CColorLabel();
	virtual ~CColorLabel();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);

private:
	CBrush* m_brush;
public:
	void setColor(COLORREF color);
};


static Color chooseColor(Color& current, CColorLabel* label)
{
	CColorDialog cdlg(RGB(current[0], current[1], current[2]), CC_FULLOPEN | CC_ANYCOLOR);

	if (cdlg.DoModal() != IDOK)
		return current;

	COLORREF newColor = cdlg.GetColor();
	label->setColor(newColor);
	current = Color(GetRValue(newColor), GetGValue(newColor), GetBValue(newColor));
	return current;
}
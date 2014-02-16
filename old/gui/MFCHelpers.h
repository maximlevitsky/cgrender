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

#pragma  once
#include "ColorLabel.h"
#include "Common\Vector4.h"
#include "StdAfx.h"


static void textBoxSetFixedPoint(CEdit & textbox, int value, int fraction = 10 )
{
	CString s;
	s.Format("%i.%i", value / fraction, value % fraction);
	textbox.SetWindowText(s);
}

static double textBoxGetDouble( CEdit &textbox )
{
	CString s;
	textbox.GetWindowText(s);
	return atof(s);
}

static void textBoxSetDouble(CEdit &textbox, double value)
{
	CString s;
	s.Format("%f", value);
	textbox.SetWindowText(s);

}

static int textBoxGetInt(CEdit &textbox) 
{
	CString s;
	textbox.GetWindowText(s);
	return strtol(s, NULL, 10);
}

static void textBoxSetInt(CEdit &textbox, int value) 
{
	CString s;
	s.Format("%i", value);
	textbox.SetWindowText(s);
}

static void spinHandler(CWnd* w, NMHDR *pNMHDR, LRESULT *pResult, double step)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CSpinButtonCtrl *ctl = (CSpinButtonCtrl *)w->GetDlgItem(pNMUpDown->hdr.idFrom);
	CEdit *textBox = (CEdit*)ctl->GetBuddy();

	double value = textBoxGetDouble(*textBox);
	value += step * pNMUpDown->iDelta;
	textBoxSetDouble(*textBox,value);
	*pResult = 0;
}
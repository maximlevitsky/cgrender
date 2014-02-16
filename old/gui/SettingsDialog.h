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
#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "afxdialogex.h"
#include "colorlabel.h"
#include "CG4View.h"

class CCG4View;

class CSettingsDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsDialog);

public:
	CSettingsDialog(CCG4View* pParent);   // standard constructor
	virtual ~CSettingsDialog() {}

// Dialog Data
	enum { IDD = IDD_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEnUpdateDistanceTextbox();
	afx_msg void OnEnChangeFinessTextbox();
	afx_msg void OnBnClickedFinessApply();
	afx_msg void OnBnClickedSensivetyReset();



	CCG4View* mainView;

	// controls
	CSliderCtrl d_slider;
	CEdit distance_textbox;
	CEdit fov_textbox;
	CSliderCtrl mouse_rotation_sensivety_slider;
	CSliderCtrl mouse_translation_sensivety_slider;
	CSliderCtrl mouse_scaling_sensivety_slider;
	CSliderCtrl fineness_slider;
	CEdit fineness_textbox;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeNormalsScaleTextbox();
	CEdit txt_normal_scale;
};

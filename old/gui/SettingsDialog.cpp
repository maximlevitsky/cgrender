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

#include "stdafx.h"
#include "CG4.h"
#include "SettingsDialog.h"
#include "CG4View.h"
#include "MFCHelpers.h"

IMPLEMENT_DYNAMIC(CSettingsDialog, CDialogEx)

BEGIN_MESSAGE_MAP(CSettingsDialog, CDialogEx)
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_FINESS_APPLY, &CSettingsDialog::OnBnClickedFinessApply)
	ON_EN_UPDATE(IDC_DISTANCE_TEXTBOX, &CSettingsDialog::OnEnUpdateDistanceTextbox)
	ON_EN_CHANGE(IDC_FINESS_TEXTBOX, &CSettingsDialog::OnEnChangeFinessTextbox)
	ON_BN_CLICKED(IDC_SENSIVETY_RESET, &CSettingsDialog::OnBnClickedSensivetyReset)
	ON_EN_CHANGE(IDC_NORMALS_SCALE_TEXTBOX, &CSettingsDialog::OnEnChangeNormalsScaleTextbox)
END_MESSAGE_MAP()


CSettingsDialog::CSettingsDialog(CCG4View* pParent)
	: CDialogEx(CSettingsDialog::IDD, CWnd::GetDesktopWindow())
{
	mainView = pParent;
}

int CSettingsDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;


}

BOOL CSettingsDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	d_slider.SetRangeMin(0);
	d_slider.SetRangeMax(50 * 10);
	d_slider.SetTicFreq(10);


	fineness_slider.SetRangeMax(50);
	fineness_slider.SetRangeMin(2);

	mouse_rotation_sensivety_slider.SetRangeMin(0);
	mouse_rotation_sensivety_slider.SetRangeMax(100);

	mouse_translation_sensivety_slider.SetRangeMin(0);
	mouse_translation_sensivety_slider.SetRangeMax(100);

	mouse_scaling_sensivety_slider.SetRangeMin(0);
	mouse_scaling_sensivety_slider.SetRangeMax(100);


	return TRUE;  // return TRUE unless you set the focus to a control
}



void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_DISTANCE, d_slider);
	DDX_Control(pDX, IDC_DISTANCE_TEXTBOX, distance_textbox);
	DDX_Control(pDX, IDC_ROTATION_SENSIVITY, mouse_rotation_sensivety_slider);
	DDX_Control(pDX, IDC_TRANSLATION_SENSIVETY, mouse_translation_sensivety_slider);
	DDX_Control(pDX, IDC_SCALING_SENSIVETY, mouse_scaling_sensivety_slider);
	DDX_Control(pDX, IDC_FINESS_SLIDER, fineness_slider);
	DDX_Control(pDX, IDC_FINESS_TEXTBOX, fineness_textbox);
	DDX_Control(pDX, IDC_NORMALS_SCALE_TEXTBOX, txt_normal_scale);

	// Initialize the dialog with values
	if (pDX->m_bSaveAndValidate == FALSE) 
	{
		// perspective settings - we will use 10x units for a bit of fraction
		d_slider.SetPos((int)(mainView->m_scene.getPerspectiveD() * 10));
		textBoxSetFixedPoint(distance_textbox, (int)(mainView->m_scene.getPerspectiveD() * 10));

		mouse_rotation_sensivety_slider.SetPos((int)mainView->rotationSensivety);
		mouse_translation_sensivety_slider.SetPos((int)mainView->movementSensivety);
		mouse_scaling_sensivety_slider.SetPos((int)mainView->scaleSensivety);

		fineness_slider.SetPos((int)mainView->fineness);
		textBoxSetFixedPoint(fineness_textbox, (int)mainView->fineness, 1);
		textBoxSetDouble(txt_normal_scale, mainView->m_scene.getNormalScale());
	}
}

void CSettingsDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
	CString s;

	if (nSBCode != SB_THUMBPOSITION && nSBCode != SB_THUMBTRACK)
		return;

	if (pSlider == &d_slider)
		textBoxSetFixedPoint(distance_textbox, nPos);
	else if (pSlider == &fineness_slider)
		textBoxSetFixedPoint(fineness_textbox, nPos, 1);
	else  if (pSlider == &mouse_rotation_sensivety_slider)
			mainView->rotationSensivety = nPos;
	else if (pSlider == &mouse_scaling_sensivety_slider)
		mainView->scaleSensivety = nPos;
	else if (pSlider == &mouse_translation_sensivety_slider)
		mainView->movementSensivety = nPos;


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CSettingsDialog::OnEnUpdateDistanceTextbox()
{
	double tmp = textBoxGetDouble(distance_textbox);
	if (tmp >= 0) {
		mainView->m_scene.setPerspectiveD(tmp);
		mainView->redrawScene();
		UpdateData(FALSE);
	}
}

void CSettingsDialog::OnEnChangeFinessTextbox()
{
	double tmp = textBoxGetDouble(fineness_textbox);
	if (tmp >= 2) mainView->fineness = (int)tmp;
	UpdateData(FALSE);
}


void CSettingsDialog::OnBnClickedSensivetyReset()
{
	mainView->rotationSensivety = 50;
	mainView->movementSensivety = 50;
	mainView->scaleSensivety = 50;
	UpdateData(FALSE);
}

void CSettingsDialog::OnBnClickedFinessApply()
{
	if (!mainView->m_strItdFileName.IsEmpty()) {
		mainView->m_scene.loadSceneFromIRIT(mainView->m_strItdFileName, mainView->fineness);
		mainView->redrawScene();
	}
}


void CSettingsDialog::OnEnChangeNormalsScaleTextbox()
{
	double scale = textBoxGetDouble(txt_normal_scale);
	mainView->m_scene.setNormalScale(scale);
	UpdateData(FALSE);
	mainView->redrawScene();
}

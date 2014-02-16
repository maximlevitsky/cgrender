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

#include "CG4View.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "colorlabel.h"

// CMaterialDlg dialog

class CCG4View;

class CMaterialDlg : public CDialog
{
	DECLARE_DYNAMIC(CMaterialDlg)

public:
	CMaterialDlg(CCG4View *view);
	virtual ~CMaterialDlg();

// Dialog Data
	enum { IDD = IDD_MATERIAL_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedResetAll();

	afx_msg void OnBnClickedChangeBackgrondColor();
	afx_msg void OnBnClickedChangeDefaultObjectColor();

private:
	CCG4View *view;
	bool updating;
public:
	afx_msg void OnDialogDataUpdate();
	CEdit txt_ambient;
	CEdit txt_diffusse;
	CEdit txt_shineness;
	CEdit txt_specular;
	CEdit texture_textbox;

	CColorLabel ctlObjectColors;
	CColorLabel ctlBackGround;

	afx_msg void OnBnClickedTextureFilenameChoose();
	afx_msg void OnBnClickedBtnColorReset();
	afx_msg void OnBnClickedBtnTextureReset();
	afx_msg void OnBnClickedBtnNoTexture();
	CEdit txt_fog_density;
	CEdit txt_fog_start;
	CEdit txt_fog_end;
	CSpinButtonCtrl spin_ambi_coof;
	CSpinButtonCtrl spin_diffuse_coof;
	CSpinButtonCtrl spin_spec_coof;
	CSpinButtonCtrl spin_shineneess_coof;
	CSpinButtonCtrl spin_fog_density_coof;
	CSpinButtonCtrl spin_fog_start;
	CSpinButtonCtrl spin_fog_end;
	afx_msg void OnSpinChangePhongCoofs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSpinChangeDensity(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSpinChangeDistances(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedChangeFogColor();
	CColorLabel fogColor;
	afx_msg void OnCbnSelchangeComboTextureMode();
	CComboBox modeCombo;
	afx_msg void OnBnClickedBackgroundColorReset();
	afx_msg void OnBnClickedFogReset();
};

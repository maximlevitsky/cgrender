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

class CCG4View;

class CLightDialog : public CDialog
{
	DECLARE_DYNAMIC(CLightDialog)

public:
	CLightDialog(CCG4View* view) : view(view), m_currentLightIdx(0), CDialog(CLightDialog::IDD, NULL), updating(true) {}
	virtual ~CLightDialog() {}

	enum { IDD = IDD_LIGHTS_DLG };

protected:
	int m_currentLightIdx;
	 bool updating;
	int GetCurrentLightIndex();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDialogDataUpdate();
	afx_msg void OnDialogLightChanged();

    virtual BOOL OnInitDialog();
private:
	CCG4View* view;
public:
	CEdit color_r;
	CEdit color_g;
	CEdit color_b;
	CEdit pos_x;
	CEdit pos_y;
	CEdit pos_z;
	CEdit dir_x;
	CEdit dir_y;
	CEdit dir_z;
	CEdit ambient_r;
	CEdit ambient_g;
	CEdit ambient_b;
	CSpinButtonCtrl spin_color_r;
	CSpinButtonCtrl spin_color_g;
	CSpinButtonCtrl spin_color_b;
	CSpinButtonCtrl spin_pos_x;
	CSpinButtonCtrl spin_pos_y;
	CSpinButtonCtrl spin_pos_z;
	CSpinButtonCtrl spin_dir_x;
	CSpinButtonCtrl spin_dir_y;
	CSpinButtonCtrl spin_dir_z;
	CSpinButtonCtrl spin_ambi_r;
	CSpinButtonCtrl spin_ambi_g;
	CSpinButtonCtrl spin_ambi_b;
	afx_msg void OnBnClickedLigthingReset();
	CSpinButtonCtrl spin_color_spec_r;
	CSpinButtonCtrl spin_color_spec_g;
	CSpinButtonCtrl spin_color_spec_b;
	CEdit color_spec_r;
	CEdit color_spec_g;
	CEdit color_spec_b;
	afx_msg void OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposMapSize(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposBiases(NMHDR *pNMHDR, LRESULT *pResult);

	CComboBox cb_space;
	CComboBox cb_type;
	CEdit txt_cutoff_angle;
	CSpinButtonCtrl spin_cutoff_angle;
	afx_msg void OnBnClickedLightDebug();
	CEdit txt_z_bias_mul;
	CEdit txt_max_z_bias;
	CEdit txt_pcf_taps;
	CEdit txt_shadow_map_res;
	CSpinButtonCtrl spin_z_bias_mul;
	CSpinButtonCtrl spin_z_bias_max;
	CSpinButtonCtrl spin_z_bias_pcf_taps;
	CSpinButtonCtrl spin_shadowmap_res;
	afx_msg void OnBnClickedShadowsReset();
};

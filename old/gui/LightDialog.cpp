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
// LightDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CG4.h"
#include "LightDialog.h"
#include "MFCHelpers.h"

#include <sstream>

BEGIN_MESSAGE_MAP(CLightDialog, CDialog)
    ON_BN_CLICKED(IDC_RADIO_LIGHT1, &CLightDialog::OnDialogLightChanged)
    ON_BN_CLICKED(IDC_RADIO_LIGHT2, &CLightDialog::OnDialogLightChanged)
    ON_BN_CLICKED(IDC_RADIO_LIGHT3, &CLightDialog::OnDialogLightChanged)
    ON_BN_CLICKED(IDC_RADIO_LIGHT4, &CLightDialog::OnDialogLightChanged)
    ON_BN_CLICKED(IDC_RADIO_LIGHT5, &CLightDialog::OnDialogLightChanged)
    ON_BN_CLICKED(IDC_RADIO_LIGHT6, &CLightDialog::OnDialogLightChanged)
    ON_BN_CLICKED(IDC_RADIO_LIGHT7, &CLightDialog::OnDialogLightChanged)
    ON_BN_CLICKED(IDC_RADIO_LIGHT8, &CLightDialog::OnDialogLightChanged)

	ON_EN_CHANGE(IDC_AMBL_COLOR_R, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_AMBL_COLOR_G, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_AMBL_COLOR_B, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_POS_X, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_POS_Y, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_POS_Z, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_DIR_X, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_DIR_Y, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_DIR_Z, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_COLOR_R, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_COLOR_G, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_COLOR_B, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_COLOR_R2, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_COLOR_G2, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_LIGHT_COLOR_B2, &CLightDialog::OnDialogDataUpdate)
	ON_BN_CLICKED(IDC_LIGHT_ENABLED, &CLightDialog::OnDialogDataUpdate)
	ON_CBN_SELCHANGE(IDC_LIGHT_TYPE,&CLightDialog::OnDialogDataUpdate)
	ON_CBN_SELCHANGE(IDC_LIGHT_SPACE, &CLightDialog::OnDialogDataUpdate)

	ON_EN_CHANGE(IDC_CUTOFF_ANGLE_TEXTBOX, &CLightDialog::OnDialogDataUpdate)

	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_DIR_X, &CLightDialog::OnDeltaposSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_DIR_Y, &CLightDialog::OnDeltaposSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_DIR_Z, &CLightDialog::OnDeltaposSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_X, &CLightDialog::OnDeltaposSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_Y, &CLightDialog::OnDeltaposSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_Z, &CLightDialog::OnDeltaposSpin)

	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Z_MULTIPLY, &CLightDialog::OnDeltaposBiases)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAX_Z_BIAS, &CLightDialog::OnDeltaposBiases)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHADOW_MAP_RES, &CLightDialog::OnDeltaposMapSize)

	ON_BN_CLICKED(IDC_LIGTHING_RESET, &CLightDialog::OnBnClickedLigthingReset)
	ON_BN_CLICKED(IDC_CAN_CAST_SHADOW, &CLightDialog::OnDialogDataUpdate)
	ON_BN_CLICKED(IDC_LIGHT_DEBUG, &CLightDialog::OnBnClickedLightDebug)

	ON_EN_CHANGE(IDC_Z_BIAS_MULITPLIER, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_MAX_Z_BIAS, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_PCF_TAPS, &CLightDialog::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_SHADOW_MAP_RES, &CLightDialog::OnDialogDataUpdate)
	ON_BN_CLICKED(IDC_CHECK_PCF, &CLightDialog::OnDialogDataUpdate)
	ON_BN_CLICKED(IDC_POISON_EN, &CLightDialog::OnDialogDataUpdate)	
	ON_BN_CLICKED(IDC_SHADOWS_RESET, &CLightDialog::OnBnClickedShadowsReset)
END_MESSAGE_MAP()


// CLightDialog dialog
IMPLEMENT_DYNAMIC(CLightDialog, CDialog)

void CLightDialog::DoDataExchange(CDataExchange* pDX)
{
	updating = true;
	CDialog::DoDataExchange(pDX);

	LightParams *ambent = view->m_scene.getLightParams(-1);
	LightParams *current = view->m_scene.getLightParams(m_currentLightIdx);

	DDX_Control(pDX, IDC_LIGHT_COLOR_R, color_r);
	DDX_Control(pDX, IDC_LIGHT_COLOR_G, color_g);
	DDX_Control(pDX, IDC_LIGHT_COLOR_B, color_b);
	DDX_Control(pDX, IDC_LIGHT_POS_X, pos_x);
	DDX_Control(pDX, IDC_LIGHT_POS_Y, pos_y);
	DDX_Control(pDX, IDC_LIGHT_POS_Z, pos_z);
	DDX_Control(pDX, IDC_LIGHT_DIR_X, dir_x);
	DDX_Control(pDX, IDC_LIGHT_DIR_Y, dir_y);
	DDX_Control(pDX, IDC_LIGHT_DIR_Z, dir_z);
	DDX_Control(pDX, IDC_AMBL_COLOR_R, ambient_r);
	DDX_Control(pDX, IDC_AMBL_COLOR_G, ambient_g);
	DDX_Control(pDX, IDC_AMBL_COLOR_B, ambient_b);

	DDX_Control(pDX, IDC_SPIN1, spin_color_r);
	DDX_Control(pDX, IDC_SPIN2, spin_color_g);
	DDX_Control(pDX, IDC_SPIN4, spin_color_b);

	DDX_Control(pDX, IDC_SPIN_POS_X, spin_pos_x);
	DDX_Control(pDX, IDC_SPIN_POS_Y, spin_pos_y);
	DDX_Control(pDX, IDC_SPIN_POS_Z, spin_pos_z);
	DDX_Control(pDX, IDC_SPIN_DIR_X, spin_dir_x);
	DDX_Control(pDX, IDC_SPIN_DIR_Y, spin_dir_y);
	DDX_Control(pDX, IDC_SPIN_DIR_Z, spin_dir_z);

	DDX_Control(pDX, IDC_SPIN_AMBI_R, spin_ambi_r);
	DDX_Control(pDX, IDC_SPIN_AMBI_G, spin_ambi_g);
	DDX_Control(pDX, IDC_SPIN_AMBI_B, spin_ambi_b);
	DDX_Control(pDX, IDC_SPIN3, spin_color_spec_r);
	DDX_Control(pDX, IDC_SPIN5, spin_color_spec_g);
	DDX_Control(pDX, IDC_SPIN12, spin_color_spec_b);
	DDX_Control(pDX, IDC_LIGHT_COLOR_R2, color_spec_r);
	DDX_Control(pDX, IDC_LIGHT_COLOR_G2, color_spec_g);
	DDX_Control(pDX, IDC_LIGHT_COLOR_B2, color_spec_b);

	DDX_Control(pDX, IDC_LIGHT_SPACE, cb_space);
	DDX_Control(pDX, IDC_LIGHT_TYPE, cb_type);

	DDX_Control(pDX, IDC_CUTOFF_ANGLE_TEXTBOX, txt_cutoff_angle);
	DDX_Control(pDX, IDC_SPIN_SPOT_ANGLE, spin_cutoff_angle);

	DDX_Control(pDX, IDC_Z_BIAS_MULITPLIER, txt_z_bias_mul);
	DDX_Control(pDX, IDC_MAX_Z_BIAS, txt_max_z_bias);
	DDX_Control(pDX, IDC_PCF_TAPS, txt_pcf_taps);
	DDX_Control(pDX, IDC_SHADOW_MAP_RES, txt_shadow_map_res);

	DDX_Control(pDX, IDC_SPIN_Z_MULTIPLY, spin_z_bias_mul);
	DDX_Control(pDX, IDC_SPIN_MAX_Z_BIAS, spin_z_bias_max);
	DDX_Control(pDX, IDC_SPIN_PCF_TAPS, spin_z_bias_pcf_taps);
	DDX_Control(pDX, IDC_SPIN_SHADOW_MAP_RES, spin_shadowmap_res);




	int helper=current->enabled;
	DDX_Check(pDX,IDC_LIGHT_ENABLED,helper);
	current->enabled = (bool)helper;

	helper =current->type;
	DDX_CBIndex(pDX,IDC_LIGHT_TYPE,helper);
	current->type = (LightType)helper;

	helper = current->space;
	DDX_CBIndex(pDX,IDC_LIGHT_SPACE,helper);
	current->space = (LightSpace)helper;


	helper=current->shadow;
	DDX_Check(pDX,IDC_CAN_CAST_SHADOW,helper);
	current->shadow = (bool)helper;

	/* reading values from controls*/
	if (pDX->m_bSaveAndValidate) {
		ambent->diffuseColor[0] = min(255,textBoxGetInt(ambient_r));
		ambent->diffuseColor[1] = min(255,textBoxGetInt(ambient_g));
		ambent->diffuseColor[2] = min(255,textBoxGetInt(ambient_b));

		current->diffuseColor[0] = min(255,textBoxGetInt(color_r));
		current->diffuseColor[1] = min(255,textBoxGetInt(color_g));
		current->diffuseColor[2] = min(255,textBoxGetInt(color_b));

		current->specularColor[0] = min(255,textBoxGetInt(color_spec_r));
		current->specularColor[1] = min(255,textBoxGetInt(color_spec_g));
		current->specularColor[2] = min(255,textBoxGetInt(color_spec_b));

		current->position[0] = textBoxGetDouble(pos_x);
		current->position[1] = textBoxGetDouble(pos_y);
		current->position[2] = textBoxGetDouble(pos_z);

		current->direction[0] = textBoxGetDouble(dir_x);
		current->direction[1] = textBoxGetDouble(dir_y);
		current->direction[2] = textBoxGetDouble(dir_z);

		current->cutoffAngle = min(179,textBoxGetInt(txt_cutoff_angle));

		view->m_scene.invalidateShadowMaps();
	} else {

		textBoxSetInt(ambient_r, (int)ambent->diffuseColor[0]);
		textBoxSetInt(ambient_g, (int)ambent->diffuseColor[1]);
		textBoxSetInt(ambient_b, (int)ambent->diffuseColor[2]);

		textBoxSetInt(color_r, (int)current->diffuseColor[0]);
		textBoxSetInt(color_g, (int)current->diffuseColor[1]);
		textBoxSetInt(color_b, (int)current->diffuseColor[2]);

		textBoxSetInt(color_spec_r, (int)current->specularColor[0]);
		textBoxSetInt(color_spec_g, (int)current->specularColor[1]);
		textBoxSetInt(color_spec_b, (int)current->specularColor[2]);


		textBoxSetDouble(pos_x, current->position[0]);
		textBoxSetDouble(pos_y, current->position[1]);
		textBoxSetDouble(pos_z, current->position[2]);

		/* point lights have no direction */
		textBoxSetDouble(dir_x, current->direction[0]);
		textBoxSetDouble(dir_y, current->direction[1]);
		textBoxSetDouble(dir_z, current->direction[2]);

		textBoxSetInt(txt_cutoff_angle, current->cutoffAngle);

	}

	// setup shadow params
	ShadowParams params = view->m_scene.getShadowParams();

	helper=params.pcf;
	DDX_Check(pDX,IDC_CHECK_PCF,helper);
	params.pcf = (bool)helper;

	helper=params.poison;
	DDX_Check(pDX,IDC_POISON_EN,helper);
	params.poison = (bool)helper;


	/* reading values from controls*/
	if (pDX->m_bSaveAndValidate) {
		params.pcf_taps = max(0,textBoxGetInt(txt_pcf_taps));
		params.shadowMapRes = max(16, textBoxGetInt(txt_shadow_map_res));
		params.z_bias_max = max(0.0,textBoxGetDouble(txt_max_z_bias));
		params.z_bias_mul = textBoxGetDouble(txt_z_bias_mul);
	} else 
	{
		textBoxSetInt(txt_pcf_taps, params.pcf_taps);
		textBoxSetInt(txt_shadow_map_res, params.shadowMapRes);
		textBoxSetDouble(txt_max_z_bias, params.z_bias_max);
		textBoxSetDouble(txt_z_bias_mul, params.z_bias_mul);
	}

	view->m_scene.invalidateShadowMaps();
	view->m_scene.freeShadowMaps();

	view->m_scene.setShadowParams(&params);

	color_r.EnableWindow(current->enabled);
	color_g.EnableWindow(current->enabled);
	color_b.EnableWindow(current->enabled);

	color_spec_r.EnableWindow(current->enabled);
	color_spec_g.EnableWindow(current->enabled);
	color_spec_b.EnableWindow(current->enabled);

	pos_x.EnableWindow(current->enabled);
	pos_y.EnableWindow(current->enabled);
	pos_z.EnableWindow(current->enabled);

	dir_x.EnableWindow(current->enabled && current->type != LIGHT_TYPE_POINT);
	dir_y.EnableWindow(current->enabled && current->type != LIGHT_TYPE_POINT);
	dir_z.EnableWindow(current->enabled && current->type != LIGHT_TYPE_POINT);

	cb_space.EnableWindow(current->enabled);
	cb_type.EnableWindow(current->enabled);
	updating = false;

}


//this callback function is called when each of the radio buttons on the dialog is clicked
void CLightDialog::OnDialogLightChanged() 
{
	if (updating)
		return;

	//save the dialog state into the data variables
	UpdateData(TRUE);
	m_currentLightIdx=GetCheckedRadioButton(IDC_RADIO_LIGHT1,IDC_RADIO_LIGHT8)-IDC_RADIO_LIGHT1;
	UpdateData(FALSE);

	view->redrawScene();

}

void CLightDialog::OnDialogDataUpdate()
{
	if (updating)
		return;

	//save the dialog state into the data variables
	UpdateData(TRUE);
	view->redrawScene();



}

BOOL CLightDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    //Set the radio button of the current light to be selected
    CheckRadioButton(IDC_RADIO_LIGHT1,IDC_RADIO_LIGHT8,m_currentLightIdx+IDC_RADIO_LIGHT1);

	spin_color_r.SetRange(0,255);
	spin_color_g.SetRange(0,255);
	spin_color_b.SetRange(0,255);

	spin_color_spec_r.SetRange(0,255);
	spin_color_spec_g.SetRange(0,255);
	spin_color_spec_b.SetRange(0,255);

	spin_dir_x.SetRange32(-INT_MAX, INT_MAX);
	spin_dir_y.SetRange32(-INT_MAX, INT_MAX);
	spin_dir_z.SetRange32(-INT_MAX, INT_MAX);

	spin_pos_x.SetRange32(-INT_MAX, INT_MAX);
	spin_pos_y.SetRange32(-INT_MAX, INT_MAX);
	spin_pos_z.SetRange32(-INT_MAX, INT_MAX);

	spin_ambi_r.SetRange(0,255);
	spin_ambi_g.SetRange(0,255);
	spin_ambi_b.SetRange(0,255);

	spin_cutoff_angle.SetRange(0,179);

	spin_z_bias_pcf_taps.SetRange(0, 1024);
	spin_shadowmap_res.SetRange(0, 20); // power of two

	spin_z_bias_mul.SetRange(0,1);
	spin_shadowmap_res.SetRange(0,1);
	spin_z_bias_max.SetRange(0,1);
    return TRUE;  
}




void CLightDialog::OnBnClickedLigthingReset()
{
	view->m_scene.resetLighting();
	UpdateData(FALSE);
	view->redrawScene();
}


void CLightDialog::OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	spinHandler(this, pNMHDR, pResult, 0.5);

}

void CLightDialog::OnDeltaposBiases(NMHDR *pNMHDR, LRESULT *pResult)
{
	spinHandler(this, pNMHDR, pResult, 0.01);

}

void CLightDialog::OnDeltaposMapSize(NMHDR *pNMHDR, LRESULT *pResult) {

	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CSpinButtonCtrl *ctl = (CSpinButtonCtrl *)GetDlgItem(pNMUpDown->hdr.idFrom);
	CEdit *textBox = (CEdit*)ctl->GetBuddy();

	int value = textBoxGetInt(*textBox);

	if (pNMUpDown->iDelta > 0)
		value *= (pNMUpDown->iDelta+1);
	else if (pNMUpDown->iDelta < 0)
		value /= (-pNMUpDown->iDelta+1);

	textBoxSetInt(*textBox,value);
	*pResult = 0;

}



void CLightDialog::OnBnClickedLightDebug()
{
	const DepthTexture *map = view->m_scene.getShadowMap(m_currentLightIdx);

	for (int i = 0 ; i < 6*MAX_LIGHT ; i++) 
	{
		const DepthTexture *map = view->m_scene.getShadowMap(i);
		if (map)  
		{
			std::stringstream str;
			str << "map" << i << ".png";
			map->saveToFile(str.str().c_str());
		}
	}
}


void CLightDialog::OnBnClickedShadowsReset()
{
	ShadowParams p = view->m_scene.getShadowParams();
	p.reset();
	view->m_scene.setShadowParams(&p);
	UpdateData(FALSE);
	view->redrawScene();
}

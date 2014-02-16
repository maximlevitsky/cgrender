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
// MaterialDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CG4.h"
#include "MaterialDlg.h"
#include "MFCHelpers.h"


BEGIN_MESSAGE_MAP(CMaterialDlg, CDialog)

	/* reset buttons */
	ON_BN_CLICKED(IDC_MATRIAL_RESET, &CMaterialDlg::OnBnClickedResetAll)
	ON_BN_CLICKED(IDC_BTN_COLOR_RESET, &CMaterialDlg::OnBnClickedBtnColorReset)
	ON_BN_CLICKED(IDC_BTN_TEXTURE_RESET, &CMaterialDlg::OnBnClickedBtnTextureReset)


	ON_EN_CHANGE(IDC_MATERIAL_AMBIENT, &CMaterialDlg::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_MATERIAL_DIFFUSE, &CMaterialDlg::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_MATERIAL_SPECULAR, &CMaterialDlg::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_MATERIAL_SHININESS, &CMaterialDlg::OnDialogDataUpdate)

	/* fog settings */
	ON_CBN_SELCHANGE(IDC_FOG_MODE, &CMaterialDlg::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_FOG_INTENSITY, &CMaterialDlg::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_TXT_FOG_START, &CMaterialDlg::OnDialogDataUpdate)
	ON_EN_CHANGE(IDC_TXT_FOG_END, &CMaterialDlg::OnDialogDataUpdate)

	/* misc*/
	ON_BN_CLICKED(IDC_TEXTURE_FILENAME_CHOOSE, &CMaterialDlg::OnBnClickedTextureFilenameChoose)
	ON_BN_CLICKED(IDC_CHANGE_BACKGROND_COLOR, &CMaterialDlg::OnBnClickedChangeBackgrondColor)
	ON_BN_CLICKED(IDC_CHANGE_DEFAULT_OBJECT_COLOR, &CMaterialDlg::OnBnClickedChangeDefaultObjectColor)
	ON_BN_CLICKED(IDC_BTN_NO_TEXTURE, &CMaterialDlg::OnBnClickedBtnNoTexture)

	ON_CBN_SELCHANGE(IDC_TEX_SAMPLE_MODE, &CMaterialDlg::OnDialogDataUpdate)

	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, &CMaterialDlg::OnSpinChangePhongCoofs)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, &CMaterialDlg::OnSpinChangePhongCoofs)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN6, &CMaterialDlg::OnSpinChangePhongCoofs)

	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, &CMaterialDlg::OnSpinChangeDensity)

	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &CMaterialDlg::OnSpinChangeDistances)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CMaterialDlg::OnSpinChangeDistances)
	ON_BN_CLICKED(IDC_CHANGE_FOG_COLOR, &CMaterialDlg::OnBnClickedChangeFogColor)
	ON_CBN_SELCHANGE(IDC_COMBO_TEXTURE_MODE, &CMaterialDlg::OnCbnSelchangeComboTextureMode)
	ON_BN_CLICKED(IDC_BACKGROUND_COLOR_RESET, &CMaterialDlg::OnBnClickedBackgroundColorReset)
	ON_BN_CLICKED(IDC_FOG_RESET, &CMaterialDlg::OnBnClickedFogReset)
END_MESSAGE_MAP()


// CMaterialDlg dialog

IMPLEMENT_DYNAMIC(CMaterialDlg, CDialog)

CMaterialDlg::CMaterialDlg(CCG4View *view)
	: CDialog(CMaterialDlg::IDD, NULL)
	, view(view), updating(true)
{}

CMaterialDlg::~CMaterialDlg()
{}


void CMaterialDlg::DoDataExchange(CDataExchange* pDX)
{
	updating = true;
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MATERIAL_AMBIENT, txt_ambient);
	DDX_Control(pDX, IDC_MATERIAL_DIFFUSE, txt_diffusse);
	DDX_Control(pDX, IDC_MATERIAL_SHININESS, txt_shineness);
	DDX_Control(pDX, IDC_MATERIAL_SPECULAR, txt_specular);
	DDX_Control(pDX, IDC_TEXTURE_FILENAME, texture_textbox);
	DDX_Control(pDX, IDC_OBJECT_COLOR, ctlObjectColors);
	DDX_Control(pDX, IDC_BACKGROUND_COLOR, ctlBackGround);
	DDX_Control(pDX, IDC_FOG_INTENSITY, txt_fog_density);
	DDX_Control(pDX, IDC_TXT_FOG_START, txt_fog_start);
	DDX_Control(pDX, IDC_TXT_FOG_END, txt_fog_end);

	DDX_Control(pDX, IDC_SPIN4, spin_ambi_coof);
	DDX_Control(pDX, IDC_SPIN5, spin_spec_coof);
	DDX_Control(pDX, IDC_SPIN6, spin_diffuse_coof);

	DDX_Control(pDX, IDC_SPIN7, spin_shineneess_coof);
	DDX_Control(pDX, IDC_SPIN3, spin_fog_density_coof);
	DDX_Control(pDX, IDC_SPIN1, spin_fog_start);
	DDX_Control(pDX, IDC_SPIN2, spin_fog_end);
	DDX_Control(pDX, IDC_FOG_COLOR, fogColor);

	MaterialParams& params = view->m_scene.getMatrialParams();


	if (pDX->m_bSaveAndValidate) {
		/* read data from controls*/
		params.setAmbient(textBoxGetDouble(txt_ambient));
		params.setDiffuse(textBoxGetDouble(txt_diffusse));

		params.setSpecular(textBoxGetDouble(txt_specular));
		params.setShinenes(textBoxGetInt(txt_shineness));
	} else 
	{
		textBoxSetDouble(txt_ambient, params.getAmbient());
		textBoxSetDouble(txt_diffusse, params.getDiffuse());
		textBoxSetDouble(txt_specular, params.getSpecular());
		textBoxSetInt(txt_shineness, params.getShineness());

		Color background = view->m_scene.getBackgroundColor();
		ctlBackGround.setColor(RGB(background[0], background[1], background[2]));

		Color objectColor = params.getObjectColor();
		ctlObjectColors.setColor(RGB(objectColor[0], objectColor[1], objectColor[2]));
	}

	texture_textbox.SetWindowText(params.getObjectTexture().c_str());

	int helper = params.getMode();
	DDX_CBIndex(pDX,IDC_COMBO_TEXTURE_MODE,helper);


	helper = view->m_scene.getTextureSampleMode();
	DDX_CBIndex(pDX,IDC_TEX_SAMPLE_MODE,helper);
	view->m_scene.setTextureSampleMode((TextureSampleMode)helper);

	/* transfer fog params*/
	FogParams fogParams = view->m_scene.getFogParams();

	helper = fogParams.enabled ? fogParams.type+1 : 0;
	DDX_CBIndex(pDX,IDC_FOG_MODE,helper);
	fogParams.enabled = helper > 0;
	fogParams.type = (FogType)(helper - 1);

	if (pDX->m_bSaveAndValidate) {
		/* read data from controls*/
		fogParams.density = max(0,textBoxGetDouble(txt_fog_density));
		fogParams.startPoint = textBoxGetDouble(txt_fog_start);
		fogParams.endPoint = textBoxGetDouble(txt_fog_end);
		view->m_scene.setFogParams(fogParams);
	} else {

		/* set data to the controls*/
		textBoxSetDouble(txt_fog_density, fogParams.density);
		textBoxSetDouble(txt_fog_start, fogParams.startPoint);
		textBoxSetDouble(txt_fog_end, fogParams.endPoint);
		fogColor.setColor(RGB(fogParams.color[0],fogParams.color[1], fogParams.color[2]));
	}


	updating = false;
	DDX_Control(pDX, IDC_COMBO_TEXTURE_MODE, modeCombo);
}


afx_msg void CMaterialDlg::OnBnClickedResetAll()
{
	view->m_scene.resetMaterials();
	view->m_scene.resetFog();
	view->m_scene.resetBackground();
	view->m_scene.resetTextureMode();

	UpdateData(FALSE);
	view->redrawScene();
}


void CMaterialDlg::OnDialogDataUpdate()
{
	if (updating)
		return;

	//save the dialog state into the data variables
	UpdateData(TRUE);
	view->redrawScene();
}


void CMaterialDlg::OnBnClickedTextureFilenameChoose()
{
	TCHAR szFilters[] = _T ("Image files (*.png,*.jpeg,*gif)|*.png;*.jpeg;*.jpg;*.gif|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, "png", "", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ,szFilters);

	if (dlg.DoModal () != IDOK)
		return;

	MaterialParams& params = view->m_scene.getMatrialParams();

	params.setObjectTexture(dlg.GetPathName().GetBuffer(),1,1);
	texture_textbox.SetWindowText(params.getObjectTexture().c_str());
	view->m_scene.reloadTextures();
	view->redrawScene();
}


void CMaterialDlg::OnBnClickedChangeBackgrondColor()
{
	Engine &s = view->m_scene;
	s.setBackgroundColor(chooseColor(s.getBackgroundColor(), &ctlBackGround));
	view->redrawScene();
}

void CMaterialDlg::OnBnClickedChangeDefaultObjectColor()
{
	Engine &s = view->m_scene;

	MaterialParams& params = view->m_scene.getMatrialParams();
	params.setObjectColor(chooseColor(params.getObjectColor(), &ctlObjectColors));
	view->redrawScene();
}

void CMaterialDlg::OnBnClickedBtnColorReset()
{
	MaterialParams& params = view->m_scene.getMatrialParams();
	params.resetObjectColor();
	UpdateData(FALSE);
	view->redrawScene();
}


void CMaterialDlg::OnBnClickedBtnTextureReset()
{
	MaterialParams& params = view->m_scene.getMatrialParams();
	params.resetTexture();
	params.resetMode();
	UpdateData(FALSE);

	view->m_scene.resetTextureMode();
	view->m_scene.reloadTextures();
	view->redrawScene();
}


void CMaterialDlg::OnBnClickedBtnNoTexture()
{
	MaterialParams& params = view->m_scene.getMatrialParams();
	params.setObjectTexture("",1,1);
	texture_textbox.SetWindowText(params.getObjectTexture().c_str());
	view->m_scene.reloadTextures();
	view->redrawScene();
}


void CMaterialDlg::OnSpinChangePhongCoofs(NMHDR *pNMHDR, LRESULT *pResult)
{
	spinHandler(this, pNMHDR, pResult, 0.1);
	UpdateData(TRUE);
	UpdateData(FALSE);
}

void CMaterialDlg::OnSpinChangeDensity( NMHDR *pNMHDR, LRESULT *pResult )
{
	spinHandler(this, pNMHDR, pResult, 0.5);
	UpdateData(TRUE);
	UpdateData(FALSE);
}

afx_msg void CMaterialDlg::OnSpinChangeDistances(NMHDR *pNMHDR, LRESULT *pResult)
{
	spinHandler(this, pNMHDR, pResult, 0.05);
	UpdateData(TRUE);
	UpdateData(FALSE);
}


BOOL CMaterialDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	spin_ambi_coof.SetRange(0,10);
	spin_diffuse_coof.SetRange(0,10);
	spin_spec_coof.SetRange(0,10);
	spin_shineneess_coof.SetRange32(0, INT_MAX);

	spin_fog_density_coof.SetRange32(0, INT_MAX);
	spin_fog_start.SetRange32(0, INT_MAX);
	spin_fog_end.SetRange32(0, INT_MAX);

	return TRUE;
}


void CMaterialDlg::OnBnClickedChangeFogColor()
{
	FogParams fogParams = view->m_scene.getFogParams();
	fogParams.color = chooseColor(fogParams.color, &fogColor);
	view->m_scene.setFogParams(fogParams);
	view->redrawScene();
}


void CMaterialDlg::OnCbnSelchangeComboTextureMode()
{
	MaterialParams& params = view->m_scene.getMatrialParams();
	params.setMode((TextureUVMode)modeCombo.GetCurSel());
	view->redrawScene();
}


void CMaterialDlg::OnBnClickedBackgroundColorReset()
{
	view->m_scene.resetBackground();
	UpdateData(FALSE);
	view->redrawScene();
}


void CMaterialDlg::OnBnClickedFogReset()
{
	view->m_scene.resetFog();
	UpdateData(FALSE);
	view->redrawScene();
}
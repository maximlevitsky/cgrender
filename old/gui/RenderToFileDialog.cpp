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
#include "RenderToFileDialog.h"
#include "afxdialogex.h"


// CRenderToFileDialog dialog

IMPLEMENT_DYNAMIC(CRenderToFileDialog, CDialogEx)

CRenderToFileDialog::CRenderToFileDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRenderToFileDialog::IDD, pParent)
	, targetFileName(_T(""))
	, dimisionX(0)
	, dimisionY(0)
{

}

void CRenderToFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RENDER_FILE_LOCATION, targetFileName);
	DDX_Text(pDX, IDC_RENDER_FILE_X, dimisionX);
	DDX_Text(pDX, IDC_RENDER_FILE_Y, dimisionY);
	DDV_MinMaxInt(pDX, dimisionX, 1, INT_MAX);
	DDV_MinMaxInt(pDX, dimisionY, 1, INT_MAX);
	DDX_Control(pDX, IDOK, btnOK);
	DDX_Control(pDX, IDC_RENDER_FILE_CHOOSE, btnFileChoose);
}


BEGIN_MESSAGE_MAP(CRenderToFileDialog, CDialogEx)
	ON_BN_CLICKED(IDC_RENDER_FILE_CHOOSE, &CRenderToFileDialog::OnBnClickedRenderFileChoose)
END_MESSAGE_MAP()


// CRenderToFileDialog message handlers
void CRenderToFileDialog::OnBnClickedRenderFileChoose()
{
	TCHAR szFilters[] = _T ("PNG (*.png)|*.png|All Files (*.*)|*.*||");
	CFileDialog dlg(FALSE, "png", targetFileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY ,szFilters);

	if (dlg.DoModal() == IDOK) {
		UpdateData(true);
		targetFileName = dlg.GetPathName();
		UpdateData(false);
	}
	
	btnOK.SetFocus();
	btnFileChoose.SetButtonStyle(BS_PUSHBUTTON);
	btnOK.SetButtonStyle(BS_DEFPUSHBUTTON);
}
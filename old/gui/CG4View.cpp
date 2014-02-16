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
// CG4View.cpp : implementation of the CCG4View class
//
#include "stdafx.h"
#include "CG4.h"

#include "CG4View.h"

#include <iostream>
using std::cout;
using std::endl;
#include "MaterialDlg.h"
#include "LightDialog.h"
#include "RenderToFileDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "IO\iritSkel.h"

#include <time.h>


// For Status Bar access
#include "MainFrm.h"
#include "SettingsDialog.h"

// Use this macro to display text messages in the status bar.
#define STATUS_BAR_TEXT(str) (((CMainFrame*)GetParentFrame())->getStatusBar().SetWindowText(str))


/////////////////////////////////////////////////////////////////////////////
// CCG4View

IMPLEMENT_DYNCREATE(CCG4View, CWnd)

BEGIN_MESSAGE_MAP(CCG4View, CWnd)

	// messages
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SETCURSOR()

	ON_COMMAND(ID_FILE_LOAD, OnFileLoad)
	ON_COMMAND(ID_FILE_RESET, OnFileReset)

	// perspective
	ON_COMMAND(ID_VIEW_ORTHOGRAPHIC, OnViewOrthographic)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ORTHOGRAPHIC, OnUpdateViewOrthographic)
	ON_COMMAND(ID_VIEW_PERSPECTIVE, OnViewPerspective)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PERSPECTIVE, OnUpdateViewPerspective)

	// transformations
	ON_COMMAND(ID_ACTION_ROTATE, OnActionRotate)
	ON_UPDATE_COMMAND_UI(ID_ACTION_ROTATE, OnUpdateActionRotate)
	ON_COMMAND(ID_ACTION_SCALE, OnActionScale)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SCALE, OnUpdateActionScale)
	ON_COMMAND(ID_ACTION_TRANSLATE, OnActionTranslate)
	ON_UPDATE_COMMAND_UI(ID_ACTION_TRANSLATE, OnUpdateActionTranslate)
	ON_COMMAND(ID_AXIS_X, OnAxisX)
	ON_UPDATE_COMMAND_UI(ID_AXIS_X, OnUpdateAxisX)
	ON_COMMAND(ID_AXIS_Y, OnAxisY)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Y, OnUpdateAxisY)
	ON_COMMAND(ID_AXIS_Z, OnAxisZ)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Z, OnUpdateAxisZ)
	ON_COMMAND(ID_AXIS_COMBINED, OnAxisCombined)
	ON_UPDATE_COMMAND_UI(ID_AXIS_COMBINED, OnUpdateAxisCombined)
	ON_COMMAND(ID_OBJECT_SPACE_TRANSFORM, &CCG4View::OnObjectSpaceTransform)
	ON_COMMAND(ID_OPTIONS_PERSPECTIVECONTROL, &CCG4View::OnOptionsGeneralSettings)
	ON_COMMAND(ID_ACTION_RESETVIEW, &CCG4View::OnActionResetTransformations)
	ON_COMMAND(ID_SEP_OBJECT_TRANSFORM, &CCG4View::OnSepObjectTransform)
	ON_UPDATE_COMMAND_UI(ID_SEP_OBJECT_TRANSFORM, &CCG4View::OnUpdateSepObjectTransform)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_SPACE_TRANSFORM, &CCG4View::OnUpdateObjectSpaceTransform)
	ON_COMMAND(ID_CAMERA_SPACE_TRANSFORM, &CCG4View::OnCameraSpaceTransform)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_SPACE_TRANSFORM, &CCG4View::OnUpdateCameraSpaceTransform)


	// shading
	ON_COMMAND(ID_SHADING_NONE, &CCG4View::OnShadingNone)
	ON_UPDATE_COMMAND_UI(ID_SHADING_NONE, &CCG4View::OnUpdateShadingNone)
	ON_COMMAND(ID_LIGHT_SHADING_FLAT, OnShadingFlat)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_FLAT, OnUpdateShadingFlat)
	ON_COMMAND(ID_LIGHT_SHADING_GOURAUD, OnShadingGouraud)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_GOURAUD, OnUpdateShadingGouraud)
	ON_COMMAND(ID_LIGHT_SHADING_PHONG, &CCG4View::OnShadingPhong)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_PHONG, &CCG4View::OnUpdateShadingPhong)
	ON_COMMAND(ID_LIGHT_CONSTANTS, OnShadingLightSettings)
	ON_COMMAND(ID_SHADINGLIGHTNING_MATERIAL, &CCG4View::OnShadingMaterialParams)

	// background


	// render
	ON_COMMAND(ID_RENDER_ONSCREEN, &CCG4View::OnRenderOnscreen)
	ON_COMMAND(ID_RENDER_TOFILE, &CCG4View::OnRenderTofile)

	// options
	ON_COMMAND(ID_DRAW_BOUNDING_BOX, &CCG4View::OnOptionsDrawBoundingBox)
	ON_COMMAND(ID_DRAW_SURFACE_NORMALS, &CCG4View::OnOptionsDrawSurfaceNormals)
	ON_COMMAND(ID_DRAW_VERTEX_NORMALS, &CCG4View::OnOptionsDrawVertexNormals)
	ON_UPDATE_COMMAND_UI(ID_DRAW_BOUNDING_BOX, &CCG4View::OnOptionsUpdateDrawBoundingBox)
	ON_UPDATE_COMMAND_UI(ID_DRAW_SURFACE_NORMALS, &CCG4View::OnOptionsUpdateDrawSurfaceNormals)
	ON_UPDATE_COMMAND_UI(ID_DRAW_VERTEX_NORMALS, &CCG4View::OnOptionsUpdateDrawVertexNormals)

	ON_COMMAND(ID_DRAW_AXES, &CCG4View::OnOptionsDrawAxes)
	ON_UPDATE_COMMAND_UI(ID_DRAW_AXES, &CCG4View::OnOptionsUpdateDrawAxes)

	ON_COMMAND(ID_SHADING_WIREFRAME, &CCG4View::OnShadingWireframe)
	ON_UPDATE_COMMAND_UI(ID_SHADING_WIREFRAME, &CCG4View::OnUpdateShadingWireframe)

	ON_COMMAND(ID_OPTIONS_BACKFACESCULLING, &CCG4View::OnOptionsBackfacesculling)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_BACKFACESCULLING, &CCG4View::OnUpdateOptionsBackfacesculling)

	ON_COMMAND(ID_OPTIONS_DEPTHRENDERING, &CCG4View::OnDebugDepthrendering)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_DEPTHRENDERING, &CCG4View::OnUpdateDebugDepthrendering)

	ON_COMMAND(ID_OPTIONS_PERSPECTIVECORRECTRENDERING, &CCG4View::OnDebugPerspectivecorrectrendering)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_PERSPECTIVECORRECTRENDERING, &CCG4View::OnUpdateDebugPerspectivecorrectrendering)
	ON_COMMAND(ID_BACKGROUND_LOAD, &CCG4View::OnBackgroundLoad)
	ON_COMMAND(ID_BACKGROUND_RESET, &CCG4View::OnBackgroundReset)
	ON_COMMAND(ID_BACKGROUND_STRETCH, &CCG4View::OnBackgroundStretch)
	ON_COMMAND(ID_BACKGROUND_TILE, &CCG4View::OnBackgroundTile)
	ON_UPDATE_COMMAND_UI(ID_BACKGROUND_STRETCH, &CCG4View::OnUpdateBackgroundStretch)
	ON_UPDATE_COMMAND_UI(ID_BACKGROUND_TILE, &CCG4View::OnUpdateBackgroundTile)
	ON_COMMAND(ID_OPTIONS_INVERSEVERTEX_NORMALS, &CCG4View::OnDebugInversevertexNormals)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_INVERSEVERTEX_NORMALS, &CCG4View::OnUpdateDebugInversevertexNormals)
	ON_COMMAND(ID_OPTIONS_INVERSESURFACENORMALS, &CCG4View::OnDebugInverseSurfaceNormals)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_INVERSESURFACENORMALS, &CCG4View::OnDebugOptionsInverseSurfaceNormals)
	ON_COMMAND(ID_SHADINGLIGHTNING_LIGHTBACKFACES, &CCG4View::OnOptionsLightbackfaces)
	ON_UPDATE_COMMAND_UI(ID_SHADINGLIGHTNING_LIGHTBACKFACES, &CCG4View::OnUpdateOptionsLightbackfaces)
	ON_COMMAND(ID_DEBUG_DEBUGMODEL, &CCG4View::OnDebugLoadDebugmodel)
	ON_COMMAND(ID_DEBUG_INVERTDEPTH, &CCG4View::OnDebugInvertdepth)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_INVERTDEPTH, &CCG4View::OnUpdateDebugInvertdepth)
	ON_COMMAND(ID_DEBUG_FORCEALLFRONTFACES, &CCG4View::OnDebugForceallfrontfaces)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_FORCEALLFRONTFACES, &CCG4View::OnUpdateDebugForceallfrontfaces)
	ON_COMMAND(ID_OPTIONS_DRAWLIGHTSOURCES, &CCG4View::OnOptionsDrawlightsources)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_DRAWLIGHTSOURCES, &CCG4View::OnUpdateOptionsDrawlightsources)
	END_MESSAGE_MAP()


CCG4View::CCG4View() : m_pDC(NULL)
{
	// Set default values
	m_nAxis = ID_AXIS_COMBINED;
	m_nAction = ID_ACTION_ROTATE;
	m_transform_space = ID_OBJECT_SPACE_TRANSFORM;

	mouseLDown = false;
	mouseRDown = false;
	fineness = 20;

	movementSensivety = 50;
	scaleSensivety = 50;
	rotationSensivety = 50;

	resizing = false;
	moving = false;
	sceneValid = false;

	settingsDiag = new CSettingsDialog(this);
	lightDialog = new CLightDialog(this);
	matDialog = new CMaterialDlg(this);

	outputTexture = NULL;
}

CCG4View::~CCG4View()
{
	delete settingsDiag;
	delete lightDialog;
	delete matDialog;
	delete outputTexture;
}


#ifdef _DEBUG
void CCG4View::AssertValid() const
{
	CWnd::AssertValid();
}

void CCG4View::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);
}

#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CCG4View Window Creation - Linkage of windows to CG4

BOOL CCG4View::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);
	return TRUE;
}

int CCG4View::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pDC = new CClientDC(this);

	/* Create the DC we will use to paint the bitmap*/
	memoryDC.CreateCompatibleDC(m_pDC);
	settingsDiag->Create(CSettingsDialog::IDD, CWnd::GetDesktopWindow());
	settingsDiag->ShowWindow(SW_HIDE);

	lightDialog->Create(CLightDialog::IDD, CWnd::GetDesktopWindow());
	lightDialog->ShowWindow(SW_HIDE);

	matDialog->Create(CMaterialDlg::IDD, CWnd::GetDesktopWindow()); 
	matDialog->ShowWindow(SW_HIDE);

	return 0;
}

void CCG4View::OnDestroy() 
{
	CWnd::OnDestroy();

	// delete the DC
	if ( m_pDC ) {
		delete m_pDC;
	}

	settingsDiag->DestroyWindow();

	DeleteObject (hGlobalBitMap);
	memoryDC.DeleteDC();
}

//////////////// GENERAL MESSAGES /////////////////////////////////////////////////////////////

void CCG4View::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	if ( 0 >= cx || 0 >= cy ) {
		return;
	}

	/* Get new window information*/
	RECT clientRect;
	GetClientRect(&clientRect);

	/* Calculate our data*/
	windowSize.cx = clientRect.right - clientRect.left;
	windowSize.cy = clientRect.bottom - clientRect.top;

	if (windowSize.cx > bitmapSize.cx || windowSize.cy > bitmapSize.cy) 
	{
		if (outputTexture)
			delete outputTexture;

		/* Free the old bitmap if needed*/
		if (hGlobalBitMap)
			DeleteObject (hGlobalBitMap);


		/* Calculate the new bitmap size*/
		bitmapSize.cx = max(windowSize.cx, bitmapSize.cx);
		bitmapSize.cy = max(windowSize.cy, bitmapSize.cy);

		/* Create a minimum header for our bitmap*/
		BITMAPINFO BMInfo;
		memset(&BMInfo, 0, sizeof(BMInfo));
		BMInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		BMInfo.bmiHeader.biWidth       = bitmapSize.cx;
		BMInfo.bmiHeader.biHeight      = bitmapSize.cy;
		BMInfo.bmiHeader.biPlanes      = 1;
		BMInfo.bmiHeader.biBitCount    = 32;
		BMInfo.bmiHeader.biCompression = BI_RGB;

		/* Create the bitmap*/
		DEVICE_PIXEL *pixels;

		hGlobalBitMap = CreateDIBSection(memoryDC, &BMInfo, DIB_RGB_COLORS, (void**)&pixels, NULL,0);

		/* Select the bitmap into memory DC*/
		memoryDC.SelectObject(hGlobalBitMap);

		/* And erase it*/
		memoryDC.FillSolidRect(0,0,bitmapSize.cx,bitmapSize.cy, RGB(0,0,0));

		/* wrap it in output texture */
		outputTexture = new Texture(pixels, bitmapSize.cx,bitmapSize.cy);
	}

	m_scene.setOutput(&m_renderer, outputTexture, MY_SIZE(windowSize.cx, windowSize.cy));
	redrawScene();
}


BOOL CCG4View::OnEraseBkgnd(CDC* pDC) 
{
	return true;
}

void CCG4View::OnPaint()
{
	if (!sceneValid) {

		clock_t start = clock();

		try {
			m_scene.render();
		} 

		catch (CMemoryException e) {
			AfxMessageBox("Out of memory while rendering");
			m_renderer.fillBackground(m_scene.getBackgroundColor());
		}
		GdiFlush();

		clock_t end = clock();

		int msecs = (end - start) / (CLOCKS_PER_SEC / 1000);

		CString str;

		if (msecs  == 0) msecs = 1;
		str.Format("Rendering took %i.%03i seconds  (%i FPS)", msecs / 1000, msecs % 1000, (int)(1000.0/msecs));

		RECT r;
		r.top = bitmapSize.cy - windowSize.cy;
		r.bottom = r.top + 100;
		r.left = windowSize.cx - 300;
		r.right = windowSize.cx;

		Color bkg = m_scene.getBackgroundColor();

		memoryDC.SetBkColor(RGB((int)bkg[0],(int)bkg[1],(int)bkg[2]));
		memoryDC.SetTextColor(RGB(255,255,255));
		memoryDC.DrawText(str, &r, DT_CENTER);

		sceneValid = true;
	}


	// Paint the window bitmap to the screen
	CPaintDC dc(this);
	m_pDC->BitBlt(0,0,windowSize.cx, windowSize.cy, &memoryDC, 0,bitmapSize.cy - windowSize.cy,SRCCOPY);


}

///////// MODEL MENU /////////////////////////////////////////////////////////////////////////////////////

void CCG4View::OnFileLoad() 
{
	TCHAR szFilters[] = _T ("IRIT Data Files (*.itd)|*.itd|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, "itd", "", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ,szFilters);

	/* TODO: this should be connected to model class*/

	if (dlg.DoModal () == IDOK) {
		m_strItdFileName = dlg.GetPathName();

		if (m_scene.loadSceneFromIRIT(m_strItdFileName, fineness)) {
			GetParent()->SetWindowText(m_strItdFileName);
			redrawScene();	// force a WM_PAINT for drawing.
			settingsDiag->UpdateData(FALSE);
			lightDialog->UpdateData(FALSE);
			matDialog->UpdateData(FALSE);
		}
	} 
}

void CCG4View::OnFileReset()
{
	m_scene.resetScene();
	redrawScene();
}

//// BACKGROUND MENU ////////////////////////////////////////////////////////////////////////////////////////

void CCG4View::OnBackgroundLoad()
{
	TCHAR szFilters[] = _T ("Image files (*.png,*.jpeg,*gif)|*.png;*.jpeg;*.jpg;*.gif|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, "png", "", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ,szFilters);

	if (dlg.DoModal () == IDOK) {
		m_strPngFileName = dlg.GetPathName();

		if (!m_scene.loadBackgroundImage(m_strPngFileName)) {
			AfxMessageBox("Failed to load image, maybe its of unsupported type (only 8/16/24/32 bpp supported");
			return;
		}

		redrawScene();	// force a WM_PAINT for drawing.		
	} 
}

void CCG4View::OnBackgroundReset()
{
	m_scene.resetBackground();
	redrawScene();
}


void CCG4View::OnBackgroundStretch()
{
	m_scene.setTileBackground(false);
	redrawScene();
}


void CCG4View::OnBackgroundTile()
{
	m_scene.setTileBackground(true);
	redrawScene();
}


void CCG4View::OnUpdateBackgroundStretch(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_scene.getTileBackground() == false);
}


void CCG4View::OnUpdateBackgroundTile(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_scene.getTileBackground() == true);
}


////// TRANSFORMATIONS/AXIS MENU //////////////////////////////////////////////////////////////////////////

void CCG4View::OnAxisX() 
{
	m_nAxis = ID_AXIS_X;
}
void CCG4View::OnAxisY() 
{
	m_nAxis = ID_AXIS_Y;
}


void CCG4View::OnAxisZ() 
{
	m_nAxis = ID_AXIS_Z;
}

void CCG4View::OnAxisCombined()
{
	m_nAxis = ID_AXIS_COMBINED;
}

void CCG4View::OnUpdateAxisX(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_X);
}

void CCG4View::OnUpdateAxisY(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_Y);
}


void CCG4View::OnUpdateAxisZ(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_Z);
}

void CCG4View::OnUpdateAxisCombined( CCmdUI* pCmdUI )
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_COMBINED);
}

////// TRANSFORMATIONS/MODE MENU ////////////////////////////////////////////////////////////////////////////


void CCG4View::OnActionRotate() 
{
	m_nAction = ID_ACTION_ROTATE;
}

void CCG4View::OnUpdateActionRotate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_ROTATE);
	pCmdUI->Enable(m_nAxis != ID_AXIS_COMBINED);

}

void CCG4View::OnActionTranslate() 
{
	m_nAction = ID_ACTION_TRANSLATE;
}

void CCG4View::OnUpdateActionTranslate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_TRANSLATE);
	pCmdUI->Enable(m_nAxis != ID_AXIS_COMBINED);

}

void CCG4View::OnActionScale() 
{
	m_nAction = ID_ACTION_SCALE;
}

void CCG4View::OnUpdateActionScale(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_SCALE && m_transform_space != ID_CAMERA_SPACE_TRANSFORM);
	pCmdUI->Enable(m_nAxis != ID_AXIS_COMBINED && m_transform_space != ID_CAMERA_SPACE_TRANSFORM);
}


////// TRANSFORMATIONS/TARGET MENU ////////////////////////////////////////////////////////////////////////

void CCG4View::OnObjectSpaceTransform()
{
	m_transform_space = ID_OBJECT_SPACE_TRANSFORM;
	m_scene.setDrawSeperateObjects(false);
	matDialog->UpdateData(FALSE);
	redrawScene();
}

void CCG4View::OnUpdateObjectSpaceTransform(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_transform_space == ID_OBJECT_SPACE_TRANSFORM);
}

void CCG4View::OnCameraSpaceTransform()
{
	m_transform_space = ID_CAMERA_SPACE_TRANSFORM;
	m_scene.setDrawSeperateObjects(false);
	matDialog->UpdateData(FALSE);
	redrawScene();
}

void CCG4View::OnUpdateCameraSpaceTransform(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_transform_space == ID_CAMERA_SPACE_TRANSFORM);
}

void CCG4View::OnSepObjectTransform()
{
	m_scene.setDrawSeperateObjects(true);
	matDialog->UpdateData(FALSE);
	redrawScene();
	m_transform_space = ID_SEP_OBJECT_TRANSFORM;
}

void CCG4View::OnUpdateSepObjectTransform(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_transform_space == ID_SEP_OBJECT_TRANSFORM);
}

void CCG4View::OnActionResetTransformations()
{
	m_scene.resetTransformations();
	settingsDiag->UpdateData(FALSE);
	redrawScene();
}



//// PROJECTION MENU /////////////////////////////////////////////////////////////////////////////////////

void CCG4View::OnViewOrthographic() 
{
	m_scene.setOrtographicRendering();
	redrawScene();		// redraw using the new view.
}

void CCG4View::OnUpdateViewOrthographic(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(!m_scene.isPerspectiveRendering());
}

void CCG4View::OnViewPerspective() 
{
	m_scene.setPerspectiveRendering();
	redrawScene();
}

void CCG4View::OnUpdateViewPerspective(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_scene.isPerspectiveRendering());
}


//////////////// SHADING /////////////////////////////////////////////////////////////////////////////

void CCG4View::OnShadingNone()
{
	m_scene.setShadingMode(Engine::SHADING_NONE);
	redrawScene();
}

void CCG4View::OnUpdateShadingNone(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getShadingMode() == Engine::SHADING_NONE);
}


void CCG4View::OnShadingFlat() 
{
	m_scene.setShadingMode(Engine::SHADING_FLAT);
	redrawScene();
}

void CCG4View::OnUpdateShadingFlat(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_scene.getShadingMode() == Engine::SHADING_FLAT);
}


void CCG4View::OnShadingGouraud() 
{
	m_scene.setShadingMode(Engine::SHADING_GOURAD);
	redrawScene();
}

void CCG4View::OnUpdateShadingGouraud(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_scene.getShadingMode() ==Engine::SHADING_GOURAD);
}


void CCG4View::OnShadingPhong()
{
	m_scene.setShadingMode(Engine::SHADING_PHONG);
	redrawScene();
}


void CCG4View::OnUpdateShadingPhong(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getShadingMode() == Engine::SHADING_PHONG);
}


void CCG4View::OnShadingWireframe()
{
	m_scene.setDrawWireFrame(!m_scene.getdrawWireFrame());
	redrawScene();
}


void CCG4View::OnUpdateShadingWireframe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getdrawWireFrame());
	pCmdUI->Enable(m_scene.getShadingMode() != Engine::SHADING_NONE);
}


void CCG4View::OnShadingLightSettings() 
{
	lightDialog->ShowWindow(SW_SHOWNORMAL);
	lightDialog->SetForegroundWindow();
}

void CCG4View::OnShadingMaterialParams()
{	
	matDialog->ShowWindow(SW_SHOWNORMAL);
	matDialog->SetForegroundWindow();
}

////// RENDER MENU ///////////////////////////////////////////////////////////////////////////////////////////


void CCG4View::OnRenderOnscreen()
{
	redrawScene();
}

void CCG4View::OnRenderTofile()
{
	CRenderToFileDialog dlg(this);
	dlg.dimisionX = windowSize.cx;
	dlg.dimisionY = windowSize.cy;

	if (dlg.DoModal() != IDOK)
		return;
	
	MY_SIZE size(dlg.dimisionX, dlg.dimisionY);
	Texture output(size.cx,size.cy);

	m_scene.setOutput(&m_renderer, &output, size);
	m_scene.render();
	m_scene.setOutput(&m_renderer, outputTexture, MY_SIZE(windowSize.cx, windowSize.cy));

	if (!output.saveToFile(dlg.targetFileName)) 
		AfxMessageBox("Cant write to destination file");	
}

//////// OPTIONS //////////////////////////////////////////////////////////////////////////////////////////

void CCG4View::OnOptionsBackfacesculling()
{
	m_scene.setBackFaceCulling(!m_scene.getBackfaceCulling());
	redrawScene();
}


void CCG4View::OnUpdateOptionsBackfacesculling(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getBackfaceCulling() && !m_scene.getForceAllFrontFaces());
	pCmdUI->Enable(!m_scene.getForceAllFrontFaces());
}


void CCG4View::OnOptionsDrawAxes()
{
	m_scene.setDrawAxes(!m_scene.getDrawAxes());
	redrawScene();
}


void CCG4View::OnOptionsUpdateDrawAxes(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getDrawAxes());
}


void CCG4View::OnOptionsDrawBoundingBox()
{
	 m_scene.setDrawBoundingBox(!m_scene.getDrawBoundingBox());
	 redrawScene();
}

void CCG4View::OnOptionsDrawSurfaceNormals()
{
	m_scene.setDrawPolygonNormals(!m_scene.getdrawPolygonNormals());
	redrawScene();
}

void CCG4View::OnOptionsDrawVertexNormals()
{
	m_scene.setDrawVertexNormals(!m_scene.getdrawVertexNormals());
	redrawScene();
}

void CCG4View::OnOptionsUpdateDrawBoundingBox(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getDrawBoundingBox());
}

void CCG4View::OnOptionsUpdateDrawSurfaceNormals(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getdrawPolygonNormals());
}

void CCG4View::OnOptionsUpdateDrawVertexNormals(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getdrawVertexNormals());
}

void CCG4View::OnOptionsLightbackfaces()
{
	m_scene.setLightBackFaces(!m_scene.getLightBackFaces());
	redrawScene();
}


void CCG4View::OnUpdateOptionsLightbackfaces(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getLightBackFaces());
	pCmdUI->Enable(!m_scene.getBackfaceCulling());
}

void CCG4View::OnOptionsGeneralSettings()
{
	settingsDiag->ShowWindow(SW_SHOWNORMAL);
	settingsDiag->SetForegroundWindow();
}

//////////////// DEBUG MENU //////////////////////////////////////////////////////////////////

void CCG4View::OnDebugDepthrendering()
{
	m_scene.setDepthRendering(!m_scene.getDepthRendering());
	redrawScene();
}

void CCG4View::OnUpdateDebugDepthrendering(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getDepthRendering());
}

void CCG4View::OnDebugPerspectivecorrectrendering()
{
	m_scene.setPerspectiveCorrect(!m_scene.getPerspectiveCorrect());
	redrawScene();
}


void CCG4View::OnUpdateDebugPerspectivecorrectrendering(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getPerspectiveCorrect());
}

void CCG4View::OnDebugInversevertexNormals()
{
	m_scene.invertVertexNormals(!m_scene.getInvertedVertexNormals());
	redrawScene();
}


void CCG4View::OnUpdateDebugInversevertexNormals(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getInvertedVertexNormals());
}


void CCG4View::OnDebugInverseSurfaceNormals()
{
	m_scene.invertPolygonNormals(!m_scene.getInvertedPolygonNormals());
	redrawScene();
}


void CCG4View::OnDebugOptionsInverseSurfaceNormals(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getInvertedPolygonNormals());
}

void CCG4View::OnDebugLoadDebugmodel()
{
	m_scene.loadDebugScene();
	settingsDiag->UpdateData(FALSE);
	lightDialog->UpdateData(FALSE);
	redrawScene();
}

void CCG4View::OnDebugInvertdepth()
{
	m_scene.setInvertDepth(!m_scene.getInvertDepth());
	redrawScene();
}


void CCG4View::OnUpdateDebugInvertdepth(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getInvertDepth());
}

//////////////////// MOUSE HANDLING ////////////////////////////////////////////////


void CCG4View::OnLButtonDown(UINT nFlags, CPoint point)
{
	point.y = windowSize.cy - point.y - 1;

	CWnd::OnLButtonDown(nFlags, point);

	if (m_transform_space == ID_SEP_OBJECT_TRANSFORM && !(nFlags & (MK_MBUTTON | MK_SHIFT | MK_CONTROL))) {
		if (m_scene.selectObject(point.x, point.y)) {
			matDialog->UpdateData(FALSE);
			redrawScene();
		}
	}

	if (!mouseLDown && !mouseRDown)
		mouseStartPos = point;
	mouseLDown = true;
}

void CCG4View::OnRButtonDown(UINT nFlags, CPoint point)
{
	point.y = windowSize.cy - point.y - 1;

	CWnd::OnRButtonDown(nFlags, point);

	if (!mouseLDown && !mouseRDown)
		mouseStartPos = point;
	mouseRDown = true;
}

void CCG4View::OnLButtonUp(UINT nFlags, CPoint point)
{
	mouseLDown = false;
	point.y = windowSize.cy - point.y - 1;

	CWnd::OnLButtonUp(nFlags, point);

}

void CCG4View::OnRButtonUp(UINT nFlags, CPoint point)
{
	mouseRDown = false;
	CWnd::OnRButtonUp(nFlags, point);
}

void CCG4View::OnMouseMove(UINT nFlags, CPoint point)
{
	point.y = windowSize.cy - point.y - 1;
	CSize screenDist = mouseStartPos - point;

	if (!mouseLDown && !mouseRDown) 
	{
		return;
	}

	Vector3 p1 = m_scene.translatePointBackToCameraSpace(mouseStartPos.x, mouseStartPos.y, 0);
	Vector3 p2 = m_scene.translatePointBackToCameraSpace(point.x, point.y, 0);
	Vector3 dist = p2 - p1;


	if (m_nAxis == ID_AXIS_COMBINED) 
	{
		if (nFlags & MK_LBUTTON) {

			// move if shift
			if (nFlags & MK_SHIFT) {

				if (m_transform_space == ID_CAMERA_SPACE_TRANSFORM) 
				{
					m_scene.moveCamera(0, dist.x() * translateSensivety(movementSensivety));
					m_scene.moveCamera(1, dist.y() * translateSensivety(movementSensivety));
				} else {
					m_scene.moveObject(0, dist.x() * translateSensivety(movementSensivety));
					m_scene.moveObject(1, dist.y() * translateSensivety(movementSensivety));
				}
		
			// scale if control
			} else if (nFlags & MK_CONTROL) {

				if (m_transform_space != ID_CAMERA_SPACE_TRANSFORM) {
					m_scene.scaleObject(0, dist.x() * translateSensivety(scaleSensivety));
					m_scene.scaleObject(1, dist.y() * translateSensivety(scaleSensivety));
				}

			// rotate otherwise
			} else {

				if (m_transform_space == ID_CAMERA_SPACE_TRANSFORM) 
				{
					m_scene.rotateCamera(0, screenDist.cy * translateSensivety(rotationSensivety));
					m_scene.rotateCamera(1,  screenDist.cx * translateSensivety(rotationSensivety));

				} else
				{
					m_scene.rotateObject(0, screenDist.cy * translateSensivety(rotationSensivety));
					m_scene.rotateObject(1,  screenDist.cx * translateSensivety(rotationSensivety));
				}

			}

		} else if (nFlags & MK_RBUTTON) {

			Vector3 p1 = m_scene.translatePointBackToCameraSpace(0, 0, 0);
			Vector3 p2 = m_scene.translatePointBackToCameraSpace(0, 0, 0.01*(screenDist.cx));

			double dist = (p1 - p2).z();

			// move if shift
			if (nFlags & MK_SHIFT) {

				if (m_transform_space == ID_CAMERA_SPACE_TRANSFORM)
					m_scene.moveCamera(2, dist * translateSensivety(movementSensivety));
				else
					m_scene.moveObject(2, dist * translateSensivety(movementSensivety));

				// scale if control
			} else if (nFlags & MK_CONTROL) {

				if (m_transform_space != ID_CAMERA_SPACE_TRANSFORM)
					m_scene.scaleObject(2, dist * translateSensivety(scaleSensivety));

				// rotate otherwise
			} else {

				if (m_transform_space == ID_CAMERA_SPACE_TRANSFORM)
					m_scene.rotateCamera(2, screenDist.cx * translateSensivety(rotationSensivety));
				else
					m_scene.rotateObject(2, screenDist.cx * translateSensivety(rotationSensivety));
			}
		}

	} else {

		int axis;
		switch(m_nAxis) {
		case ID_AXIS_X:
			axis = 0;
			break;
		case ID_AXIS_Y:
			axis = 1;
			break;
		case ID_AXIS_Z:
			axis = 2;
			break;
		}

		switch(m_nAction) {
		case ID_ACTION_ROTATE:
			if (m_transform_space == ID_CAMERA_SPACE_TRANSFORM)
				m_scene.rotateCamera(axis, screenDist.cx * translateSensivety(rotationSensivety));
			else
				m_scene.rotateObject(axis, screenDist.cx * translateSensivety(rotationSensivety));

			break;
		case ID_ACTION_SCALE:

			if (m_transform_space != ID_CAMERA_SPACE_TRANSFORM)
				m_scene.scaleObject(axis, dist.x() * translateSensivety(scaleSensivety));
			break;
		case ID_ACTION_TRANSLATE:

			if (m_transform_space == ID_CAMERA_SPACE_TRANSFORM)
				m_scene.moveCamera(axis, dist.x() * translateSensivety(movementSensivety));
			else
				m_scene.moveObject(axis, dist.x() * translateSensivety(movementSensivety));

			break;
		} 
	}

	redrawScene();
	CWnd::OnMouseMove(nFlags, point);
	mouseStartPos = point;
}

BOOL CCG4View::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	/* delta is given in 120x multiplier*/
	zDelta /= 120;

	Vector3 p1 = m_scene.translatePointBackToCameraSpace(0, 0, 0);
	Vector3 p2 = m_scene.translatePointBackToCameraSpace(0, 0, 0.01*(zDelta));
	double scaleDist = p1.z() - p2.z();

	if (m_transform_space != ID_CAMERA_SPACE_TRANSFORM) {
		m_scene.scaleObject(0, scaleDist * translateSensivety(scaleSensivety));
		m_scene.scaleObject(1, scaleDist * translateSensivety(scaleSensivety));
		m_scene.scaleObject(2, scaleDist * translateSensivety(scaleSensivety));
	}

	redrawScene();
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CCG4View::OnLButtonDblClk(UINT nFlags, CPoint point)
{	
	redrawScene();
	CWnd::OnLButtonDblClk(nFlags, point);
}

///////////////// KEYBOARD HANDLING ///////////////////////////////////////////

void CCG4View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_nAxis != ID_AXIS_COMBINED)
		return;

	if (nChar == VK_SHIFT) {
		moving = true;
	}

	else if (nChar == VK_CONTROL && m_transform_space != ID_CAMERA_SPACE_TRANSFORM) {
		resizing = true;
	}

	updateCursor();
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCG4View::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	resizing = moving = false;
	updateCursor();
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL CCG4View::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	updateCursor();
	return TRUE;
}

//////////////////////////// HELPER FUNCTIONS ////////////////////////////////////

void  CCG4View::updateCursor()
{
	if (resizing)
		::SetCursor(LoadCursor(NULL, IDC_SIZEALL));
	else if (moving)
		::SetCursor(LoadCursor(NULL, IDC_HAND));
	else
		::SetCursor(LoadCursor(NULL, IDC_ARROW));
}





void CCG4View::OnDebugForceallfrontfaces()
{
	m_scene.setForceAllFrontFaces(!m_scene.getForceAllFrontFaces());
	redrawScene();
}


void CCG4View::OnUpdateDebugForceallfrontfaces(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getForceAllFrontFaces());
}


void CCG4View::OnOptionsDrawlightsources()
{
	m_scene.setDrawLightSources(!m_scene.getDrawLightSources());
	redrawScene();
}


void CCG4View::OnUpdateOptionsDrawlightsources(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_scene.getDrawLightSources());
}

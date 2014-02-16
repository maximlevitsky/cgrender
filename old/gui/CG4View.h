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
// CG4View.h : interface of the CCG4View class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CG4VIEW_H__5857316D_EA60_11D5_9FD5_00D0B718E2CD__INCLUDED_)
#define AFX_CG4VIEW_H__5857316D_EA60_11D5_9FD5_00D0B718E2CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Common\Utilities.h"
#include "Common\Mat4.h"
#include "Common\Vector4.h"
#include "Renderer\Renderer.h"
#include "Engine\Engine.h"
#include "SettingsDialog.h"
#include "LightDialog.h"
#include "MaterialDlg.h"



class CSettingsDialog;
class CLightDialog;
class CMaterialDlg;

class CCG4View : public CWnd
{
public: // create from serialization only
	CCG4View();
	DECLARE_DYNCREATE(CCG4View)

private:
	// current model status
	CString m_strItdFileName;		// file name of IRIT data
	int fineness;

	// gui status
	int m_nAxis;				// Axis of Action, X Y or Z
	int m_nAction;				// Rotate, Translate, Scale
	int m_transform_space;		// camera, object, separate object

	// mouse status
	bool mouseLDown;
	bool mouseRDown;
	bool resizing;
	bool moving;
	CPoint mouseStartPos;

	// Output buffer
	HBITMAP hGlobalBitMap; 	/* This is the bitmap we use to blit to the main window. Its recreated on window resizes*/

	Texture *outputTexture;
	SIZE windowSize;
	SIZE bitmapSize;
	CDC memoryDC; 	/* A GDI memory DC that bitmap is bound to*/
	CDC*  m_pDC;    // holds the Device Context


	// our hard working scene and render
	Renderer m_renderer;
	Engine m_scene;

	// mouse sensivety -don t abuse these for model specific tweaks
	double movementSensivety;
	double scaleSensivety;
	double rotationSensivety;

	CSettingsDialog* settingsDiag;
	CLightDialog *lightDialog;
	CMaterialDlg *matDialog;
	CString m_strPngFileName;       // file name of the background image

	//{{AFX_VIRTUAL(CCG4View)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL
public:
	virtual ~CCG4View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	void updateCursor();

	double translateSensivety(double value) 
	{
		double result;

		if (value >= 50)
			result =  1+((double)value - 50) / 5;
		else
			result =  1.0 / (1.0 + (50-(value-1))/5);

		return result;
	}

	bool sceneValid;
	void redrawScene() 
	{
		sceneValid = false;
		Invalidate();
	}

	friend class CSettingsDialog;
	friend class CLightDialog;
	friend class CMaterialDlg;

// Generated message map functions
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileLoad();
	afx_msg void OnViewOrthographic();
	afx_msg void OnUpdateViewOrthographic(CCmdUI* pCmdUI);
	afx_msg void OnViewPerspective();
	afx_msg void OnUpdateViewPerspective(CCmdUI* pCmdUI);
	afx_msg void OnActionRotate();
	afx_msg void OnUpdateActionRotate(CCmdUI* pCmdUI);
	afx_msg void OnActionScale();
	afx_msg void OnUpdateActionScale(CCmdUI* pCmdUI);
	afx_msg void OnActionTranslate();
	afx_msg void OnUpdateActionTranslate(CCmdUI* pCmdUI);

	afx_msg void OnAxisX();
	afx_msg void OnUpdateAxisX(CCmdUI* pCmdUI);
	afx_msg void OnAxisY();
	afx_msg void OnUpdateAxisY(CCmdUI* pCmdUI);
	afx_msg void OnAxisZ();

	afx_msg void OnAxisCombined();
	afx_msg void OnUpdateAxisCombined(CCmdUI* pCmdUI);
	
	afx_msg void OnUpdateAxisZ(CCmdUI* pCmdUI);
	afx_msg void OnShadingFlat();
	afx_msg void OnUpdateShadingFlat(CCmdUI* pCmdUI);
	afx_msg void OnShadingGouraud();
	afx_msg void OnUpdateShadingGouraud(CCmdUI* pCmdUI);
	afx_msg void OnShadingLightSettings();
	afx_msg void OnPaint();
	afx_msg void OnOptionsDrawBoundingBox();
	afx_msg void OnOptionsDrawSurfaceNormals();
	afx_msg void OnOptionsDrawVertexNormals();
	afx_msg void OnObjectSpaceTransform();
	afx_msg void OnOptionsGeneralSettings();
	afx_msg void OnOptionsUpdateDrawBoundingBox(CCmdUI *pCmdUI);
	afx_msg void OnOptionsUpdateDrawSurfaceNormals(CCmdUI *pCmdUI);
	afx_msg void OnOptionsUpdateDrawVertexNormals(CCmdUI *pCmdUI);
	afx_msg void OnUpdateObjectSpaceTransform(CCmdUI *pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnCameraSpaceTransform();
	afx_msg void OnUpdateCameraSpaceTransform(CCmdUI *pCmdUI);
	afx_msg void OnActionResetTransformations();
	afx_msg void OnSepObjectTransform();
	afx_msg void OnUpdateSepObjectTransform(CCmdUI *pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOptionsDrawAxes();
	afx_msg void OnOptionsUpdateDrawAxes(CCmdUI *pCmdUI);
	afx_msg void OnShadingWireframe();
	afx_msg void OnUpdateShadingWireframe(CCmdUI *pCmdUI);
	afx_msg void OnShadingNone();
	afx_msg void OnUpdateShadingNone(CCmdUI *pCmdUI);
	afx_msg void OnShadingPhong();
	afx_msg void OnUpdateShadingPhong(CCmdUI *pCmdUI);
	afx_msg void OnOptionsBackfacesculling();
	afx_msg void OnUpdateOptionsBackfacesculling(CCmdUI *pCmdUI);
	afx_msg void OnFileReset();
	afx_msg void OnRenderOnscreen();
	afx_msg void OnRenderTofile();
	afx_msg void OnShadingMaterialParams();
	afx_msg void OnDebugDepthrendering();
	afx_msg void OnUpdateDebugDepthrendering(CCmdUI *pCmdUI);
	afx_msg void OnDebugPerspectivecorrectrendering();
	afx_msg void OnUpdateDebugPerspectivecorrectrendering(CCmdUI *pCmdUI);
	afx_msg void OnBackgroundLoad();
	afx_msg void OnBackgroundReset();
	afx_msg void OnBackgroundStretch();
	afx_msg void OnBackgroundTile();
	afx_msg void OnUpdateBackgroundStretch(CCmdUI *pCmdUI);
	afx_msg void OnUpdateBackgroundTile(CCmdUI *pCmdUI);
	afx_msg void OnDebugInversevertexNormals();
	afx_msg void OnUpdateDebugInversevertexNormals(CCmdUI *pCmdUI);
	afx_msg void OnDebugInverseSurfaceNormals();
	afx_msg void OnDebugOptionsInverseSurfaceNormals(CCmdUI *pCmdUI);
	afx_msg void OnOptionsLightbackfaces();
	afx_msg void OnUpdateOptionsLightbackfaces(CCmdUI *pCmdUI);
	afx_msg void OnDebugLoadDebugmodel();
	afx_msg void OnDebugInvertdepth();
	afx_msg void OnUpdateDebugInvertdepth(CCmdUI *pCmdUI);
	afx_msg void OnDebugForceallfrontfaces();
	afx_msg void OnUpdateDebugForceallfrontfaces(CCmdUI *pCmdUI);
	afx_msg void OnOptionsDrawlightsources();
	afx_msg void OnUpdateOptionsDrawlightsources(CCmdUI *pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CG4VIEW_H__5857316D_EA60_11D5_9FD5_00D0B718E2CD__INCLUDED_)

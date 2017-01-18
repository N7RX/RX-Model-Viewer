#pragma once

#include "afxpriv.h"

#include "Model.h"
#include "TextureLoader.h"
#include "GLCamera.h"
#include "CUDA Accelerator.h"
#include "ObjLoader.h"
#include "StlLoader.h"
#include "RXM Importer.h"
#include "RXM Exporter.h"

using namespace std;


// Maximum thread number
#define MAX_THREAD 32

// OpenGL display window's size
#define WINDOW_WIDTH  1600
#define WINDOW_HEIGHT 900

// Animation parameter scale
#define ANIM_SCALE 0.01

// Shader file path
#define SKYBOX_VSHADER "shaders/vshader_skybox.glsl"
#define SKYBOX_FSHADER "shaders/fshader_skybox.glsl"
#define AXIS_VSHADER   "shaders/vshader_axis.glsl"
#define AXIS_FSHADER   "shaders/fshader_axis.glsl"

#define BASIC_VSHADER  "shaders/vshader_basic.glsl"
#define BASIC_FSHADER  "shaders/fshader_basic.glsl"
#define MEDIUM_VSHADER "shaders/vshader_medium.glsl"
#define MEDIUM_FSHADER "shaders/fshader_medium.glsl"
#define HIGH_VSHADER   "shaders/vshader_high.glsl"
#define HIGH_FSHADER   "shaders/fshader_high.glsl"

// Scene model file path
#define SKYBOX_MODEL   "metamodel/skybox_cube.obj"
#define AXIS_MODEL     "metamodel/axis.obj"

// CRXModelViewerDlg dialog
class CRXModelViewerDlg : public CDialogEx
{
// Construction
public:
	CRXModelViewerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RXMODELVIEWER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	HBRUSH m_blackbrush, m_whitebrush, m_cyanbrush, m_bluebrush, m_pwhitebrush; // Brushes

	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor); // Override control color funtion

	string modelFile;		// Model file path
	string diffuseMapFile;	// Diffuse map file path
	string specularMapFile; // Specular map file path
	string skyboxMapFile;	// Skybox map file path

	// OpenGL part functions that require MFC integration
	int  addNewModel(int argc, char **argv);
	void setupGLfunctions();
	void reviseNewModel();
	void setupSkybox();
	void addModelInfo();
	void addTextureInfo();
	void completeSceneImport(int newModelNum);
	void updateModelTexture(int modelIndex, int meshIndex, const char* type, const char* path);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnProgramClose();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonQuit();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnNMCustomdrawSliderThread(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderShininess(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonTexture();
	afx_msg void OnEnKillfocusEditLr();
	afx_msg void OnEnKillfocusEditLg();
	afx_msg void OnEnKillfocusEditLb();
	afx_msg void OnEnKillfocusEditDistance();
	afx_msg void OnEnKillfocusEditMr();
	afx_msg void OnEnKillfocusEditMg();
	afx_msg void OnEnKillfocusEditMb();
	afx_msg void OnEnKillfocusEditUoffset();
	afx_msg void OnEnKillfocusEditVoffset();
	afx_msg void OnEnKillfocusEditLpx();
	afx_msg void OnEnKillfocusEditLpy();
	afx_msg void OnEnKillfocusEditLpz();
	afx_msg void OnCbnSelchangeComboMove();
	afx_msg void OnCbnSelchangeComboTheme();
	afx_msg void OnEnKillfocusEditXscale();
	afx_msg void OnEnKillfocusEditYscale();
	afx_msg void OnEnKillfocusEditZscale();
	afx_msg void OnEnKillfocusEditUscale();
	afx_msg void OnEnKillfocusEditVscale();
	afx_msg void OnEnKillfocusEditXoffset();
	afx_msg void OnEnKillfocusEditYoffset();
	afx_msg void OnEnKillfocusEditZoffset();
	afx_msg void OnBnClickedButtonSpecular();
	afx_msg void OnEnKillfocusEditXrotate();
	afx_msg void OnEnKillfocusEditYrotate();
	afx_msg void OnEnKillfocusEditZrotate();
	afx_msg void OnBnClickedButtonSkybox();
	afx_msg void OnCbnSelchangeComboMindex2();
	afx_msg void OnBnClickedButtonAddanim();
	afx_msg void OnBnClickedButtonPauseanim();
	afx_msg void OnBnClickedCheckCuda();
	afx_msg void OnBnClickedCheckAa();
	afx_msg void OnBnClickedRadioAnim();
	afx_msg void OnBnClickedRadioTransform();
	afx_msg void OnEnKillfocusEditXpos();
	afx_msg void OnEnKillfocusEditYpos();
	afx_msg void OnEnKillfocusEditZpos();
	afx_msg void OnEnKillfocusEditXro();
	afx_msg void OnEnKillfocusEditYro();
	afx_msg void OnEnKillfocusEditZro();
	afx_msg void OnEnKillfocusEditXsca();
	afx_msg void OnEnKillfocusEditYsca();
	afx_msg void OnEnKillfocusEditZsca();
	afx_msg void OnCbnSelchangeComboMousefunc();
	afx_msg void OnNMCustomdrawSliderMsense(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckShadow();
	afx_msg void OnEnKillfocusEditUvsu();
	afx_msg void OnEnKillfocusEditUvsv();
	afx_msg void OnEnKillfocusEditUvshu();
	afx_msg void OnEnKillfocusEditUvshv();
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnCbnSelchangeComboDisplaymode();
	afx_msg void OnBnClickedCheckAxes();
	afx_msg void OnBnClickedCheckSkybox();
	afx_msg void OnEnKillfocusEditUvrotate();
	afx_msg void OnEnKillfocusEditUvr();
	afx_msg void OnBnClickedCheckReflect();
	afx_msg void OnCbnSelchangeComboMindex();
	afx_msg void OnBnClickedButtonDeletem();
	afx_msg void OnBnClickedCheckModelreflect();
	afx_msg void OnBnClickedCheckStg();
	afx_msg void OnBnClickedButtonExport();
	afx_msg void OnBnClickedButtonExportall();
	afx_msg void OnCbnSelchangeComboMeshindex();
	afx_msg void OnBnClickedCheckFog();
	afx_msg void OnCbnSelchangeComboShadow();
};

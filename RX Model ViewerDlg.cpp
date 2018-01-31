//-------------------------------------------------------------------------------------------------
// RX Model Viewer (OpenGL-based)
// Written by Raymond.X
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "RX Model Viewer.h"
#include "RX Model ViewerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

typedef vec4 point4;
typedef vec3 color3;
typedef vec4 color4;


#pragma region Shared Variables

// ----------------------------------

GLfloat aspect_rec = GLfloat(WINDOW_WIDTH) / WINDOW_HEIGHT; // Record aspect value

// Model import parameters
// ----------------------------------

vec3 import_position(0, 0, 0);
vec3 import_scale(1, 1, 1);
vec3 import_rotation(0, 0, 0);

vec2 import_uvscale(1, 1);
vec2 import_uvshift(0, 0);
GLfloat import_uvRotation = 0;

bool stick_to_ground = false;

// ----------------------------------

GLfloat zNear = 0.10f, zFar = 100.0f; // View frustum parameters
GLfloat zFarRatio = sqrt(3);		  // Cubic. Keep skybox insight

// Instantiate model parser component
// ----------------------------------

ObjLoader *objLoader = new ObjLoader();
StlLoader *stlLoader = new StlLoader();

RXMImporter *rxmImporter = new RXMImporter();
RXMExporter *rxmExporter = new RXMExporter();
				
// ----------------------------------

vector<Model> model; // Model objects

Mesh *skybox;	   // Skybox
Mesh *skybox_bak;  // Skybox backup for transformation
Mesh *axes;		   // Axes

bool first_display = true; // temporary fix for startup display

// ----------------------------------

// Shader programs
GLuint objectShader_basic;
GLuint objectShader_medium;
GLuint objectShader_high;
GLuint skyboxShader;
GLuint axesShader;

bool firstModel = true;     // Mark whether the scene is empty

bool skyboxSet = false;		// Mark whether skybox is set

// Texture data
// ----------------------------------

GLuint diffuseMap;
GLuint specularMap;
GLuint skyboxMap;

// ----------------------------------

// Environment light color. Default is white.
color3 light_color(255, 255, 255);

// Environment light position.
vec3   light_position(0, 0, 1);

// Scene background color. Default is black.
color3 background_color(0.14, 0.14, 0.14);

// Highlight color
color3 highlight_color(0, 183, 195);

// Fog color
color4 fog_color(0.7f, 0.7f, 0.7f, 1.0f);
float fog_density = 0.2f;

// ----------------------------------

int totalModelNum = 0;   // Current total imported model number
int totalTextureNum = 0; // Current total loaded texture number

int historyModelNum = 0; // Overall number of models imported

// ----------------------------------

int theme = 0;	  // Program appearance

// ----------------------------------

int moveMode = 1;  // 0: Edit model, 1: Move camera

char* drawMode = DRAW_FACE;  // Draw shaded face or wire frame

bool displayAxes = true;	 // Display axes or not
bool displaySkybox = false;  // Display skybox or not

// Transformation parameters
// ----------------------------------

int selectedModelIndex = -1;  // Model selected, -1 means all
int selectedMeshIndex = -1;   // Mesh selected, -1 means all

int prev_selected = -1;		  // Mark which model should de-highlight
bool all_highlighted = false; // Mark if all models are highlighted

int deletion_model_index = -1;// Mark which model should be deleted

bool playAnimation = true;	  // Animation play/pause

// ----------------------------------

int threadNum = MAX_THREAD; // Current thread number

bool enableCUDA = false;	// Use CUDA acceleration or not
bool enableAA = false;		// Use Anti-Aliasing or not
bool enableShadow = true;	// Use lighting or not
bool enableReflection = false; // Dispaly reflection material or not
bool enableFog = false;		// Use fog effect or not

int aaLevel = 0;			// Sampling level

// Camera parameters
// ----------------------------------

GLCamera *camera;				// Free camera

vec2  lastPos;					// Last mouse position
int   mouseButton = 0;			// 0-Left, 1-Right, 2-Middle
int   mouseFuncSet = 0;			// Defines mouse button function set
float mouseSensitivity = 1.0;	// Sensitivity

// ----------------------------------
#pragma endregion

// Function prototypes
void setEnvironmentLight(GLuint shader);
void adjustSkybox();
void translate(int modelIndex, int meshIndex, vec3 translation);
void rotate(int modelIndex, int meshIndex, vec3 axis, float value);
void rescale(int modelIndex, int meshIndex, vec3 scale);
void rescaleUV(int modelIndex, int meshIndex, vec2 scale);
void shiftUV(int modelIndex, int meshIndex, vec2 shift);
void rotateUV(int modelIndex, int meshIndex, float value);


////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////            MFC PART            ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region MFC System Functions

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRXModelViewerDlg dialog


CRXModelViewerDlg::CRXModelViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_RXMODELVIEWER_DIALOG, pParent)
{
	// Initialize variables
	modelFile = "";
	diffuseMapFile = "";
	specularMapFile = "";
	skyboxMapFile = "";

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRXModelViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

// MFC message maps
BEGIN_MESSAGE_MAP(CRXModelViewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR(WM_CTLCOLOR, &CRXModelViewerDlg::OnCtlColor)
	ON_MESSAGE_VOID(WM_CLOSE, OnProgramClose)

	ON_BN_CLICKED(IDC_BUTTON_QUIT, &CRXModelViewerDlg::OnBnClickedButtonQuit)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CRXModelViewerDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CRXModelViewerDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_TEXTURE, &CRXModelViewerDlg::OnBnClickedButtonTexture)
	ON_BN_CLICKED(IDC_BUTTON_SPECULAR, &CRXModelViewerDlg::OnBnClickedButtonSpecular)
	ON_BN_CLICKED(IDC_BUTTON_SKYBOX, &CRXModelViewerDlg::OnBnClickedButtonSkybox)
	ON_BN_CLICKED(IDC_BUTTON_ADDANIM, &CRXModelViewerDlg::OnBnClickedButtonAddanim)
	ON_BN_CLICKED(IDC_BUTTON_PAUSEANIM, &CRXModelViewerDlg::OnBnClickedButtonPauseanim)
	ON_BN_CLICKED(IDC_CHECK_CUDA, &CRXModelViewerDlg::OnBnClickedCheckCuda)
	ON_BN_CLICKED(IDC_CHECK_AA, &CRXModelViewerDlg::OnBnClickedCheckAa)
	ON_BN_CLICKED(IDC_RADIO_ANIM, &CRXModelViewerDlg::OnBnClickedRadioAnim)
	ON_BN_CLICKED(IDC_RADIO_TRANSFORM, &CRXModelViewerDlg::OnBnClickedRadioTransform)
	ON_BN_CLICKED(IDC_CHECK_SHADOW, &CRXModelViewerDlg::OnBnClickedCheckShadow)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CRXModelViewerDlg::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_CHECK_AXES, &CRXModelViewerDlg::OnBnClickedCheckAxes)
	ON_BN_CLICKED(IDC_CHECK_SKYBOX, &CRXModelViewerDlg::OnBnClickedCheckSkybox)
	ON_BN_CLICKED(IDC_CHECK_REFLECT, &CRXModelViewerDlg::OnBnClickedCheckReflect)
	ON_BN_CLICKED(IDC_BUTTON_DELETEM, &CRXModelViewerDlg::OnBnClickedButtonDeletem)
	ON_BN_CLICKED(IDC_CHECK_MODELREFLECT, &CRXModelViewerDlg::OnBnClickedCheckModelreflect)
	ON_BN_CLICKED(IDC_CHECK_STG, &CRXModelViewerDlg::OnBnClickedCheckStg)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &CRXModelViewerDlg::OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_EXPORTALL, &CRXModelViewerDlg::OnBnClickedButtonExportall)
	ON_BN_CLICKED(IDC_CHECK_FOG, &CRXModelViewerDlg::OnBnClickedCheckFog)

	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_THREAD, &CRXModelViewerDlg::OnNMCustomdrawSliderThread)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SHININESS, &CRXModelViewerDlg::OnNMCustomdrawSliderShininess)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_MSENSE, &CRXModelViewerDlg::OnNMCustomdrawSliderMsense)

	ON_CBN_SELCHANGE(IDC_COMBO_MOVE, &CRXModelViewerDlg::OnCbnSelchangeComboMove)
	ON_CBN_SELCHANGE(IDC_COMBO_THEME, &CRXModelViewerDlg::OnCbnSelchangeComboTheme)
	ON_CBN_SELCHANGE(IDC_COMBO_MINDEX2, &CRXModelViewerDlg::OnCbnSelchangeComboMindex2)
	ON_CBN_SELCHANGE(IDC_COMBO_MOUSEFUNC, &CRXModelViewerDlg::OnCbnSelchangeComboMousefunc)
	ON_CBN_SELCHANGE(IDC_COMBO_DISPLAYMODE, &CRXModelViewerDlg::OnCbnSelchangeComboDisplaymode)
	ON_CBN_SELCHANGE(IDC_COMBO_MINDEX, &CRXModelViewerDlg::OnCbnSelchangeComboMindex)
	ON_CBN_SELCHANGE(IDC_COMBO_MESHINDEX, &CRXModelViewerDlg::OnCbnSelchangeComboMeshindex)

	ON_EN_KILLFOCUS(IDC_EDIT_LR, &CRXModelViewerDlg::OnEnKillfocusEditLr)
	ON_EN_KILLFOCUS(IDC_EDIT_LG, &CRXModelViewerDlg::OnEnKillfocusEditLg)
	ON_EN_KILLFOCUS(IDC_EDIT_LB, &CRXModelViewerDlg::OnEnKillfocusEditLb)
	ON_EN_KILLFOCUS(IDC_EDIT_DISTANCE, &CRXModelViewerDlg::OnEnKillfocusEditDistance)
	ON_EN_KILLFOCUS(IDC_EDIT_MR, &CRXModelViewerDlg::OnEnKillfocusEditMr)
	ON_EN_KILLFOCUS(IDC_EDIT_MG, &CRXModelViewerDlg::OnEnKillfocusEditMg)
	ON_EN_KILLFOCUS(IDC_EDIT_MB, &CRXModelViewerDlg::OnEnKillfocusEditMb)
	ON_EN_KILLFOCUS(IDC_EDIT_UOFFSET, &CRXModelViewerDlg::OnEnKillfocusEditUoffset)
	ON_EN_KILLFOCUS(IDC_EDIT_VOFFSET, &CRXModelViewerDlg::OnEnKillfocusEditVoffset)
	ON_EN_KILLFOCUS(IDC_EDIT_LPX, &CRXModelViewerDlg::OnEnKillfocusEditLpx)
	ON_EN_KILLFOCUS(IDC_EDIT_LPY, &CRXModelViewerDlg::OnEnKillfocusEditLpy)
	ON_EN_KILLFOCUS(IDC_EDIT_LPZ, &CRXModelViewerDlg::OnEnKillfocusEditLpz)
	ON_EN_KILLFOCUS(IDC_EDIT_XSCALE, &CRXModelViewerDlg::OnEnKillfocusEditXscale)
	ON_EN_KILLFOCUS(IDC_EDIT_YSCALE, &CRXModelViewerDlg::OnEnKillfocusEditYscale)
	ON_EN_KILLFOCUS(IDC_EDIT_ZSCALE, &CRXModelViewerDlg::OnEnKillfocusEditZscale)
	ON_EN_KILLFOCUS(IDC_EDIT_USCALE, &CRXModelViewerDlg::OnEnKillfocusEditUscale)
	ON_EN_KILLFOCUS(IDC_EDIT_VSCALE, &CRXModelViewerDlg::OnEnKillfocusEditVscale)
	ON_EN_KILLFOCUS(IDC_EDIT_XOFFSET, &CRXModelViewerDlg::OnEnKillfocusEditXoffset)
	ON_EN_KILLFOCUS(IDC_EDIT_YOFFSET, &CRXModelViewerDlg::OnEnKillfocusEditYoffset)
	ON_EN_KILLFOCUS(IDC_EDIT_ZOFFSET, &CRXModelViewerDlg::OnEnKillfocusEditZoffset)
	ON_EN_KILLFOCUS(IDC_EDIT_XROTATE, &CRXModelViewerDlg::OnEnKillfocusEditXrotate)
	ON_EN_KILLFOCUS(IDC_EDIT_YROTATE, &CRXModelViewerDlg::OnEnKillfocusEditYrotate)
	ON_EN_KILLFOCUS(IDC_EDIT_ZROTATE, &CRXModelViewerDlg::OnEnKillfocusEditZrotate)
	ON_EN_KILLFOCUS(IDC_EDIT_XPOS, &CRXModelViewerDlg::OnEnKillfocusEditXpos)
	ON_EN_KILLFOCUS(IDC_EDIT_YPOS, &CRXModelViewerDlg::OnEnKillfocusEditYpos)
	ON_EN_KILLFOCUS(IDC_EDIT_ZPOS, &CRXModelViewerDlg::OnEnKillfocusEditZpos)
	ON_EN_KILLFOCUS(IDC_EDIT_XRO, &CRXModelViewerDlg::OnEnKillfocusEditXro)
	ON_EN_KILLFOCUS(IDC_EDIT_YRO, &CRXModelViewerDlg::OnEnKillfocusEditYro)
	ON_EN_KILLFOCUS(IDC_EDIT_ZRO, &CRXModelViewerDlg::OnEnKillfocusEditZro)
	ON_EN_KILLFOCUS(IDC_EDIT_XSCA, &CRXModelViewerDlg::OnEnKillfocusEditXsca)
	ON_EN_KILLFOCUS(IDC_EDIT_YSCA, &CRXModelViewerDlg::OnEnKillfocusEditYsca)
	ON_EN_KILLFOCUS(IDC_EDIT_ZSCA, &CRXModelViewerDlg::OnEnKillfocusEditZsca)
	ON_EN_KILLFOCUS(IDC_EDIT_UVSU, &CRXModelViewerDlg::OnEnKillfocusEditUvsu)
	ON_EN_KILLFOCUS(IDC_EDIT_UVSV, &CRXModelViewerDlg::OnEnKillfocusEditUvsv)
	ON_EN_KILLFOCUS(IDC_EDIT_UVSHU, &CRXModelViewerDlg::OnEnKillfocusEditUvshu)
	ON_EN_KILLFOCUS(IDC_EDIT_UVSHV, &CRXModelViewerDlg::OnEnKillfocusEditUvshv)
	ON_EN_KILLFOCUS(IDC_EDIT_UVROTATE, &CRXModelViewerDlg::OnEnKillfocusEditUvrotate)
	ON_EN_KILLFOCUS(IDC_EDIT_UVR, &CRXModelViewerDlg::OnEnKillfocusEditUvr)

END_MESSAGE_MAP()


// CRXModelViewerDlg message handlers

BOOL CRXModelViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize UI paint brush
	m_blackbrush = CreateSolidBrush(RGB(36, 36, 36));
	m_whitebrush = CreateSolidBrush(RGB(240, 240, 240));
	m_cyanbrush = CreateSolidBrush(RGB(0, 183, 195));
	m_bluebrush = CreateSolidBrush(RGB(64, 64, 100));
	m_pwhitebrush = CreateSolidBrush(RGB(255, 255, 255));

	// Initialize dialog components
#pragma region Contents

	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREAD);
	slider->SetRange(1, MAX_THREAD, TRUE);
	slider->SetPos(MAX_THREAD);

	slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SHININESS);
	slider->SetRange(0, 100, TRUE);
	slider->SetPos(0);

	slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_AA);
	slider->SetRange(1, 4, TRUE);
	slider->SetPos(1);

	slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_MSENSE);
	slider->SetRange(1, 5, TRUE);
	slider->SetPos(3);

	CEdit* editBox;
	editBox = (CEdit*)GetDlgItem(IDC_EDIT_XSCALE);
	editBox->SetWindowText(_T("1.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_YSCALE);
	editBox->SetWindowText(_T("1.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_ZSCALE);
	editBox->SetWindowText(_T("1.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_DISTANCE);
	editBox->SetWindowText(_T("100.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_UOFFSET);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_VOFFSET);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_LR);
	editBox->SetWindowText(_T("255"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_LG);
	editBox->SetWindowText(_T("255"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_LB);
	editBox->SetWindowText(_T("255"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_LPX);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_LPY);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_LPZ);
	editBox->SetWindowText(_T("1.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_URATIO);
	editBox->SetWindowText(_T("1.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_VRATIO);
	editBox->SetWindowText(_T("1.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_XOFFSET);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_YOFFSET);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_ZOFFSET);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_XLIMIT);
	editBox->SetWindowText(_T("Not Set"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_YLIMIT);
	editBox->SetWindowText(_T("Not Set"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_ZLIMIT);
	editBox->SetWindowText(_T("Not Set"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_XROTATE);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_YROTATE);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_ZROTATE);
	editBox->SetWindowText(_T("0.00"));

	editBox = (CEdit*)GetDlgItem(IDC_EDIT_UVROTATE);
	editBox->SetWindowText(_T("0.00"));

	CComboBox* cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_SHADOW);
	cmb_function->AddString(_T("Hard Shadow"));
	cmb_function->SetCurSel(0);

	cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_MOVE));
	cmb_function->AddString(_T("Move Model"));
	cmb_function->AddString(_T("Camera"));
	cmb_function->SetCurSel(1);

	cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_MAPTYPE));
	cmb_function->AddString(_T("UV Coordinates"));
	cmb_function->SetCurSel(0);

	cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_THEME));
	cmb_function->AddString(_T("RX Black"));
	cmb_function->AddString(_T("RX White"));
	cmb_function->SetCurSel(0);

	cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_MINDEX2));
	cmb_function->AddString(_T("All Models"));
	cmb_function->SetCurSel(0);

	cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_MOUSEFUNC));
	cmb_function->AddString(_T("Function Set 1"));
	cmb_function->AddString(_T("Function Set 2"));
	cmb_function->SetCurSel(0);

	cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_DISPLAYMODE));
	cmb_function->AddString(_T("Shaded Model"));
	cmb_function->AddString(_T("Wire Frame"));
	cmb_function->SetCurSel(0);

	CButton *checkBtn = (CButton*)GetDlgItem(IDC_CHECK_AREPEAT);
	checkBtn->SetCheck(1);

	checkBtn = (CButton*)GetDlgItem(IDC_CHECK_SHADOW);
	checkBtn->SetCheck(1);

	checkBtn = (CButton*)GetDlgItem(IDC_RADIO_ANIM);
	checkBtn->SetCheck(1);

	checkBtn = (CButton*)GetDlgItem(IDC_CHECK_AXES);
	checkBtn->SetCheck(1);

#pragma endregion

#pragma region StartupInfo
	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
	CString str; CTime tm;
	tm = CTime::GetCurrentTime();
	str = tm.Format("Program Initiated. %m/%d/%Y %X"); // Display system time
	outputBox->AddString(str);
	outputBox->AddString(_T("Product Version: 1.5.4.1. Feature Level 3"));
	outputBox->AddString(_T("Supported Parsers: ObjLoader; StlLoader; RXMImporter; RXMExporter"));
	outputBox->AddString(_T("Installed Mods: CUDA Accelerator;"));
	outputBox->AddString(_T("API: OpenGL 4.5"));
	outputBox->AddString(_T("Shader Versions Supported: Vertex: 3.3; Fragment 3.3;"));
	outputBox->AddString(_T("> This is the internal beta version. Functions are limited, still under development."));
	outputBox->AddString(_T("--------------------------------------------------------------------------"));
	outputBox->AddString(_T("CUDA Acceleration requires CUDA support."));
	outputBox->AddString(_T("-------------------"));
	outputBox->AddString(_T("Under Move Camera Mode:"));
	outputBox->AddString(_T("Press 'W''A''S''D'+'Q''E' to move camera."));
	outputBox->AddString(_T("Press 'X''Y''Z' to rotate camera. (CapsLock Inverse)"));
	outputBox->AddString(_T("Press 'M' to zoom in/out. (CapsLock Inverse)"));
	outputBox->AddString(_T("Press 'Space' to reset camera."));
	outputBox->AddString(_T("-------------------"));
	outputBox->AddString(_T("Under Edit Model Mode:"));
	outputBox->AddString(_T("Press 'W''A''S''D'+'Q''E' to move model."));
	outputBox->AddString(_T("Press 'X''Y''Z' to rotate model. (CapsLock Inverse)"));
	outputBox->AddString(_T("Press 'M' to scale up/down model. (CapsLock Inverse)"));
	outputBox->AddString(_T("-------------------"));
	outputBox->AddString(_T("> More features will be added in the future."));
	outputBox->AddString(_T("-------------------------------------"));
#pragma endregion

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRXModelViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRXModelViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// Set window's background color
		if (theme == 0) {
			CRect rect;
			CPaintDC dc(this);
			GetClientRect(rect);
			dc.FillSolidRect(rect, RGB(36, 36, 36));
		}

		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRXModelViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Override OnClose() funtion
void CRXModelViewerDlg::OnProgramClose()
{
	CString notifStr("Quit RX Model Viewer?");
	if (AfxMessageBox(notifStr, MB_OKCANCEL) == IDOK) {
		// Release garbage
		model.swap(vector<Model>());
		delete camera;
		delete objLoader;
		delete stlLoader;
		delete rxmImporter;
		delete rxmExporter;
		delete skybox;
		delete skybox_bak;
		delete axes;

		this->OnClose();
	}
}

// Override color brush function, customize components color
HBRUSH CRXModelViewerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID()) {
	// Statics and digits
	case IDC_STATIC: case IDC_STATIC_THREAD: case IDC_STATIC_SHININESS: case IDC_STATIC_AA: 
	case IDC_STATIC_AF: case IDC_STATIC_TS: case IDC_STATIC_TDL: case IDC_STATIC_RS: 
	case IDC_STATIC_RSX: case IDC_STATIC_RSY: case IDC_STATIC_RSZ: case IDC_STATIC_MS: 
	case IDC_STATIC_S: case IDC_STATIC_SX: case IDC_STATIC_SY: case IDC_STATIC_SZ: 
	case IDC_STATIC_UVS: case IDC_STATIC_UVSU: case IDC_STATIC_UVSV: case IDC_STATIC_UVSH: 
	case IDC_STATIC_UVSHU: case IDC_STATIC_UVSHV: case IDC_STATIC_UVR: case IDC_STATIC_TDLX: 
	case IDC_STATIC_TDLY: case IDC_STATIC_TDLZ: case IDC_STATIC_TSX: case IDC_STATIC_TSY: 
	case IDC_STATIC_TSZ: case IDC_STATIC_POS: case IDC_STATIC_ROT: case IDC_STATIC_POSX: 
	case IDC_STATIC_POSY: case IDC_STATIC_POSZ: case IDC_STATIC_ROTX: case IDC_STATIC_ROTY: 
	case IDC_STATIC_ROTZ: {
		if (theme == 0) {
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(255, 255, 255));
			hbr = (HBRUSH)m_blackbrush;
		}
		if (theme == 1) {
			pDC->SetBkMode(OPAQUE);
			pDC->SetTextColor(RGB(0, 0, 0));
			hbr = (HBRUSH)m_whitebrush;
		}
		break;
	}
	// Group boxes
	case IDC_STATIC_ISETTINGS: case IDC_STATIC_ASETTINGS: case IDC_STATIC_OUTPUT: case IDC_STATIC_MCONTROL: {
		hbr = (HBRUSH)m_cyanbrush;
		break;
	}
	// Sliders
	case IDC_SLIDER_THREAD: case IDC_SLIDER_SHININESS: case IDC_SLIDER_AA: case IDC_SLIDER_AF:
	case IDC_SLIDER_MSENSE: {
		if (theme == 0)
			hbr = (HBRUSH)m_blackbrush;
		if (theme == 1)
			hbr = (HBRUSH)m_whitebrush;
		break;
	}
	// Check boxes
	case IDC_CHECK_AF: case IDC_CHECK_HDR: case IDC_CHECK_GILLUMINATION: case IDC_CHECK_SSAO: 
	case IDC_CHECK_GAMMA: case IDC_CHECK_BLOOM: case IDC_CHECK_AREPEAT: case IDC_CHECK_DOF: {
		if (theme == 0) {
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(255, 255, 255));
			hbr = (HBRUSH)m_blackbrush;
		}
		if (theme == 1) {
			pDC->SetBkMode(OPAQUE);
			pDC->SetTextColor(RGB(0, 0, 0));
			hbr = (HBRUSH)m_whitebrush;
		}
		break;
	}
	// Enabled check boxes
	case IDC_CHECK_AA: case IDC_CHECK_SHADOW: case IDC_RADIO_ANIM: case IDC_RADIO_TRANSFORM: 
	case IDC_CHECK_CUDA: case IDC_CHECK_AXES: case IDC_CHECK_SKYBOX: case IDC_CHECK_REFLECT: 
	case IDC_CHECK_MODELREFLECT: case IDC_CHECK_STG: case IDC_CHECK_FOG: {
		if (theme == 0) {
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(255, 255, 255));
			hbr = (HBRUSH)m_bluebrush;
		}
		if (theme == 1) {
			pDC->SetBkMode(OPAQUE);
			pDC->SetTextColor(RGB(0, 0, 0));
			hbr = (HBRUSH)m_whitebrush;
		}
		break;
	}
	default: break;
	}

	return hbr;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region MFC Control Components

// Close program
void CRXModelViewerDlg::OnBnClickedButtonQuit()
{
	AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
}

// About menu
void CRXModelViewerDlg::OnBnClickedButtonAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

// Select model file
void CRXModelViewerDlg::OnBnClickedButtonOpen()
{
	// Select file dialog
	TCHAR  szFilter[] = _T("OBJ File (*.obj)|*.obj|STL File (*.stl)|*.stl|RXM File (*.rxm)|*.rxm|RXM Scene File (*rxms)|*.rxms|"); // Supported formats
	CFileDialog fileOpenDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDlg.DoModal() == IDOK)
	{
		CString imFilePath;
		VERIFY(imFilePath = fileOpenDlg.GetPathName());

		modelFile = CT2A(imFilePath.GetBuffer());

		// Retrieve system arguments
		LPTSTR cmd = ::GetCommandLine();
		int argc = 0;
		WCHAR *const *argv;
		argv = ::CommandLineToArgvW(cmd, &argc);

		addNewModel(argc, (char**)argv); // Boot up in OpenGL

		this->Invalidate();
	}
}

// Select diffuse texture file
void CRXModelViewerDlg::OnBnClickedButtonTexture()
{
	// Select file dialog
	TCHAR  szFilter[] = _T("JPEG File (*.jpg)|*.jpg|PNG File (*.png)|*.png|BMP File (*.bmp)|*.bmp|"); // More format suppoted by SOIL
	CFileDialog fileOpenDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDlg.DoModal() == IDOK)
	{
		CString imFilePath;
		VERIFY(imFilePath = fileOpenDlg.GetPathName());

		diffuseMapFile = CT2A(imFilePath.GetBuffer());

		// Get image resolution
		int width, height;
		unsigned char* image;
		image = SOIL_load_image(diffuseMapFile.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		SOIL_free_image_data(image);

		// Update selected model
		if (!firstModel && selectedMeshIndex >= 0) {
			updateModelTexture(selectedModelIndex, selectedMeshIndex, DIFFUSE, diffuseMapFile.c_str());
			glutPostRedisplay();
		}

		CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
		outputBox->AddString(_T(">-------------------------------------"));
		CString text("Diffuse Texture Added.");
		outputBox->AddString(text);
		text = diffuseMapFile.c_str();
		outputBox->AddString(text);
		text.Format(_T("Resolution: %d × %d"), width, height);
		outputBox->AddString(text);

		this->Invalidate();
	}
}

// Select specular texture file
void CRXModelViewerDlg::OnBnClickedButtonSpecular()
{
	// Select file dialog
	TCHAR  szFilter[] = _T("JPEG File (*.jpg)|*.jpg|PNG File (*.png)|*.png|BMP File (*.bmp)|*.bmp|"); // More format suppoted by SOIL
	CFileDialog fileOpenDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDlg.DoModal() == IDOK)
	{
		CString imFilePath;
		VERIFY(imFilePath = fileOpenDlg.GetPathName());

		specularMapFile = CT2A(imFilePath.GetBuffer());

		// Update selected model
		if (!firstModel &&selectedMeshIndex >= 0) {
			updateModelTexture(selectedModelIndex, selectedMeshIndex, SPECULAR, specularMapFile.c_str());
			glutPostRedisplay();
		}

		// Get image resolution
		int width, height;
		unsigned char* image;
		image = SOIL_load_image(specularMapFile.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		SOIL_free_image_data(image);

		CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
		outputBox->AddString(_T(">-------------------------------------"));
		CString text("Specular Map Added.");
		outputBox->AddString(text);
		text = specularMapFile.c_str();
		outputBox->AddString(text);
		text.Format(_T("Resolution: %d × %d"), width, height);
		outputBox->AddString(text);

		this->Invalidate();
	}
}

// Change thread number
void CRXModelViewerDlg::OnNMCustomdrawSliderThread(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREAD);
	threadNum = slider->GetPos();
	CString text("");
	text.Format(_T("%d"), threadNum);
	GetDlgItem(IDC_STATIC_THREAD)->SetWindowText(text);
	*pResult = 0;
}

// Change material shininess
void CRXModelViewerDlg::OnNMCustomdrawSliderShininess(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SHININESS);
	float md_shininess = slider->GetPos();
	if (!firstModel && selectedMeshIndex >= 0 && selectedModelIndex >= 0) { // Update current scene
		model[selectedModelIndex].mesh[selectedMeshIndex].setupMaterial(
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[0],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[1],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[2],
			md_shininess);
		glutPostRedisplay();
	}
	CString text("");
	text.Format(_T("%d"), (int)md_shininess);
	GetDlgItem(IDC_STATIC_SHININESS)->SetWindowText(text);
	*pResult = 0;
}

// Change model import ratio
void CRXModelViewerDlg::OnEnKillfocusEditXscale()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XSCALE);
	CString str;
	eBox->GetWindowText(str);
	float ratio; int flag = 0;
	if (ratio = _tstof(str)) {
		if (ratio > 0) {
			import_scale.x = ratio;
			flag = 1;
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_scale.x);
		eBox->SetWindowText(str);
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditYscale()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_YSCALE);
	CString str;
	eBox->GetWindowText(str);
	float ratio; int flag = 0;
	if (ratio = _tstof(str)) {
		if (ratio > 0) {
			import_scale.y = ratio;
			flag = 1;
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_scale.y);
		eBox->SetWindowText(str);
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditZscale()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZSCALE);
	CString str;
	eBox->GetWindowText(str);
	float ratio; int flag = 0;
	if (ratio = _tstof(str)) {
		if (ratio > 0) {
			import_scale.z = ratio;
			flag = 1;
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_scale.z);
		eBox->SetWindowText(str);
	}
}

// Change view distance
void CRXModelViewerDlg::OnEnKillfocusEditDistance()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_DISTANCE);
	CString str;
	eBox->GetWindowText(str);
	float distance; int flag = 0;
	if (distance = _tstof(str)) {
		if (distance > zNear) {
			flag = 1;

			// Update skybox's size
			if (skyboxSet) {
				float ratio = (distance / zFar);
				for (int i = 0; i < skybox->vertices.size(); i++) {
					skybox->vertices[i].position = skybox_bak->vertices[i].position * vec4(ratio, ratio, ratio, 1);
					skybox_bak->vertices[i].position = skybox->vertices[i].position;
				}
				skybox->setupBuffers();
			}

			zFar = distance;

			if (!firstModel) { // Re-display skybox
				glutPostRedisplay();
			}
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), zFar);
		eBox->SetWindowText(str);
	}
}

// Change environment light color
void CRXModelViewerDlg::OnEnKillfocusEditLr()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_LR);
	CString str;
	eBox->GetWindowText(str);
	int color = 0, flag = 0;
	if (color = _tstoi(str)) {
		if (color >= 0 && color <= 255) {
			light_color[0] = color;
			flag = 1;
		}
	}
	else if (str == _T("0")) { // "0" would fail in _tstoi()
		light_color[0] = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%d"), light_color[0]);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel) { // Update current scene
		setEnvironmentLight(objectShader_basic);
		setEnvironmentLight(objectShader_medium);
		setEnvironmentLight(objectShader_high);
		glutPostRedisplay();
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditLg()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_LG);
	CString str;
	eBox->GetWindowText(str);
	int color = 0, flag = 0;
	if (color = _tstoi(str)) {
		if (color >= 0 && color <= 255) {
			light_color[1] = color;
			flag = 1;
		}
	}
	else if (str == _T("0")) {
		light_color[1] = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%d"), light_color[1]);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel) {
		setEnvironmentLight(objectShader_basic);
		setEnvironmentLight(objectShader_medium);
		setEnvironmentLight(objectShader_high);
		glutPostRedisplay();
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditLb()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_LB);
	CString str;
	eBox->GetWindowText(str);
	int color = 0, flag = 0;
	if (color = _tstoi(str)) {
		if (color >= 0 && color <= 255) {
			light_color[2] = color;
			flag = 1;
		}
	}
	else if (str == _T("0")) {
		light_color[2] = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%d"), light_color[2]);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel) {
		setEnvironmentLight(objectShader_basic);
		setEnvironmentLight(objectShader_medium);
		setEnvironmentLight(objectShader_high);
		glutPostRedisplay();
	}
}

// Change model color
void CRXModelViewerDlg::OnEnKillfocusEditMr()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_MR);
	CString str;
	eBox->GetWindowText(str);
	int color = 0, flag = 0;
	if (color = _tstoi(str)) {
		if (color >= 0 && color <= 255) {
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[0] = color;
			flag = 1;
		}
	}
	else if (str == _T("0")) {
		model[selectedModelIndex].mesh[selectedMeshIndex].material_color[0] = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%d"), (int)model[selectedModelIndex].mesh[selectedMeshIndex].material_color[0]);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel && selectedMeshIndex >= 0) {
		model[selectedModelIndex].mesh[selectedMeshIndex].highlight(true, highlight_color);
		model[selectedModelIndex].mesh[selectedMeshIndex].setupMaterial( // Update color
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[0],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[1],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[2],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_shininess);
		glutPostRedisplay();
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditMg()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_MG);
	CString str;
	eBox->GetWindowText(str);
	int color = 0, flag = 0;
	if (color = _tstoi(str)) {
		if (color >= 0 && color <= 255) {
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[1] = color;
			flag = 1;
		}
	}
	else if (str == _T("0")) {
		model[selectedModelIndex].mesh[selectedMeshIndex].material_color[1] = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%d"), (int)model[selectedModelIndex].mesh[selectedMeshIndex].material_color[1]);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel && selectedMeshIndex >= 0) {
		model[selectedModelIndex].mesh[selectedMeshIndex].highlight(true, highlight_color);
		model[selectedModelIndex].mesh[selectedMeshIndex].setupMaterial(
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[0],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[1],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[2],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_shininess);
		glutPostRedisplay();
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditMb()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_MB);
	CString str;
	eBox->GetWindowText(str);
	int color = 0, flag = 0;
	if (color = _tstoi(str)) {
		if (color >= 0 && color <= 255) {
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[2] = color;
			flag = 1;
		}
	}
	else if (str == _T("0")) {
		model[selectedModelIndex].mesh[selectedMeshIndex].material_color[2] = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%d"), (int)model[selectedModelIndex].mesh[selectedMeshIndex].material_color[2]);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel && selectedMeshIndex >= 0) {
		model[selectedModelIndex].mesh[selectedMeshIndex].highlight(true, highlight_color);
		model[selectedModelIndex].mesh[selectedMeshIndex].setupMaterial(
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[0],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[1],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_color[2],
			model[selectedModelIndex].mesh[selectedMeshIndex].material_shininess);
		glutPostRedisplay();
	}
}

// Change UV offsets
void CRXModelViewerDlg::OnEnKillfocusEditUoffset()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_UOFFSET);
	CString str;
	eBox->GetWindowText(str);
	float offset = 0, flag = 0;
	if (offset = _tstof(str)) {
		import_uvshift[0] = offset;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_uvshift[0] = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_uvshift[0]);
		eBox->SetWindowText(str);
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditVoffset()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_VOFFSET);
	CString str;
	eBox->GetWindowText(str);
	float offset = 0, flag = 0;
	if (offset = _tstof(str)) {
		import_uvshift[1] = offset;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_uvshift[1] = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_uvshift[1]);
		eBox->SetWindowText(str);
	}
}

// Change light position
void CRXModelViewerDlg::OnEnKillfocusEditLpx()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_LPX);
	CString str;
	eBox->GetWindowText(str);
	float position = 0, flag = 0;
	if (position = _tstof(str)) {
		light_position.x = position;
		flag = 1;
	}
	else if (str == _T("0")) {
		light_position.x = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), light_position.x);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel) {
		setEnvironmentLight(objectShader_basic);
		setEnvironmentLight(objectShader_medium);
		setEnvironmentLight(objectShader_high);
		glutPostRedisplay();
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditLpy()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_LPY);
	CString str;
	eBox->GetWindowText(str);
	float position = 0, flag = 0;
	if (position = _tstof(str)) {
		light_position.y = position;
		flag = 1;
	}
	else if (str == _T("0")) {
		light_position.y = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), light_position.y);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel) {
		setEnvironmentLight(objectShader_basic);
		setEnvironmentLight(objectShader_medium);
		setEnvironmentLight(objectShader_high);
		glutPostRedisplay();
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditLpz()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_LPZ);
	CString str;
	eBox->GetWindowText(str);
	float position = 0, flag = 0;
	if (position = _tstof(str)) {
		light_position.z = position;
		flag = 1;
	}
	else if (str == _T("0")) {
		light_position.z = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), light_position.z);
		eBox->SetWindowText(str);
	}
	if (flag == 1 && !firstModel) {
		setEnvironmentLight(objectShader_basic);
		setEnvironmentLight(objectShader_medium);
		setEnvironmentLight(objectShader_high);
		glutPostRedisplay();
	}
}

// Change movement mode
void CRXModelViewerDlg::OnCbnSelchangeComboMove()
{
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_MOVE));
	moveMode = cmb_function->GetCurSel();

	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
	outputBox->AddString(_T(">-------------------------------------"));
	if (moveMode == 0) {
		if (selectedModelIndex < 0) { // Highlight all models
			for (int i = 0; i < model.size(); i++) {
				model[i].highlightModel(false, highlight_color);
			}
			all_highlighted = true;
			glutPostRedisplay();
		}
		if (selectedModelIndex >= 0 && selectedMeshIndex < 0) { // Highlight selected model
			model[selectedModelIndex].highlightModel(false, highlight_color);
		}
		outputBox->AddString(_T("View Mode Set To: Move Model"));
		outputBox->AddString(_T("Under this mode, your model's position will be transformed."));
	}
	else {
		if (all_highlighted) {
			for (int i = 0; i < model.size(); i++) {
				model[i].highlightModel(true, highlight_color);
			}
			all_highlighted = false;
			glutPostRedisplay();
		}
		outputBox->AddString(_T("View Mode Set To: Camera"));
		outputBox->AddString(_T("Under this mode, your model's position will not be changed."));
	}
}

// Change theme
void CRXModelViewerDlg::OnCbnSelchangeComboTheme()
{
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_THEME));
	theme = cmb_function->GetCurSel();
	// Change scene background
	if (!firstModel) {
		if (theme == 0) glClearColor(0.14, 0.14, 0.14, 1.0);
		if (theme == 1) glClearColor(0.95, 0.95, 0.95, 1.0);
		glutPostRedisplay();
	}
	else {
		if (theme == 0) background_color = color3(0.14, 0.14, 0.14);
		if (theme == 1) background_color = color3(0.95, 0.95, 0.95);
	}
	this->Invalidate(); // Refresh window
}

// Change UV import scale
void CRXModelViewerDlg::OnEnKillfocusEditUscale()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_USCALE);
	CString str;
	eBox->GetWindowText(str);
	float ratio; int flag = 0;
	if (ratio = _tstof(str)) {
		if (ratio > 0) {
			import_uvscale[0] = ratio;
			flag = 1;
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_uvscale[0]);
		eBox->SetWindowText(str);
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditVscale()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_VSCALE);
	CString str;
	eBox->GetWindowText(str);
	float ratio; int flag = 0;
	if (ratio = _tstof(str)) {
		if (ratio > 0) {
			import_uvscale[1] = ratio;
			flag = 1;
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_uvscale[1]);
		eBox->SetWindowText(str);
	}
}

// Change model import offsets
void CRXModelViewerDlg::OnEnKillfocusEditXoffset()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XOFFSET);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		import_position.x = offset;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_position.x = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_position.x);
		eBox->SetWindowText(str);
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditYoffset()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_YOFFSET);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		import_position.y = offset;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_position.y = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_position.y);
		eBox->SetWindowText(str);
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditZoffset()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZOFFSET);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		import_position.z = offset;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_position.z = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_position.z);
		eBox->SetWindowText(str);
	}
}

// Change model import rotations
void CRXModelViewerDlg::OnEnKillfocusEditXrotate()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XROTATE);
	CString str;
	eBox->GetWindowText(str);
	float angle; int flag = 0;
	if (angle = _tstof(str)) {
		import_rotation.x = angle*DegreesToRadians;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_rotation.x = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_rotation.x / DegreesToRadians);
		eBox->SetWindowText(str);
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditYrotate()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_YROTATE);
	CString str;
	eBox->GetWindowText(str);
	float angle; int flag = 0;
	if (angle = _tstof(str)) {
		import_rotation.y = angle * DegreesToRadians;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_rotation.y = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_rotation.y / DegreesToRadians);
		eBox->SetWindowText(str);
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditZrotate()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZROTATE);
	CString str;
	eBox->GetWindowText(str);
	float angle; int flag = 0;
	if (angle = _tstof(str)) {
		import_rotation.z = angle*DegreesToRadians;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_rotation.z = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_rotation.z / DegreesToRadians);
		eBox->SetWindowText(str);
	}
}

// Select skybox texture
void CRXModelViewerDlg::OnBnClickedButtonSkybox()
{
	TCHAR  szFilter[] = _T("JPEG File (*.jpg)|*.jpg|PNG File (*.png)|*.png|BMP File (*.bmp)|*.bmp|"); // More format suppoted by SOIL
	CFileDialog fileOpenDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDlg.DoModal() == IDOK)
	{
		CString imFilePath;
		VERIFY(imFilePath = fileOpenDlg.GetPathName());

		if (skyboxMapFile == "" && firstModel) {
			addTextureInfo();
		}

		skyboxMapFile = CT2A(imFilePath.GetBuffer());

		// Get image resolution
		int width, height;
		unsigned char* image;
		image = SOIL_load_image(skyboxMapFile.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		SOIL_free_image_data(image);

		if (skyboxSet) {
			skyboxMap = loadTexture(skyboxMapFile.c_str()); // Replace current skybox texture
			skybox->setDiffuseTexture(skyboxMap);
			addTextureInfo();
		}

		CListBox *outputBox;
		outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
		outputBox->AddString(_T(">-------------------------------------"));
		CString text("Skybox Texture Added.");
		outputBox->AddString(text);
		text = skyboxMapFile.c_str();
		outputBox->AddString(text);
		text.Format(_T("Resolution: %d × %d"), width, height);
		outputBox->AddString(text);

		this->Invalidate();
	}
}

// Change selected model
void CRXModelViewerDlg::OnCbnSelchangeComboMindex2()
{
	CComboBox* cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_MINDEX2);
	selectedModelIndex = cmb_function->GetCurSel() - 1;

	// Highlight all / De-highlight previous
	if (selectedModelIndex < 0) {
		GetDlgItem(IDC_BUTTON_EXPORT)->EnableWindow(false);
		if (moveMode == 0) {
			for (int i = 0; i < model.size(); i++) {
				model[i].highlightModel(false, highlight_color);
			}
			all_highlighted = true;
		}
		else if (prev_selected >= 0) {
			model[prev_selected].highlightModel(true, highlight_color);
		}
	}
	else {
		GetDlgItem(IDC_BUTTON_EXPORT)->EnableWindow(true);
		if (all_highlighted) {
			for (int i = 0; i < model.size(); i++) {
				model[i].highlightModel(true, highlight_color);
			}
			all_highlighted = false;
		}
	}

	// Update mesh selection combo
	if (selectedModelIndex >= 0 && prev_selected != selectedModelIndex) {
		cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_MESHINDEX);
		cmb_function->ResetContent(); // Clear content
		CString item;
		cmb_function->AddString(_T("All Mesh"));
		for (int i = 0; i < model[selectedModelIndex].mesh.size(); i++) {
			item.Format(_T("Mesh %d"), i);
			cmb_function->AddString(item);
		}
		cmb_function->EnableWindow(true);
		selectedMeshIndex = -1;
		if (prev_selected >= 0)
			model[prev_selected].highlightModel(true, highlight_color);
	}
	else if (selectedModelIndex < 0) {
		GetDlgItem(IDC_COMBO_MESHINDEX)->EnableWindow(false);
	}

	// Highlight whole model
	if (moveMode == 0 && selectedModelIndex >= 0 && selectedMeshIndex < 0) {
		model[selectedModelIndex].highlightModel(false, highlight_color);
	}

	if (selectedModelIndex != prev_selected) {
		GetDlgItem(IDC_EDIT_MR)->EnableWindow(false);		GetDlgItem(IDC_EDIT_MG)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_MB)->EnableWindow(false);		GetDlgItem(IDC_SLIDER_SHININESS)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_XSPEED)->EnableWindow(false);	GetDlgItem(IDC_EDIT_YSPEED)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_ZSPEED)->EnableWindow(false);	GetDlgItem(IDC_EDIT_XPOS)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_YPOS)->EnableWindow(false);		GetDlgItem(IDC_EDIT_ZPOS)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_XRO)->EnableWindow(false);		GetDlgItem(IDC_EDIT_YRO)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_ZRO)->EnableWindow(false);		GetDlgItem(IDC_EDIT_XSCA)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_YSCA)->EnableWindow(false);		GetDlgItem(IDC_EDIT_ZSCA)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_XRSPEED)->EnableWindow(false);	GetDlgItem(IDC_EDIT_YRSPEED)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_ZRSPEED)->EnableWindow(false);	GetDlgItem(IDC_EDIT_UVSU)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_UVSV)->EnableWindow(false);		GetDlgItem(IDC_EDIT_UVSHU)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_UVSHV)->EnableWindow(false);	GetDlgItem(IDC_BUTTON_ADDANIM)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(false);GetDlgItem(IDC_EDIT_UVR)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_TEXTURE)->EnableWindow(false);GetDlgItem(IDC_CHECK_MODELREFLECT)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_SPECULAR)->EnableWindow(false);
	}

	prev_selected = selectedModelIndex;

	if (!firstModel)
		glutPostRedisplay();
}

// Change selected mesh
void CRXModelViewerDlg::OnCbnSelchangeComboMeshindex()
{
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_MESHINDEX));
	selectedMeshIndex = cmb_function->GetCurSel() - 1;

	// Highlight mesh
	if (selectedModelIndex >= 0) {
		if (selectedMeshIndex < 0) {
			model[selectedModelIndex].highlightModel(false, highlight_color);
		}
		else {
			model[selectedModelIndex].highlightModel(true, highlight_color);
			model[selectedModelIndex].mesh[selectedMeshIndex].highlight(false, highlight_color);
		}
	}

	if (selectedModelIndex >= 0 && selectedMeshIndex >= 0) {
		// Enbale components
		GetDlgItem(IDC_EDIT_MR)->EnableWindow(true);		GetDlgItem(IDC_EDIT_MG)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_MB)->EnableWindow(true);		GetDlgItem(IDC_SLIDER_SHININESS)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_XSPEED)->EnableWindow(true);	GetDlgItem(IDC_EDIT_YSPEED)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_ZSPEED)->EnableWindow(true);	GetDlgItem(IDC_EDIT_XPOS)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_YPOS)->EnableWindow(true);		GetDlgItem(IDC_EDIT_ZPOS)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_XRO)->EnableWindow(true);		GetDlgItem(IDC_EDIT_YRO)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_ZRO)->EnableWindow(true);		GetDlgItem(IDC_EDIT_XSCA)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_YSCA)->EnableWindow(true);		GetDlgItem(IDC_EDIT_ZSCA)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_XRSPEED)->EnableWindow(true);	GetDlgItem(IDC_EDIT_YRSPEED)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_ZRSPEED)->EnableWindow(true);	GetDlgItem(IDC_EDIT_UVSU)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_UVSV)->EnableWindow(true);		GetDlgItem(IDC_EDIT_UVSHU)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_UVSHV)->EnableWindow(true);		GetDlgItem(IDC_BUTTON_ADDANIM)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(true);	GetDlgItem(IDC_EDIT_UVR)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_DELETEM)->EnableWindow(true); GetDlgItem(IDC_CHECK_MODELREFLECT)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_EXPORT)->EnableWindow(true);	GetDlgItem(IDC_BUTTON_EXPORTALL)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_TEXTURE)->EnableWindow(true); GetDlgItem(IDC_BUTTON_SPECULAR)->EnableWindow(true);

		CString str;
		// Animation info
		CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XSPEED);
		str.Format(_T("%.3f"), model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.x);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_YSPEED);
		str.Format(_T("%.3f"), model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.y);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZSPEED);
		str.Format(_T("%.3f"), model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.z);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_XRSPEED);
		str.Format(_T("%.3f"), model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.x);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_YRSPEED);
		str.Format(_T("%.3f"), model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.y);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZRSPEED);
		str.Format(_T("%.3f"), model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.z);
		eBox->SetWindowText(str);

		eBox = (CEdit*)GetDlgItem(IDC_EDIT_MR);
		str.Format(_T("%d"), (int)model[selectedModelIndex].mesh[selectedMeshIndex].material_color[0]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_MG);
		str.Format(_T("%d"), (int)model[selectedModelIndex].mesh[selectedMeshIndex].material_color[1]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_MB);
		str.Format(_T("%d"), (int)model[selectedModelIndex].mesh[selectedMeshIndex].material_color[2]);
		eBox->SetWindowText(str);
		CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SHININESS);
		slider->SetPos(model[selectedModelIndex].mesh[selectedMeshIndex].material_shininess);

		CButton* button = (CButton*)GetDlgItem(IDC_CHECK_MODELREFLECT);
		button->SetCheck(model[selectedModelIndex].mesh[selectedMeshIndex].is_reflective);

		// Transform info
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_XPOS);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[0]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_YPOS);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[1]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZPOS);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[2]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_XSCA);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[0]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_YSCA);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[1]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZSCA);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[2]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_XRO);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[0] / DegreesToRadians);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_YRO);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[1] / DegreesToRadians);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZRO);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[2] / DegreesToRadians);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSU);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[0]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSV);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[1]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSHU);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[0]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSHV);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[1]);
		eBox->SetWindowText(str);
		eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVR);
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_rotation / DegreesToRadians);
		eBox->SetWindowText(str);
	}
	else {
		// Disbale components
		GetDlgItem(IDC_EDIT_MR)->EnableWindow(false);		GetDlgItem(IDC_EDIT_MG)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_MB)->EnableWindow(false);		GetDlgItem(IDC_SLIDER_SHININESS)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_XSPEED)->EnableWindow(false);	GetDlgItem(IDC_EDIT_YSPEED)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_ZSPEED)->EnableWindow(false);	GetDlgItem(IDC_EDIT_XPOS)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_YPOS)->EnableWindow(false);		GetDlgItem(IDC_EDIT_ZPOS)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_XRO)->EnableWindow(false);		GetDlgItem(IDC_EDIT_YRO)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_ZRO)->EnableWindow(false);		GetDlgItem(IDC_EDIT_XSCA)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_YSCA)->EnableWindow(false);		GetDlgItem(IDC_EDIT_ZSCA)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_XRSPEED)->EnableWindow(false);	GetDlgItem(IDC_EDIT_YRSPEED)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_ZRSPEED)->EnableWindow(false);	GetDlgItem(IDC_EDIT_UVSU)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_UVSV)->EnableWindow(false);		GetDlgItem(IDC_EDIT_UVSHU)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_UVSHV)->EnableWindow(false);	GetDlgItem(IDC_BUTTON_ADDANIM)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_REFRESH)->EnableWindow(false);GetDlgItem(IDC_EDIT_UVR)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_TEXTURE)->EnableWindow(false);GetDlgItem(IDC_CHECK_MODELREFLECT)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_SPECULAR)->EnableWindow(false);
	}

	glutPostRedisplay();
}

// Bind animation
void CRXModelViewerDlg::OnBnClickedButtonAddanim()
{
	if (selectedMeshIndex >= 0)
	{
		float value;
		CString text;

		CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XSPEED);
		eBox->GetWindowText(text);
		if (text == _T("0")) {
			model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.x = 0;
		}
		else if (value = _tstof(text)) {
			model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.x = value;
		}

		eBox = (CEdit*)GetDlgItem(IDC_EDIT_YSPEED);
		eBox->GetWindowText(text);
		if (text == _T("0")) {
			model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.y = 0;
		}
		else if (value = _tstof(text)) {
			model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.y = value;
		}

		eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZSPEED);
		eBox->GetWindowText(text);
		if (text == _T("0")) {
			model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.z = 0;
		}
		else if (value = _tstof(text)) {
			model[selectedModelIndex].mesh[selectedMeshIndex].translate_velocity.z = value;
		}

		eBox = (CEdit*)GetDlgItem(IDC_EDIT_XRSPEED);
		eBox->GetWindowText(text);
		if (text == _T("0")) {
			model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.x = 0;
		}
		else if (value = _tstof(text)) {
			model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.x = value;
		}

		eBox = (CEdit*)GetDlgItem(IDC_EDIT_YRSPEED);
		eBox->GetWindowText(text);
		if (text == _T("0")) {
			model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.y = 0;
		}
		else if (value = _tstof(text)) {
			model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.y = value;
		}

		eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZRSPEED);
		eBox->GetWindowText(text);
		if (text == _T("0")) {
			model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.z = 0;
		}
		else if (value = _tstof(text)) {
			model[selectedModelIndex].mesh[selectedMeshIndex].rotate_velocity.z = value;
		}

		model[selectedModelIndex].mesh[selectedMeshIndex].is_animated = true;

		CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
		text.Format(_T("> Model Animation Updated: Model %d - Mesh %d"), selectedModelIndex, selectedMeshIndex);
		outputBox->AddString(text);
	}
}

// Pause all animation
void CRXModelViewerDlg::OnBnClickedButtonPauseanim()
{
	CButton *pBtn = (CButton*)GetDlgItem(IDC_BUTTON_PAUSEANIM);
	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);

	if (playAnimation) {
		playAnimation = false;
		pBtn->SetWindowText(_T("Continue"));
		outputBox->AddString(_T("> Animation Paused."));
	}
	else {
		playAnimation = true;
		pBtn->SetWindowText(_T("Pause All"));
		outputBox->AddString(_T("> Animation Continued."));
	}
}

// Enable/Disable CUDA
void CRXModelViewerDlg::OnBnClickedCheckCuda()
{
	enableCUDA = !enableCUDA;

	if (enableCUDA) {
		GetDlgItem(IDC_SLIDER_THREAD)->EnableWindow(false);
		CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
		outputBox->AddString(_T("> CUDA Acceleration Enabled."));
	}
	else {
		GetDlgItem(IDC_SLIDER_THREAD)->EnableWindow(true);
		CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
		outputBox->AddString(_T("> CUDA Acceleration Disabled."));
	}
}

// Enable/Disable anti-aliasing
void CRXModelViewerDlg::OnBnClickedCheckAa()
{
	enableAA = !enableAA;

	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_AA);
	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);

	if (enableAA) {
		/* temporary */
		slider->SetPos(2);
		CString text("");
		text.Format(_T("%d×"), 4);
		GetDlgItem(IDC_STATIC_AA)->SetWindowText(text);
		outputBox->AddString(_T("> MSAA Enabled."));
	}
	else {
		/* temporary */
		slider->SetPos(1);
		CString text("");
		text.Format(_T("%d×"), 0);
		GetDlgItem(IDC_STATIC_AA)->SetWindowText(text);
		outputBox->AddString(_T("> MSAA Disabled."));
	}

	if (!firstModel)
		glutPostRedisplay();
}

// Display animation control
void CRXModelViewerDlg::OnBnClickedRadioAnim()
{
	GetDlgItem(IDC_EDIT_XSPEED)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_YSPEED)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_ZSPEED)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_XLIMIT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_YLIMIT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_ZLIMIT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CHECK_AREPEAT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_XRSPEED)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_YRSPEED)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_ZRSPEED)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_ADDANIM)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_PAUSEANIM)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_TS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_TSX)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_TSY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_TSZ)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_TDL)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_TDLX)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_TDLY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_TDLZ)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_RS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_RSX)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_RSY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_RSZ)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_EDIT_XPOS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_YPOS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ZPOS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_XRO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_YRO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ZRO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_XSCA)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_YSCA)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ZSCA)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_POS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_POSX)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_POSY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_POSZ)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ROT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ROTX)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ROTY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ROTZ)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_S)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_SX)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_SY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_SZ)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_UVS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_UVSU)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_UVSV)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_UVSU)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_UVSV)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_UVSH)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_UVSHU)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_UVSHV)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_UVSHU)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_UVSHV)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_UVR)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_UVR)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_REFRESH)->ShowWindow(SW_HIDE);
}

// Display transfrom control
void CRXModelViewerDlg::OnBnClickedRadioTransform()
{
	GetDlgItem(IDC_EDIT_XSPEED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_YSPEED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ZSPEED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_XLIMIT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_YLIMIT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ZLIMIT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CHECK_AREPEAT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_XRSPEED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_YRSPEED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ZRSPEED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_ADDANIM)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_PAUSEANIM)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_TS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_TSX)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_TSY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_TSZ)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_TDL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_TDLX)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_TDLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_TDLZ)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_RS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_RSX)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_RSY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_RSZ)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_EDIT_XPOS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_YPOS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_ZPOS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_XRO)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_YRO)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_ZRO)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_XSCA)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_YSCA)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_ZSCA)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_POS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_POSX)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_POSY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_POSZ)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_ROT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_ROTX)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_ROTY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_ROTZ)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_S)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_SX)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_SY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_SZ)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_UVS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_UVSU)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_UVSV)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_UVSU)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_UVSV)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_UVSH)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_UVSHU)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_UVSHV)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_UVSHU)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_UVSHV)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_UVR)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_UVR)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_REFRESH)->ShowWindow(SW_SHOW);
}

// Modify model transform properties
void CRXModelViewerDlg::OnEnKillfocusEditXpos()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XPOS);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_position[0]) {
			translate(selectedModelIndex, selectedMeshIndex,
				vec3(offset - model[selectedModelIndex].mesh[selectedMeshIndex].model_position[0], 0, 0));
		}
		flag = 1;
	}
	else if (str == _T("0")) {
		offset = 0;
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_position[0]) {
			translate(selectedModelIndex, selectedMeshIndex,
				vec3(-model[selectedModelIndex].mesh[selectedMeshIndex].model_position[0], 0, 0));
		}
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[0]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditYpos()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_YPOS);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_position[1]) {
			translate(selectedModelIndex, selectedMeshIndex,
				vec3(0, offset - model[selectedModelIndex].mesh[selectedMeshIndex].model_position[1], 0));
		}
		flag = 1;
	}
	else if (str == _T("0")) {
		offset = 0;
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_position[1]) {
			translate(selectedModelIndex, selectedMeshIndex,
				vec3(0, -model[selectedModelIndex].mesh[selectedMeshIndex].model_position[1], 0));
		}
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[1]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditZpos()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZPOS);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_position[2]) {
			translate(selectedModelIndex, selectedMeshIndex,
				vec3(0, 0, offset - model[selectedModelIndex].mesh[selectedMeshIndex].model_position[2]));
		}
		flag = 1;
	}
	else if (str == _T("0")) {
		offset = 0;
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_position[2]) {
			translate(selectedModelIndex, selectedMeshIndex,
				vec3(0, 0, -model[selectedModelIndex].mesh[selectedMeshIndex].model_position[2]));
		}
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[2]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditXro()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XRO);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset*DegreesToRadians != model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[0]) {
			rotate(selectedModelIndex, selectedMeshIndex, vec3(1, 0, 0),
				offset*DegreesToRadians - model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[0]);
		}
		flag = 1;
	}
	else if (str == _T("0")) {
		offset = 0;
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[0]) {
			rotate(selectedModelIndex, selectedMeshIndex, vec3(1, 0, 0),
				-model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[0]);
		}
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[0] / DegreesToRadians);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditYro()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_YRO);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset*DegreesToRadians != model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[1]) {
			rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 1, 0),
				offset*DegreesToRadians - model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[1]);
		}
		flag = 1;
	}
	else if (str == _T("0")) {
		offset = 0;
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[1]) {
			rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 1, 0),
				-model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[1]);
		}
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[1] / DegreesToRadians);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditZro()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZRO);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset*DegreesToRadians != model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[2]) {
			rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 0, 1),
				offset*DegreesToRadians - model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[2]);
		}
		flag = 1;
	}
	else if (str == _T("0")) {
		offset = 0;
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[2]) {
			rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 0, 1),
				-model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[2]);
		}
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[2] / DegreesToRadians);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditXsca()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XSCA);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset > 0) {
			if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[0]) {
				rescale(selectedModelIndex, selectedMeshIndex, 
					vec3(offset / model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[0], 1, 1));
				flag = 1;
			}
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[0]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditYsca()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_YSCA);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset > 0) {
			if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[1]) {
				rescale(selectedModelIndex, selectedMeshIndex,
					vec3(1, offset / model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[1], 1));
				flag = 1;
			}
		}
		if (flag == 0) {
			str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[1]);
			eBox->SetWindowText(str);
		}
		if (flag == 1) glutPostRedisplay();
	}
}

void CRXModelViewerDlg::OnEnKillfocusEditZsca()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZSCA);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset > 0) {
			if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[2]) {
				rescale(selectedModelIndex, selectedMeshIndex,
					vec3(1, 1, offset / model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[2]));
				flag = 1;
			}
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[2]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

// Change mouse function set
void CRXModelViewerDlg::OnCbnSelchangeComboMousefunc()
{
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_MOUSEFUNC));
	mouseFuncSet = cmb_function->GetCurSel();
}

// Change mouse sensitivity
void CRXModelViewerDlg::OnNMCustomdrawSliderMsense(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_MSENSE);

	mouseSensitivity = 0.25f + 0.25f*slider->GetPos();

	CString text("");
	text.Format(_T("%.2f"), mouseSensitivity);
	GetDlgItem(IDC_STATIC_MS)->SetWindowText(text);
	*pResult = 0;
}

// Enable/Disable shadow
void CRXModelViewerDlg::OnBnClickedCheckShadow()
{
	enableShadow = !enableShadow;

	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);

	if (enableShadow) {
		outputBox->AddString(_T("> Shadow Enabled."));
	}
	else {
		outputBox->AddString(_T("> Shadow Disabled."));
	}

	if (!firstModel)
		glutPostRedisplay();
}

// Edit model's uv scale
void CRXModelViewerDlg::OnEnKillfocusEditUvsu()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSU);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset > 0) {
			if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[0]) {
				rescaleUV(selectedModelIndex, selectedMeshIndex, 
					vec2(model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[0] / offset, 1)); // 1/(offset/uv_scale)
				flag = 1;
			}
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[0]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditUvsv()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSV);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset > 0) {
			if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[1]) {
				rescaleUV(selectedModelIndex, selectedMeshIndex,
					vec2(1, model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[1] / offset));
				flag = 1;
			}
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[1]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

// Edit model's uv shift
void CRXModelViewerDlg::OnEnKillfocusEditUvshu()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSHU);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[0]) {
			shiftUV(selectedModelIndex, selectedMeshIndex, 
				vec2(offset - model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[0], 0));
			flag = 1;
		}
	}
	else if (str == _T("0")) {
		if (model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[0] != 0) {
			shiftUV(selectedModelIndex, selectedMeshIndex,
				vec2(-model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[0], 0));
			flag = 1;
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[0]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

void CRXModelViewerDlg::OnEnKillfocusEditUvshv()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSHV);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[1]) {
			shiftUV(selectedModelIndex, selectedMeshIndex,
				vec2(0, offset - model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[1]));
			flag = 1;
		}

	}
	else if (str == _T("0")) {
		if (model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[1] != 0) {
			shiftUV(selectedModelIndex, selectedMeshIndex,
				vec2(0, -model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[1]));
			flag = 1;
		}
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[1]);
		eBox->SetWindowText(str);
	}
	if (flag == 1) glutPostRedisplay();
}

// Update transform info view
void CRXModelViewerDlg::OnBnClickedButtonRefresh()
{
	CString str;
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_XPOS);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[0]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_YPOS);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[1]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZPOS);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_position[2]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_XSCA);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[0]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_YSCA);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[1]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZSCA);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_scale[2]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_XRO);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[0] / DegreesToRadians);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_YRO);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[1] / DegreesToRadians);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_ZRO);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].model_rotation[2] / DegreesToRadians);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSU);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[0]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSV);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_scale[1]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSHU);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[0]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVSHV);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_shift[1]);
	eBox->SetWindowText(str);
	eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVR);
	str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_rotation / DegreesToRadians);
	eBox->SetWindowText(str);
}

// Change display mode
void CRXModelViewerDlg::OnCbnSelchangeComboDisplaymode()
{
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_DISPLAYMODE));
	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
	int displaymode = cmb_function->GetCurSel();

	if (displaymode == 0) {
		drawMode = DRAW_FACE;
		outputBox->AddString(_T("> Displaying In Shaded Model."));
	}
	if (displaymode == 1) {
		drawMode = DRAW_LINE;
		outputBox->AddString(_T("> Displaying In Wire Frame."));
	}
	if (!firstModel) glutPostRedisplay();
}

void CRXModelViewerDlg::OnBnClickedCheckAxes()
{
	displayAxes = !displayAxes;

	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
	if (!firstModel) glutPostRedisplay();

	if (displayAxes) {
		outputBox->AddString(_T("> Axes Displayed."));
	}
	else {
		outputBox->AddString(_T("> Axes Hidden."));
	}
}

void CRXModelViewerDlg::OnBnClickedCheckSkybox()
{
	displaySkybox = !displaySkybox;

	if (!firstModel) glutPostRedisplay();

	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
	if (displaySkybox) {
		outputBox->AddString(_T("> Skybox Displayed."));
	}
	else {
		outputBox->AddString(_T("> Skybox Hidden."));
	}
}

// Change import uv rotate value
void CRXModelViewerDlg::OnEnKillfocusEditUvrotate()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVROTATE);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		import_uvRotation = offset*DegreesToRadians;
		flag = 1;
	}
	else if (str == _T("0")) {
		import_uvRotation = 0;
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), import_uvRotation / DegreesToRadians);
		eBox->SetWindowText(str);
	}
}

// Edit model's uv rotation
void CRXModelViewerDlg::OnEnKillfocusEditUvr()
{
	CEdit* eBox = (CEdit*)GetDlgItem(IDC_EDIT_UVR);
	CString str;
	eBox->GetWindowText(str);
	float offset; int flag = 0;
	if (offset = _tstof(str)) {
		if (offset*DegreesToRadians != model[selectedModelIndex].mesh[selectedMeshIndex].uv_rotation) {
			rotateUV(selectedModelIndex, selectedMeshIndex, 
				offset*DegreesToRadians - model[selectedModelIndex].mesh[selectedMeshIndex].uv_rotation);
			glutPostRedisplay();
		}
		flag = 1;
	}
	else if (str == _T("0")) {
		offset = 0;
		if (offset != model[selectedModelIndex].mesh[selectedMeshIndex].uv_rotation) {
			rotateUV(selectedModelIndex, selectedMeshIndex,
				-model[selectedModelIndex].mesh[selectedMeshIndex].uv_rotation);
			glutPostRedisplay();
		}
		flag = 1;
	}
	if (flag == 0) {
		str.Format(_T("%.2f"), model[selectedModelIndex].mesh[selectedMeshIndex].uv_rotation / DegreesToRadians);
		eBox->SetWindowText(str);
	}
}

// Enable/Disable reflection
void CRXModelViewerDlg::OnBnClickedCheckReflect()
{
	enableReflection = !enableReflection;

	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
	if (enableReflection) {
		outputBox->AddString(_T("> Reflection Enabled."));
	}
	else {
		outputBox->AddString(_T("> Reflection Disabled."));
	}

	if (!firstModel)
		glutPostRedisplay();
}

// Choose which model to delete
void CRXModelViewerDlg::OnCbnSelchangeComboMindex()
{
	CComboBox* cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_MINDEX);
	deletion_model_index = cmb_function->GetCurSel();
}

// Delete selected model
void CRXModelViewerDlg::OnBnClickedButtonDeletem()
{
	int i = 0;
	for (vector<Model>::iterator iterator = model.begin(); iterator != model.end(); ) {
		if (i == deletion_model_index) {

			CString notifStr("Delete this model?");
			if (AfxMessageBox(notifStr, MB_OKCANCEL) == IDOK) {

				// Erase vector data
				iterator = model.erase(iterator);

				totalModelNum--;

				// Update combobox info
				CComboBox* cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_MINDEX);
				cmb_function->DeleteString(deletion_model_index);
				cmb_function->SetCurSel(totalModelNum - 1);

				cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_MINDEX2);
				cmb_function->DeleteString(deletion_model_index + 1);

				if (deletion_model_index == selectedModelIndex) {
					cmb_function->SetCurSel(0);
					prev_selected = -1;
					OnCbnSelchangeComboMindex2();
				}

				deletion_model_index = totalModelNum - 1;

				CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
				outputBox->AddString(_T("> Selected Model Deleted."));
			}
		}
		if (iterator == model.end()) break;
		i++;
		iterator++;
	}

	if (!firstModel) // Prevent uninitialized crash
		glutPostRedisplay();
}

// Change model reflection property
void CRXModelViewerDlg::OnBnClickedCheckModelreflect()
{
	model[selectedModelIndex].mesh[selectedMeshIndex].is_reflective = 
		!model[selectedModelIndex].mesh[selectedMeshIndex].is_reflective;
}

// Choose whether model would stick to ground after import
void CRXModelViewerDlg::OnBnClickedCheckStg()
{
	stick_to_ground = !stick_to_ground;
}

// Export selected model to RXM file
void CRXModelViewerDlg::OnBnClickedButtonExport()
{
	// Save file dialog
	CString imFilePath;
	TCHAR  szFilter[] = _T("RXM File (*.rxm)|*.rxm|"); // Supported formats
	CFileDialog fileSaveDlg(FALSE, _T("*.rxm"), NULL, OFN_HIDEREADONLY, szFilter);
	if (fileSaveDlg.DoModal() == IDOK)
	{
		VERIFY(imFilePath = fileSaveDlg.GetPathName());
		CString fileName = fileSaveDlg.GetFileName();

		// Export model
		rxmExporter->exportToRXM(model[selectedModelIndex], CT2A(imFilePath.GetBuffer()));

		CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
		outputBox->AddString(_T("> Model Exported As " + fileName));

		CString notifStr("Model exported successfully.");
		AfxMessageBox(notifStr);
	}
}

// Export current scene to RXMS fole
void CRXModelViewerDlg::OnBnClickedButtonExportall()
{
	if (model.size() != 0) { // Is scene empty?
		CString imFilePath;
		TCHAR  szFilter[] = _T("RXM Scene File (*.rxms)|*.rxms|"); // Supported formats
		CFileDialog fileSaveDlg(FALSE, _T("*.rxms"), NULL, OFN_HIDEREADONLY, szFilter);
		if (fileSaveDlg.DoModal() == IDOK)
		{
			VERIFY(imFilePath = fileSaveDlg.GetPathName());
			CString fileName = fileSaveDlg.GetFileName();

			// Export scene
			rxmExporter->exportToRXMS(model, CT2A(imFilePath.GetBuffer()));

			CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
			outputBox->AddString(_T("> Scene Exported As " + fileName));

			CString notifStr("Scene exported successfully.");
			AfxMessageBox(notifStr);
		}
	}
	else {
		CString notifStr("Current scene is empty.");
		AfxMessageBox(notifStr);
	}
}

// Enable/Disable fog effect
void CRXModelViewerDlg::OnBnClickedCheckFog()
{
	enableFog = !enableFog;

	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
	if (enableFog) {
		outputBox->AddString(_T("> Fog Enabled."));
	}
	else {
		outputBox->AddString(_T("> Fog Disabled."));
	}

	if (!firstModel)
		glutPostRedisplay();
}

#pragma endregion


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////            OPENGL PART            /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region OpenGL Functions
//--------------------------------------------------------------------------------------
// Record newly added model/texture's info

void CRXModelViewerDlg::addModelInfo()
{
	CComboBox* cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_MINDEX); // Model deletion combo
	CString str;
	str.Format(_T("Model %d"), historyModelNum);
	cmb_function->AddString(str);
	cmb_function->SetCurSel(totalModelNum);
	deletion_model_index = totalModelNum;

	cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_MINDEX2); // Model selection combo
	str.Format(_T("Model %d"), historyModelNum);
	cmb_function->AddString(str);

	totalModelNum++;
	historyModelNum++;
}

void CRXModelViewerDlg::addTextureInfo()
{
	CComboBox* cmb_function = (CComboBox*)GetDlgItem(IDC_COMBO_TINDEX); // Texture deletion combo
	CString str;
	str.Format(_T("Texture %d"), totalTextureNum);
	cmb_function->AddString(str);
	cmb_function->SetCurSel(totalTextureNum);

	//TODO...

	totalTextureNum++;
}

//--------------------------------------------------------------------------------------
// Set lighting parameters

void setEnvironmentLight(GLuint shader)
{
	// Translate color value into decimal form
	float r = light_color[0] / 255.0f, g = light_color[1] / 255.0f, b = light_color[2] / 255.0f;

	// Light properties
	point4 light_position(light_position.x, light_position.y, light_position.z, 0.0f);
	// These constants are determined by light source's characteristics, customizable in later updates
	color4 light_ambient (r / 4.0f, g / 4.0f, b / 4.0f, 1.0f);
	color4 light_diffuse (r, g, b, 1.0f);
	color4 light_specular(r, g, b, 1.0f);

	// Set uniform values
	glUniform4fv(glGetUniformLocation(shader, "LightAmbient"), 1, light_ambient);
	glUniform4fv(glGetUniformLocation(shader, "LightDiffuse"), 1, light_diffuse);
	glUniform4fv(glGetUniformLocation(shader, "LightSpecular"), 1, light_specular);
	glUniform4fv(glGetUniformLocation(shader, "LightPosition"), 1, light_position);
}

//--------------------------------------------------------------------------------------
// Update model texture

void CRXModelViewerDlg::updateModelTexture(int modelIndex, int meshIndex, const char* type, const char* path)
{
	if (strcmp(type, DIFFUSE) == 0) {
		// Already had equal or higher version shader
		if (strcmp(model[modelIndex].mesh[meshIndex].diffuse_tex, "") != 0 || strcmp(model[modelIndex].mesh[meshIndex].specular_tex, "") != 0) {
			diffuseMap = loadTexture(path);
			model[modelIndex].mesh[meshIndex].setDiffuseTexture(diffuseMap);
			model[modelIndex].mesh[meshIndex].setTextureFilePath(path, model[modelIndex].mesh[meshIndex].specular_tex);
			addTextureInfo();
		}
		// Need to update shader
		else {
			model[modelIndex].mesh[meshIndex].replaceShader(objectShader_medium);
			diffuseMap = loadTexture(path);
			model[modelIndex].mesh[meshIndex].setDiffuseTexture(diffuseMap);
			model[modelIndex].mesh[meshIndex].setTextureFilePath(path, model[modelIndex].mesh[meshIndex].specular_tex);
			addTextureInfo();
		}
	}

	else if (strcmp(type, SPECULAR) == 0) {
		if (strcmp(model[modelIndex].mesh[meshIndex].specular_tex, "") != 0) {
			specularMap = loadTexture(path);
			model[modelIndex].mesh[meshIndex].setSpecularTexture(specularMap);
			model[modelIndex].mesh[meshIndex].setTextureFilePath(model[modelIndex].mesh[meshIndex].diffuse_tex, path);
			addTextureInfo();
		}
		else {
			model[modelIndex].mesh[meshIndex].replaceShader(objectShader_high);
			specularMap = loadTexture(path);
			model[modelIndex].mesh[meshIndex].setSpecularTexture(specularMap);
			model[modelIndex].mesh[meshIndex].setTextureFilePath(model[modelIndex].mesh[meshIndex].diffuse_tex, path);
			addTextureInfo();
		}
	}
}

//--------------------------------------------------------------------------------------
// Scene initialization

void initScene()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Background color
	glClearColor(background_color[0], background_color[1], background_color[2], 1.0);

	// Load shaders
	objectShader_high   = InitShader(HIGH_VSHADER, HIGH_FSHADER);	  // Shading with diffuse map and specular map
	objectShader_medium = InitShader(MEDIUM_VSHADER, MEDIUM_FSHADER); // Shading with diffuse map only
	objectShader_basic  = InitShader(BASIC_VSHADER, BASIC_FSHADER);   // Rough shading

	glUseProgram(objectShader_high); // Must be called first
	setEnvironmentLight(objectShader_high);
	glUseProgram(objectShader_medium);
	setEnvironmentLight(objectShader_medium);
	glUseProgram(objectShader_basic);
	setEnvironmentLight(objectShader_basic);

	glShadeModel(GL_SMOOTH);

	// Camera initial position
	vec3 pos(0.0f, 0.0f, 1.0f);
	vec3 at(0.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 1.0f, 0.0f);

	// Free camera
	camera = new GLCamera(pos, at, up, objectShader_basic, objectShader_medium, objectShader_high);
}

//--------------------------------------------------------------------------------------
// Display function

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup projection matrix
	mat4 _projection = Perspective(45.0, aspect_rec, zNear, zFar*zFarRatio);

	// MSAA
	if (enableAA) glEnable(GL_MULTISAMPLE_ARB);
	else glDisable(GL_MULTISAMPLE_ARB);

	// Fog
	if (enableFog) {
		// TODO
	}
	else {
		// TODO
	}

	// Draw skybox
	if (displaySkybox) {
		glUseProgram(skyboxShader);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "ModelView"), 1, GL_TRUE, camera->getModelViewMatrix());
		skybox->glDraw(_projection, false, false, DRAW_FACE, vec3(0, 0, 0), 0);
	}
	// Draw axes
	if (displayAxes) {
		glUseProgram(axesShader);
		glUniformMatrix4fv(glGetUniformLocation(axesShader, "ModelView"), 1, GL_TRUE, camera->getModelViewMatrix());
		axes->glDraw(_projection, false, false, DRAW_LINE, vec3(0, 0, 0), 0);
	}
	// Draw models
	for (int i = 0; i < model.size(); i++) {
		model[i].glDraw(_projection, enableShadow, enableReflection, drawMode, camera->getCamPosition(), skyboxMap);
	}

	/* temporary fix for model not being displayed correctly on startup */
	if (first_display) {
		first_display = false;
		camera->resetModelViewMatrix();
		glutPostRedisplay();
	}

	glutSwapBuffers(); // For GL_DOUBLE. If GL_SINGLE, use glFlush() instead
}

//--------------------------------------------------------------------------------------
// Transform model vertices

void translate(int modelIndex, int meshIndex, vec3 translation)
{
	// Translate all models
	if (modelIndex < 0) {
		if (enableCUDA) { // CUDA
			for (int i = 0; i < model.size(); i++) {
				for (int j = 0; j < model[i].mesh.size(); j++) {
					translate_CUDA(model[i].mesh[j].vertices, vec4(translation, 0));
				}
				model[i].setupModelBuffers();
			}
		}
		else { // OpenMP
			for (int k = 0; k < model.size(); k++) {
				for (int i = 0; i < model[k].mesh.size(); i++) {
#pragma omp parallel for num_threads(threadNum) 
					for (int j = 0; j < model[k].mesh[i].vertices.size(); j++) {
						model[k].mesh[i].vertices[j].position = model[k].mesh[i].vertices[j].position + vec4(translation, 0);
					}
				}
				model[k].setupModelBuffers();
			}
		}

		// Record translation info
		for (int j = 0; j < model.size(); j++) {
			for (int i = 0; i < model[j].mesh.size(); i++) {
				model[j].mesh[i].model_position = model[j].mesh[i].model_position + translation;
			}
		}
	}

	// Tanslate selected model
	else if (meshIndex < 0) {
		if (enableCUDA) {
			for (int i = 0; i < model[modelIndex].mesh.size(); i++) {
				translate_CUDA(model[modelIndex].mesh[i].vertices, vec4(translation, 0));
			}
		}
		else {
			for (int j = 0; j < model[modelIndex].mesh.size(); j++) {
#pragma omp parallel for num_threads(threadNum)
				for (int i = 0; i < model[modelIndex].mesh[j].vertices.size(); i++) {
					model[modelIndex].mesh[j].vertices[i].position = model[modelIndex].mesh[j].vertices[i].position + vec4(translation, 0);
				}
			}
		}
		model[modelIndex].setupModelBuffers();

		// Record translation info
		for (int i = 0; i < model[modelIndex].mesh.size(); i++) {
			model[modelIndex].mesh[i].model_position = model[modelIndex].mesh[i].model_position + translation;
		}
	}

	// Translate selected mesh
	else {
		if (enableCUDA) {
			translate_CUDA(model[modelIndex].mesh[meshIndex].vertices, vec4(translation, 0));		
		}

		else {
#pragma omp parallel for num_threads(threadNum)
			for (int i = 0; i < model[modelIndex].mesh[meshIndex].vertices.size(); i++) {
				model[modelIndex].mesh[meshIndex].vertices[i].position = model[modelIndex].mesh[meshIndex].vertices[i].position + vec4(translation, 0);
			}
		}
		model[modelIndex].setupModelBuffers();

		model[modelIndex].mesh[meshIndex].model_position = model[modelIndex].mesh[meshIndex].model_position + translation;
	}

}

void rotate(int modelIndex, int meshIndex, vec3 axis, float value)
{
	/* currently can only rotate around origin xyz axis */
	char ax;
	if (axis.x == 1) ax = 'x';
	else if (axis.y == 1) ax = 'y';
	else if (axis.z == 1) ax = 'z';

	float s = sin(value);
	float c = cos(value);
	mat4 rotation;

	switch (ax) {
	case 'y': {
		rotation = { // Y rotation matrix
			c, 0, s, 0,
			0, 1, 0, 0,
		   -s, 0, c, 0,
			0, 0, 0, 1
		};
		break; }
	case 'x': {
		rotation = { // X rotation matrix
			1, 0, 0, 0,
			0, c,-s, 0,
			0, s, c, 0,
			0, 0, 0, 1
		};
		break; }
	case 'z': {
		rotation = { // Z rotation matrix
			c,-s, 0, 0,
			s, c, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		break; }
	default: break;
	}

	// Record
	vec3 deltaR;
	if (ax == 'x') deltaR = vec3(value, 0, 0);
	if (ax == 'y') deltaR = vec3(0, value, 0);
	if (ax == 'z') deltaR = vec3(0, 0, value);

	// Rotate all models
	if (modelIndex < 0) {

		if (enableCUDA) {
			for (int j = 0; j < model.size(); j++) {
				for (int i = 0; i < model[j].mesh.size(); i++) {
					rotate_CUDA(model[j].mesh[i].vertices, rotation);
				}
				model[j].setupModelBuffers();
			}
		}
		else {
			for (int k = 0; k < model.size(); k++) {
				for (int i = 0; i < model[k].mesh.size(); i++) {
#pragma omp parallel for num_threads(threadNum)
					for (int j = 0; j < model[k].mesh[i].vertices.size(); j++) {
						model[k].mesh[i].vertices[j].position = rotation * model[k].mesh[i].vertices[j].position;
					}
				}
			}

			// Don't forget to rotate normals as well
			for (int k = 0; k < model.size(); k++) {
				for (int i = 0; i < model[k].mesh.size(); i++) {
#pragma omp parallel for num_threads(threadNum)
					for (int j = 0; j < model[k].mesh[i].vertices.size(); j++) {
						model[k].mesh[i].vertices[j].normal = rotation * model[k].mesh[i].vertices[j].normal;
					}
				}
				model[k].setupModelBuffers();
			}
		}

		for (int k = 0; k < model.size(); k++) {
			for (int i = 0; i < model[k].mesh.size(); i++) {
				model[k].mesh[i].model_rotation = model[k].mesh[i].model_rotation + deltaR;
			}
		}
	}
	
	// Rotate selectd model
	else if (meshIndex < 0) {
		if (enableCUDA) {
			for (int i = 0; i < model[modelIndex].mesh.size(); i++) {
				rotate_CUDA(model[modelIndex].mesh[i].vertices, rotation);
			}

		}
		else {
			for (int j = 0; j < model[modelIndex].mesh.size(); j++) {
#pragma omp parallel for num_threads(threadNum)
				for (int i = 0; i < model[modelIndex].mesh[j].vertices.size(); i++) {
					model[modelIndex].mesh[j].vertices[i].position = rotation * model[modelIndex].mesh[j].vertices[i].position;
				}
			}

			for (int j = 0; j < model[modelIndex].mesh.size(); j++) {
#pragma omp parallel for num_threads(threadNum)
				for (int i = 0; i < model[modelIndex].mesh[j].vertices.size(); i++) {
					model[modelIndex].mesh[j].vertices[i].normal = rotation * model[modelIndex].mesh[j].vertices[i].normal;
				}
			}
		}
		model[modelIndex].setupModelBuffers();

		for (int j = 0; j < model[modelIndex].mesh.size(); j++) {
			model[modelIndex].mesh[j].model_rotation = model[modelIndex].mesh[j].model_rotation + deltaR;
		}
	}

	// Rotate selected mesh
	else {
		if (enableCUDA) {
			rotate_CUDA(model[modelIndex].mesh[meshIndex].vertices, rotation);
		}

		else {
#pragma omp parallel for num_threads(threadNum)
			for (int i = 0; i < model[modelIndex].mesh[meshIndex].vertices.size(); i++) {
				model[modelIndex].mesh[meshIndex].vertices[i].position = rotation * model[modelIndex].mesh[meshIndex].vertices[i].position;
			}
#pragma omp parallel for num_threads(threadNum)
			for (int i = 0; i < model[modelIndex].mesh[meshIndex].vertices.size(); i++) {
				model[modelIndex].mesh[meshIndex].vertices[i].normal = rotation * model[modelIndex].mesh[meshIndex].vertices[i].normal;
			}
		}
		model[modelIndex].setupModelBuffers();

		model[modelIndex].mesh[meshIndex].model_rotation = model[modelIndex].mesh[meshIndex].model_rotation + deltaR;
	}
}

void rescale(int modelIndex, int meshIndex, vec3 scale)
{
	// Rescale all models
	if (modelIndex < 0) {

		if (enableCUDA) {
			for (int j = 0; j < model.size(); j++) {
				for (int i = 0; i < model[j].mesh.size(); i++) {
					rescale_CUDA(model[j].mesh[i].vertices, vec4(scale, 1));
				}
				model[j].setupModelBuffers();
			}
		}
		else {
			for (int k = 0; k < model.size(); k++) {
				for (int i = 0; i < model[k].mesh.size(); i++) {
#pragma omp parallel for num_threads(threadNum)
					for (int j = 0; j < model[k].mesh[i].vertices.size(); j++) {
						model[k].mesh[i].vertices[j].position = model[k].mesh[i].vertices[j].position * vec4(scale, 1);
					}
				}
				model[k].setupModelBuffers();
			}
		}

		for (int k = 0; k < model.size(); k++) {
			for (int i = 0; i < model[k].mesh.size(); i++) {
				model[k].mesh[i].model_scale = model[k].mesh[i].model_scale + scale;
			}
		}
	}

	// Rescale selected model
	else if (meshIndex < 0) {
		if (enableCUDA) {
			for (int i = 0; i < model[modelIndex].mesh.size(); i++) {
				rescale_CUDA(model[modelIndex].mesh[i].vertices, vec4(scale, 1));
			}
		}
		else {
			for (int j = 0; j < model[modelIndex].mesh.size(); j++) {
#pragma omp parallel for num_threads(threadNum)
				for (int i = 0; i < model[modelIndex].mesh[j].vertices.size(); i++) {
					model[modelIndex].mesh[j].vertices[i].position = model[modelIndex].mesh[j].vertices[i].position * vec4(scale, 1);
				}
			}
			model[modelIndex].setupModelBuffers();

			for (int j = 0; j < model[modelIndex].mesh.size(); j++) {
				model[modelIndex].mesh[j].model_scale = model[modelIndex].mesh[j].model_scale * scale;
			}
		}
	}

	// Rescale selected mesh
	else {
		if (enableCUDA) {
			rescale_CUDA(model[modelIndex].mesh[meshIndex].vertices, vec4(scale, 1));
		}

		else {
#pragma omp parallel for num_threads(threadNum)
			for (int i = 0; i < model[modelIndex].mesh[meshIndex].vertices.size(); i++) {
				model[modelIndex].mesh[meshIndex].vertices[i].position = model[modelIndex].mesh[meshIndex].vertices[i].position * vec4(scale, 1);
			}
		}
		model[modelIndex].setupModelBuffers();

		model[modelIndex].mesh[meshIndex].model_scale = model[modelIndex].mesh[meshIndex].model_scale * scale;
	}
}

//--------------------------------------------------------------------------------------
// Transform UV coordinates

void rescaleUV(int modelIndex, int meshIndex, vec2 scale)
{
	if (enableCUDA) {
		rescaleUV_CUDA(model[modelIndex].mesh[meshIndex].vertices, vec4(scale, 1, 1));
	}
	else {
#pragma omp parallel for num_threads(threadNum)
		for (int i = 0; i < model[modelIndex].mesh[meshIndex].vertices.size(); i++) {
			model[modelIndex].mesh[meshIndex].vertices[i].tex_coords = model[modelIndex].mesh[meshIndex].vertices[i].tex_coords * vec4(scale, 1, 1);
		}
	}
	model[modelIndex].setupModelBuffers();

	model[modelIndex].mesh[meshIndex].uv_scale = model[modelIndex].mesh[meshIndex].uv_scale * vec2(1.0f / scale.x, 1.0f / scale.y); // Record = 1 / Actual
}

void shiftUV(int modelIndex, int meshIndex, vec2 shift)
{
	if (enableCUDA) {
		shiftUV_CUDA(model[modelIndex].mesh[meshIndex].vertices, vec4(shift, 0, 0));
	}
	else {
#pragma omp parallel for num_threads(threadNum)
		for (int i = 0; i < model[modelIndex].mesh[meshIndex].vertices.size(); i++) {
			model[modelIndex].mesh[meshIndex].vertices[i].tex_coords = model[modelIndex].mesh[meshIndex].vertices[i].tex_coords + vec4(shift, 0, 0);
		}
	}
	model[modelIndex].setupModelBuffers();

	model[modelIndex].mesh[meshIndex].uv_shift = model[modelIndex].mesh[meshIndex].uv_shift + shift;
}

void rotateUV(int modelIndex, int meshIndex, float value)
{
	if (enableCUDA) {
		rotateUV_CUDA(model[modelIndex].mesh[meshIndex].vertices, value);
	}
	else {
		float cuv = cos(value), suv = sin(value);
		mat4 rotate{
			cuv, suv, 0, 0,
			-suv, cuv, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

#pragma omp parallel for num_threads(threadNum)
		for (int i = 0; i < model[modelIndex].mesh[meshIndex].vertices.size(); i++) {
			model[modelIndex].mesh[meshIndex].vertices[i].tex_coords = rotate * model[modelIndex].mesh[meshIndex].vertices[i].tex_coords;
		}
	}
	model[modelIndex].setupModelBuffers();

	model[modelIndex].mesh[meshIndex].uv_rotation = model[modelIndex].mesh[meshIndex].uv_rotation + value;
}

//--------------------------------------------------------------------------------------
// Keyboard function

void keyboard(unsigned char key, int x, int y)
{
	// Close program
	if (key == 033) { // Escape key 
		CString notifStr("Quit RX Model Viewer?");
		if (AfxMessageBox(notifStr, MB_OKCANCEL) == IDOK) {
			// Release memory
			model.swap(vector<Model>());
			delete camera;
			delete objLoader;
			delete stlLoader;
			delete rxmImporter;
			delete rxmExporter;
			delete skybox;
			delete skybox_bak;
			delete axes;
			exit(EXIT_SUCCESS);
		}
	}

	// Move model
	if (moveMode == 0) {

		switch (key) {

		// Rotate model
		case 'x': rotate(selectedModelIndex, selectedMeshIndex, vec3(1, 0, 0), 3 * DegreesToRadians); break;
		case 'X': rotate(selectedModelIndex, selectedMeshIndex, vec3(1, 0, 0), -3 * DegreesToRadians); break;

		case 'y': rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 1, 0), 3 * DegreesToRadians); break;
		case 'Y': rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 1, 0), -3 * DegreesToRadians); break;

		case 'z': rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 0, 1), 3 * DegreesToRadians); break;
		case 'Z': rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 0, 1), -3 * DegreesToRadians); break;

		// Rescale model
		case 'm': rescale(selectedModelIndex, selectedMeshIndex, vec3(0.8f, 0.8f, 0.8f)); break;
		case 'M': rescale(selectedModelIndex, selectedMeshIndex, vec3(1.25f, 1.25f, 1.25f)); break;

		// Move along axes
		case 'w': case 'W': translate(selectedModelIndex, selectedMeshIndex, vec3(0, 0, -0.3f)); break;
		case 's': case 'S': translate(selectedModelIndex, selectedMeshIndex, vec3(0, 0, 0.3f)); break;
		case 'a': case 'A': translate(selectedModelIndex, selectedMeshIndex, vec3(0.3f, 0, 0)); break;
		case 'd': case 'D': translate(selectedModelIndex, selectedMeshIndex, vec3(-0.3f, 0, 0)); break;
		case 'q': case 'Q': translate(selectedModelIndex, selectedMeshIndex, vec3(0, 0.3f, 0)); break;
		case 'e': case 'E': translate(selectedModelIndex, selectedMeshIndex, vec3(0, -0.3f, 0)); break;

		default: break;
		}
	}

	// Move camera
	if (moveMode == 1) {

		switch (key) {

		// Move along axes
		case 'w': case 'W': camera->slide(0, 0, -0.3); break;
		case 's': case 'S': camera->slide(0, 0,  0.3); break;
		case 'a': case 'A': camera->slide(-0.3, 0, 0); break;
		case 'd': case 'D': camera->slide( 0.3, 0, 0); break;
		case 'q': case 'Q': camera->slide( 0, 0.3, 0); break;
		case 'e': case 'E': camera->slide(0, -0.3, 0); break;

		// Rotate camera
		case 'z': camera->roll(3); break;
		case 'Z': camera->roll(-3); break;
		case 'x': camera->pitch(3); break;
		case 'X': camera->pitch(-3); break;
		case 'y': camera->yaw(3); break;
		case 'Y': camera->yaw(-3); break;

		// Zoom in/out
		case 'm': camera->slide(0, 0, -3); break;
		case 'M': camera->slide(0, 0, 3); break;

		// Reset position
		case ' ': camera->resetModelViewMatrix(); break;

		default: break;
		}

		adjustSkybox();
	}

	glutPostRedisplay(); // Mark current window to be re-displayed
}

//--------------------------------------------------------------------------------------
// Reshape function, prevent distortion when window size changed

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	aspect_rec = GLfloat(width) / height;
}

//--------------------------------------------------------------------------------------
// Idle funtion, used for simple animation

void idle()
{
	for (int i = 0; i < model.size() && playAnimation; i++)
	{
		for (int j = 0; j < model[i].mesh.size(); j++) {
			if (model[i].mesh[j].is_animated == true)
			{
				if (model[i].mesh[j].translate_velocity.x != 0)
					translate(i, j, vec3(model[i].mesh[j].translate_velocity.x * ANIM_SCALE, 0, 0));
				if (model[i].mesh[j].translate_velocity.y != 0)
					translate(i, j, vec3(0, model[i].mesh[j].translate_velocity.y * ANIM_SCALE, 0));
				if (model[i].mesh[j].translate_velocity.z != 0)
					translate(i, j, vec3(0, 0, model[i].mesh[j].translate_velocity.z * ANIM_SCALE));

				if (model[i].mesh[j].rotate_velocity.x != 0)
					rotate(i, j, vec3(1, 0, 0), model[i].mesh[j].rotate_velocity.x * ANIM_SCALE);
				if (model[i].mesh[j].rotate_velocity.y != 0)
					rotate(i, j, vec3(0, 1, 0), model[i].mesh[j].rotate_velocity.y * ANIM_SCALE);
				if (model[i].mesh[j].rotate_velocity.z != 0)
					rotate(i, j, vec3(0, 0, 1), model[i].mesh[j].rotate_velocity.z * ANIM_SCALE);

				glutPostRedisplay();
			}
		}
	}
}

//--------------------------------------------------------------------------------------
// Mouse and camera related function

// Adjust skybox postion to follow camera
void adjustSkybox()
{
	for (int i = 0; i < skybox->vertices.size(); i++)
	{
		skybox->vertices[i].position = skybox_bak->vertices[i].position + vec4(camera->getCamPosition(), 0.0);
	}
	skybox->setupBuffers();
}

// Camera rotation
void fcRotateX(float angle)
{
	float d = camera->getDist();
	int cnt = 100;
	float theta = angle / cnt;
	float slide_d = -2 * d*sin(theta*DegreesToRadians);

	camera->yaw(theta / 2);

	for (; cnt != 0; cnt--) {
		camera->slide(slide_d, 0, 0);
		camera->yaw(theta);
	}

	camera->yaw(-theta / 2);
}

void fcRotateY(float angle)
{
	float d = camera->getDist();
	int cnt = 100;
	float theta = angle / cnt;
	float slide_d = 2 * d*sin(theta*DegreesToRadians);

	camera->pitch(theta / 2);

	for (; cnt != 0; cnt--) {
		camera->slide(0, slide_d, 0);
		camera->pitch(theta);
	}

	camera->pitch(-theta / 2);
}

// Mouse functions
void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {

		// Record click position
		lastPos = vec2(x, y);

		if (button == GLUT_LEFT_BUTTON)
			mouseButton = 0;
		if (button == GLUT_RIGHT_BUTTON)
			mouseButton = 1;
		if (button == GLUT_MIDDLE_BUTTON)
			mouseButton = 2;
	}
}

void mouseMove(int x, int y)
{
	int dx = x - lastPos.x;
	int dy = y - lastPos.y;

	// Move camera
	if (moveMode == 1) {
		// Left
		if (mouseButton == 0) {
			fcRotateX(dx * 0.3f * mouseSensitivity); // Rotate view
			fcRotateY(dy * 0.3f * mouseSensitivity);

			if (skyboxSet) adjustSkybox();
		}
		// Right
		if (mouseButton == 1) {
			if (mouseFuncSet == 0) {
				camera->roll(dx * 0.3f * mouseSensitivity); // Rotate on xy plane
				camera->pitch(dy * 0.3f * mouseSensitivity);
			}
			if (mouseFuncSet == 1)
				camera->slide(0, 0, -dy * 0.3f * mouseSensitivity);

			if (skyboxSet) adjustSkybox();
		}
		// Middle
		if (mouseButton == 2) {
			camera->slide(-dx * 0.1f * mouseSensitivity, dy * 0.1f * mouseSensitivity, 0); // Slide on xy plane

			if (skyboxSet) adjustSkybox();
		}
	}

	// Move model
	if (moveMode == 0) {
		// Left
		if (mouseButton == 0) {
			translate(selectedModelIndex, selectedMeshIndex, vec3((dx / 5.0f)*mouseSensitivity, 0, 0));
			translate(selectedModelIndex, selectedMeshIndex, vec3(0, 0, (dy / 5.0f)*mouseSensitivity));
		}
		// Right
		if (mouseButton == 1) {
			translate(selectedModelIndex, selectedMeshIndex, vec3(0, (-dy / 5.0f)*mouseSensitivity, 0));
		}
		// Middle
		if (mouseButton == 2) {
			if (mouseFuncSet == 0) {
				float sca = 1 + (dx / 30.0f)*mouseSensitivity;
				rescale(selectedModelIndex, selectedMeshIndex, vec3(sca, sca, sca));
			}
			if (mouseFuncSet == 1) {
				rotate(selectedModelIndex, selectedMeshIndex, vec3(0, 0, 1), (dx / 10.0f)*mouseSensitivity);
				rotate(selectedModelIndex, selectedMeshIndex, vec3(1, 0, 0), (dy / 10.0f)*mouseSensitivity);
			}
		}
	}

	lastPos = vec2(x, y);

	glutPostRedisplay();
}

void mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0) { // Zoom in
		camera->slide(0, 0, -1 * mouseSensitivity);
		if (skyboxSet) adjustSkybox();
	}

	else { // Zoom out
		camera->slide(0, 0, 1 * mouseSensitivity);
		if (skyboxSet) adjustSkybox();
	}

	glutPostRedisplay();
}

//--------------------------------------------------------------------------------------
// Setup skybox and axes model

void CRXModelViewerDlg::setupSkybox()
{
	skyboxShader = InitShader(SKYBOX_VSHADER, SKYBOX_FSHADER); // Skybox shader

	skybox = new Mesh(skyboxShader);
	skybox_bak = new Mesh(skyboxShader);

	// Skybox import properties
	objLoader->threadNum = threadNum;
	objLoader->scale = vec4(2 * zFar, 2 * zFar, 2 * zFar, 1);
	objLoader->position = vec4(0, -zFar, 0, 0);
	objLoader->rotation = vec3(0, 0, 0);
	objLoader->uvScale = vec4(1, 1, 1, 1);
	objLoader->uvShift = vec4(0, 0, 0, 0);
	objLoader->uvRotation = 0;
	objLoader->stickToGround = false;
	objLoader->useCUDA = false;

	// Load skybox model
	objLoader->readFile(SKYBOX_MODEL, *skybox, skyboxShader, false);

	skybox->setupBuffers();

	/* Use cubemap */
	//vector<const char*> faces;
	//for (int i = 0; i < 6; i++) {
	//	faces.push_back(skyboxMapFile.c_str());
	//}

	if (skyboxMapFile != "") {
		skyboxMap = loadTexture(skyboxMapFile.c_str()); // Load skybox texture
	}
	skybox->setDiffuseTexture(skyboxMap);

	skyboxSet = true;

	// Backup skybox vertices for adjustment
	for (int i = 0; i < skybox->vertices.size(); i++) {
		skybox_bak->vertices.push_back(skybox->vertices[i]);
	}
}

void setupAxes()
{
	axesShader = InitShader(AXIS_VSHADER, AXIS_FSHADER);

	axes = new Mesh(axesShader);

	objLoader->threadNum = threadNum;
	objLoader->scale = vec4(1, 1, 1, 1);
	objLoader->position = vec4(0, 0, 0, 0);
	objLoader->rotation = vec3(0, 0, 0);
	objLoader->uvScale = vec4(1, 1, 1, 1);
	objLoader->uvShift = vec4(0, 0, 0, 0);
	objLoader->uvRotation = 0;
	objLoader->stickToGround = false;
	objLoader->useCUDA = false;

	objLoader->readFile(AXIS_MODEL, *axes, axesShader, false);

	axes->setupBuffers();
}

//--------------------------------------------------------------------------------------
// Setup OpenGL functions

void CRXModelViewerDlg::setupGLfunctions()
{
	setupAxes();   // Initialize axes
	setupSkybox(); // Initialize skybox

	glutDisplayFunc(display);		// Display
	glutMouseFunc(mouse);			// Mouse
	glutMotionFunc(mouseMove);		// Mouse motion
	glutMouseWheelFunc(mouseWheel); // Mouse wheel
	glutKeyboardFunc(keyboard);		// Keyboard
	glutReshapeFunc(reshape);		// Reshape
	glutIdleFunc(idle);				// Idle

	glutMainLoop();
}

//--------------------------------------------------------------------------------------
// Initialize OpenGL environment

void initGLEnvironment(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GL_MULTISAMPLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Test freeglut environment
	glutInitContextVersion(4, 5);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitWindowPosition(100, 50);
	glutCreateWindow(">"); // Coding error on my device, reason unknown. So no name is set.

	// Must be initialized after window is created
	glewExperimental = GL_TRUE;
	glewInit(); // Initialize GLEW
}

//--------------------------------------------------------------------------------------
// Revise new model's texture info

void CRXModelViewerDlg::reviseNewModel()
{
	int m_index = model.size() - 1;
	for (int i = 0; i < model[m_index].mesh.size(); i++) {
		// Get linked texture file path
		specularMapFile = model[m_index].mesh[i].specular_tex;
		diffuseMapFile = model[m_index].mesh[i].diffuse_tex;

		// Assign textures
		if (strcmp(specularMapFile.c_str(), "") != 0) {
			model[m_index].mesh[i].replaceShader(objectShader_high);
			updateModelTexture(m_index, i, SPECULAR, specularMapFile.c_str());
			if (strcmp(diffuseMapFile.c_str(), "") != 0) {
				updateModelTexture(m_index, i, DIFFUSE, diffuseMapFile.c_str());
			}
			else
				model[m_index].mesh[i].setDiffuseTexture(0);
		}
		else if (strcmp(diffuseMapFile.c_str(), "") != 0) {
			model[m_index].mesh[i].replaceShader(objectShader_medium);
			updateModelTexture(m_index, i, DIFFUSE, diffuseMapFile.c_str());
		}

		specularMapFile = ""; diffuseMapFile = "";
	}
}

//--------------------------------------------------------------------------------------
// Finish scene importing

void CRXModelViewerDlg::completeSceneImport(int newModelNum)
{
	int start = model.size() - newModelNum; // Set in 'for' would cause access violation (?)
	int end = model.size();
	// Assign each model
	for (int i = start; i < end; i++) {

		for (int j = 0; j < model[i].mesh.size(); j++) {
			// Get linked texture file path
			specularMapFile = model[i].mesh[j].specular_tex;
			diffuseMapFile = model[i].mesh[j].diffuse_tex;

			// Assign textures
			if (strcmp(specularMapFile.c_str(), "") != 0) {
				model[i].mesh[j].replaceShader(objectShader_high);
				updateModelTexture(i, j, SPECULAR, specularMapFile.c_str());
				if (strcmp(diffuseMapFile.c_str(), "") != 0) {
					updateModelTexture(i, j, DIFFUSE, diffuseMapFile.c_str());
				}
				else
					model[i].mesh[j].setDiffuseTexture(0);
			}
			else if (strcmp(diffuseMapFile.c_str(), "") != 0) {
				model[i].mesh[j].replaceShader(objectShader_medium);
				updateModelTexture(i, j, DIFFUSE, diffuseMapFile.c_str());
			}

			specularMapFile = ""; diffuseMapFile = "";
		}
		model[i].setupModelBuffers();

		addModelInfo();
	}
}

//--------------------------------------------------------------------------------------
// Add a new model into the scene

int CRXModelViewerDlg::addNewModel(int argc, char **argv)
{
	// Initialize OpenGL Environment
	if (firstModel) {
		initGLEnvironment(argc, argv);
		initScene();
	}

	int newMeshNum = 0;  // Imported meshes' number

	bool isScene = false;// Indicates it's importing a scene
	int newModelNum = 0; // Imported model's number

	// Display
	CListBox *outputBox = (CListBox*)GetDlgItem(IDC_LIST_OUTPUT);
	outputBox->AddString(_T(">-------------------------------------"));
	outputBox->AddString(_T("Analysing Model..."));

	DWORD startTime = GetTickCount();

	// Get file name extension
	char drive[_MAX_DRIVE]; char dir[_MAX_DIR];
	char fname[_MAX_FNAME]; char ext[_MAX_EXT];
	_splitpath_s(modelFile.c_str(), drive, dir, fname, ext);

	// Switch which model loader to use
	// OBJ File
	if (strcmp(ext, ".obj") == 0) {
		// Set loader parameters
		objLoader->threadNum = threadNum;
		objLoader->useCUDA = enableCUDA;
		objLoader->scale = vec4(import_scale, 1);
		objLoader->position = vec4(import_position, 0);
		objLoader->rotation = import_rotation;
		objLoader->uvShift = vec4(import_uvshift, 0, 0);
		objLoader->uvScale = vec4(import_uvscale, 0, 0);
		objLoader->uvRotation = import_uvRotation;
		objLoader->stickToGround = stick_to_ground;

		// Load model file
		newMeshNum = 
			objLoader->readFile(modelFile.c_str(), model, objectShader_basic, true);

		model[model.size() - 1].setupModelBuffers();
		addModelInfo();
	}
	// STL File
	else if (strcmp(ext, ".stl") == 0) {
		//...
		/* remove these in formal use */
		CString notifStr("Parser under development.");
		AfxMessageBox(notifStr);
		return -1;
	}
	// RXM File
	else if (strcmp(ext, ".rxm") == 0) {

		rxmImporter->threadNum = threadNum;
		rxmImporter->useCUDA = enableCUDA;
		rxmImporter->scale = vec4(import_scale, 1);
		rxmImporter->position = vec4(import_position, 0);
		rxmImporter->rotation = import_rotation;
		rxmImporter->uvShift = vec4(import_uvshift, 0, 0);
		rxmImporter->uvScale = vec4(import_uvscale, 0, 0);
		rxmImporter->uvRotation = import_uvRotation;
		rxmImporter->stickToGround = stick_to_ground;

		newMeshNum =
			rxmImporter->readRXMFile(modelFile.c_str(), model, objectShader_basic);

		// Revise new model's texture data
		reviseNewModel();

		model[model.size() - 1].setupModelBuffers();
		addModelInfo();
	}
	// RXM Scene File
	else if (strcmp(ext, ".rxms") == 0) {

		isScene = true;

		rxmImporter->threadNum = threadNum;
		rxmImporter->useCUDA = enableCUDA;
		rxmImporter->scale = vec4(import_scale, 1);
		rxmImporter->position = vec4(import_position, 0);
		rxmImporter->rotation = import_rotation;
		rxmImporter->uvShift = vec4(import_uvshift, 0, 0);
		rxmImporter->uvScale = vec4(import_uvscale, 0, 0);
		rxmImporter->uvRotation = import_uvRotation;
		rxmImporter->stickToGround = stick_to_ground;

		newModelNum = 
			rxmImporter->readRXMSFile(modelFile.c_str(), model, objectShader_basic);

		completeSceneImport(newModelNum);
	}
	// Add New Parser Here
	//...
	else // Error, shouldn't come here
		return -1;

	DWORD endTime = GetTickCount();

	// Display statistics
	CString text(modelFile.c_str());
	outputBox->AddString(text);

	CFileStatus rStatus;
	CFile::GetStatus(text, rStatus);
	ULONGLONG fSize = rStatus.m_size;
	text.Format(_T("File Size: %d bytes"), fSize);
	outputBox->AddString(text);

	text.Format(_T("Load Time: %d ms"), endTime - startTime);
	outputBox->AddString(text);

	if (!isScene) {
		for (int i = 0; i < model[model.size() - 1].mesh.size(); i++) {
			text.Format(_T("Mesh %d"), i);
			outputBox->AddString(text);
			text.Format(_T("Triangle Number: %d"), model[model.size() - 1].mesh[i].face_number);
			outputBox->AddString(text);
			text.Format(_T("Vertex Number: %d"), model[model.size() - 1].mesh[i].vertex_number);
			outputBox->AddString(text);
		}
	}
	else {
		text.Format(_T("Imported Model Number: %d"), newModelNum);
		outputBox->AddString(text);
	}

	// Highlight newly imported model if scene is already highlighted
	if (all_highlighted) {
		if (!isScene) {
			model[model.size() - 1].highlightModel(false, highlight_color);
		}
		else {
			int start = model.size() - 1, end = model.size() - 1 - newModelNum;
			for (int i = start; i > end; i--) {
				model[i].highlightModel(false, highlight_color);
			}
		}
	}

	if (firstModel) { // First import
		firstModel = false;
		GetDlgItem(IDC_COMBO_MOVE)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_TEXTURE)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_SPECULAR)->EnableWindow(true);
		setupGLfunctions();
	}
	else { // Consecutive import
		glutPostRedisplay();
	}

	return 0;
}
#pragma endregion

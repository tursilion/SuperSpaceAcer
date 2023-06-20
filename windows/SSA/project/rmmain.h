
#define SAFE_RELEASE(x) if (x != NULL) {x->Release(); x = NULL;}
#define MSG(msg) MessageBox( NULL, msg, "Application Message", MB_OK )

// Functions to build the customize the app for each sample
BOOL BuildScene( LPDIRECT3DRM3 pD3DRM,
				 LPDIRECT3DRMDEVICE3 dev, LPDIRECT3DRMVIEWPORT2 view,
				 LPDIRECT3DRMFRAME3 scene, LPDIRECT3DRMFRAME3 camera );
VOID OverrideDefaults( BOOL* bNoTextures, BOOL* pbResizingDisabled, 
					   BOOL* pbConstRenderQuality, CHAR** pstrName );

//-----------------------------------------------------------------------------
// GLOBAL VARIABLES
//-----------------------------------------------------------------------------
LPDIRECT3DRM3         g_pD3DRM      = NULL; // D3DRM object
LPDIRECTDRAWCLIPPER  g_pDDClipper  = NULL; // DDrawClipper object
LPDIRECT3DRMDEVICE3   g_pDevice     = NULL; // D3DRM device 
LPDIRECT3DRMVIEWPORT2 g_pViewport   = NULL; // D3DRM viewport
LPDIRECT3DRMFRAME3    g_pScene      = NULL; // Root frame for the scene
LPDIRECT3DRMFRAME3    g_pCamera     = NULL; // Camera's frame

#define MAX_DRIVERS 5               // Max # of D3D drivers to enumerate
GUID g_DriverGUID[MAX_DRIVERS];     // GUIDs of the available D3D drivers
CHAR g_DriverName[MAX_DRIVERS][50]; // names of the available D3D drivers
WORD g_wNumDrivers = 0;             // number of available D3D drivers
WORD g_wCurrDriver = 0;             // D3D driver currently being used

D3DRMRENDERQUALITY  g_eRenderQuality;        // Shade, fill and light state
D3DRMTEXTUREQUALITY g_eTextureQuality;       // Texture interpolation
BOOL                g_bDithering    = FALSE; // Dithering enable flag
BOOL                g_bAntialiasing = FALSE; // Antialiasing enable flag

BOOL g_bQuit               = FALSE; // Program is about to terminate
BOOL g_bInitialized        = FALSE; // Objects have been initialized
BOOL g_bMinimized          = FALSE; // Window is minimized
BOOL g_bSingleStepMode     = FALSE; // Render one frame at a time
BOOL g_bDrawAFrame         = FALSE; // Render the scene
BOOL g_bNoTextures         = FALSE; // This sample doesn't use any textures
BOOL g_bConstRenderQuality = FALSE; // Whether sample is not constructed
                                    // w/MeshBuilders and so the RenderQuality
                                    // cannot be changed

DWORD g_wBPP;                 // bit depth of the current display mode

WORD  g_wMouseButtons;         // mouse button state
WORD  g_wMouseX;               // mouse cursor x position
WORD  g_wMouseY;               // mouse cursor y position

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
LONG FAR PASCAL WndProc( HWND, UINT, WPARAM, LPARAM );
extern VOID     ReadMouse( WORD*, WORD*, WORD* );

HWND     InitApp();
VOID     InitGlobals();
HRESULT  CreateDevAndView( LPDIRECTDRAWCLIPPER lpDDClipper, WORD wDriver,
						   DWORD dwWidth, DWORD dwHeight );
HRESULT  RenderScene();
VOID     CleanUpAndPostQuit();
VOID     SetRenderState();
BOOL     BuildDeviceMenu( HMENU hmenu );

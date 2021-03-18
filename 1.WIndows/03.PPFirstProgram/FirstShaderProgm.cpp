#include <windows.h>
#include<stdio.h>
#include<gl/glew.h>
#include <gl/GL.h>


#include "vmath.h"

#define WIN_WIDTH 600
#define WIN_HEIGHT 600

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")


using namespace vmath;

enum 
{
	VDG_ATTRIBUTE_VERTEX =0,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE0,
};
//Prototype Of WndProc() declared Globally
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Global variable declarations
FILE *gpFile =NULL;

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullscreen = false;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

//main()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function prototype
	void initialize(void);
	void uninitialize(void);
	void display(void);
    
	//variable declaration
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("RTROGL");
	bool bDone = false;

	//code
	//creare Log file
	if(fopen_s(&gpFile,"Log.txt","w") != 0)
	{
		MessageBox(NULL,TEXT("Failed To create Log File\n.."),TEXT("ERROR"),MB_OK);
	    exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File is created successfully..\n");
	}
	//initializing members of struct WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;

	//Registering Class
	RegisterClassEx(&wndclass);
UINT uiWidth=GetSystemMetrics(SM_CXSCREEN);
UINT uiHeight=GetSystemMetrics(SM_CYSCREEN);

UINT uiXPos=(uiWidth/2)-(WIN_WIDTH/2);
UINT uiYPos=(uiHeight/2)-(WIN_HEIGHT/2);


	//Create Window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szClassName,
		TEXT("OpenGL programmable  Pipeline Using Native Windowing"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		uiXPos,
		uiYPos,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	//initialize
	initialize();

	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	
	//Message Loop
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true)
			{   
				display();
				if (gbEscapeKeyIsPressed == true)
					bDone = true;
				
			}
		}
	}

	uninitialize();
	return((int)msg.wParam);
}

//WndProc()
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function prototype
	//void display(void);
	void resize(int, int);
	void ToggleFullscreen(void);
	void uninitialize(void);
   
   //variable declarations
	static WORD xMouse = NULL;
	static WORD yMouse = NULL;
	//code
	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0) 
			gbActiveWindow = true;
		else 
			gbActiveWindow = false;
		break;
	//case WM_PAINT:
		//display();
		//break;
	case WM_ERASEBKGND:
		return(0);
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam)); 
		break;
	case WM_KEYDOWN: 
		switch (wParam)
		{
		case VK_ESCAPE:
			gbEscapeKeyIsPressed = true; 
			break;
		case 0x46: //for 'f' or 'F'
			if (gbFullscreen == false)
			{
				ToggleFullscreen();
				gbFullscreen = true;
			}
			else
			{
				ToggleFullscreen();
				gbFullscreen = false;
			}
			break;
		default:
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	//variable declarations
	MONITORINFO mi;

	//code
	if (gbFullscreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}

	else
	{
		//code
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

void initialize(void)
{
	//function prototypes
	void uninitialize(void);
	void resize(int, int);

	//variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	//code
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	//Initialization of structure 'PIXELFORMATDESCRIPTOR'
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; 
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits=32;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

   GLenum glew_error = glewInit();
   if(glew_error != GLEW_OK)
   {
   	wglDeleteContext(ghrc);
   	ghrc =NULL;
   	ReleaseDC(ghwnd,ghdc);
   	ghdc = NULL;
   }

   //**VERTEX SHADER ***
   //create Shader 
   gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

   //provide source code to shader
    const GLchar *vertexShaderSourceCode =
        "void main(void)"  \
        "{"    \
        "}";
    glShaderSource(gVertexShaderObject,1,(const char **)&vertexShaderSourceCode,NULL);
    
    //compile shader
    glCompileShader(gVertexShaderObject);

    //**Fragment Shader **
    //create Shader
    gFragmentShaderObject =glCreateShader(GL_FRAGMENT_SHADER);

    //provide source code to shader
    const GLchar *fragmentShaderSourceCode=
         "void main(void)"  \
         "{"     \
         "}";

    glShaderSource(gFragmentShaderObject,1,(const GLchar **)&fragmentShaderSourceCode,NULL);
    
    //compile shader
    glCompileShader(gFragmentShaderObject);

    //*** SHADER PROGRAM ***
    //create 
    gShaderProgramObject =glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(gShaderProgramObject,gVertexShaderObject);

    //attact fragment shader to shader program
    glAttachShader(gShaderProgramObject,gFragmentShaderObject);

    //link shader
    glLinkProgram(gShaderProgramObject);


    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    glEnable(GL_CULL_FACE);//we will always cull back faces for better performance
	
   //set background color to which it will display  even if it's empty.THIS LINE CAN BE IN drawRect();

	glClearColor(0.0f, 1.0f, 1.0f, 0.0f);//

	resize(WIN_WIDTH, WIN_HEIGHT);
}

void display(void)
{  
    
	  
	  
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//start using OpenGL program object
	glUseProgram(gShaderProgramObject);

	//OpenGL drawing

	//Stopj using OpenGL program object
	glUseProgram(0);


	SwapBuffers(ghdc);
}



void resize(int width, int height)
{
	//code
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	
}



void uninitialize(void)
{
	//UNINITIALIZATION CODE

	if (gbFullscreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);

	}

	//detach vertex shader from shader programm object 
	glDetachShader(gShaderProgramObject,gVertexShaderObject);

	//detach fragment shader from shader program object
	glDetachShader(gShaderProgramObject,gFragmentShaderObject);

	//delete vertex shader object
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject=0;

	//delete fragment shadet object 
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject=0;

	//delete shadet program object
	glDeleteShader(gShaderProgramObject);
	gShaderProgramObject=0;

	//unlink shader program
	glUseProgram(0);



	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ghrc);
	ghrc = NULL;

	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;
   
     if(gpFile)
     {
     	fprintf(gpFile, "Log file is closed successfully...\n");
        fclose(gpFile);
        gpFile=NULL;
     }
  
	DestroyWindow(ghwnd);
	ghwnd = NULL;
}


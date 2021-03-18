#include <windows.h>
#include <stdio.h>

#include <gl\glew.h>  //for GLSL extensions IMP: This line should be before gl.h

#include <gl\GL.h>

#include "vmath.h"
#include "GLError.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

#define WIN_WIDTH   800
#define WIN_HEIGHT  600

using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX =0,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE,
};

//callback function prototype
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

//Global variable declarations
FILE *gpFile =NULL;

HWND ghwnd =NULL;
HDC ghdc=   NULL;
HGLRC ghrc =NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev{sizeof(WINDOWPLACEMENT)};

bool gbActiveWindow        =false;
bool gbEscapeKeyIsPressed  =false;
bool gbFullscreen          =false;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_triangle;
GLuint gVao_square;

GLuint gVbo_triangle_position;
GLuint gVbo_triangle_color;
GLuint gVbo_square_position;


GLuint  gMVPUniform;

mat4 gPerspectiveProjectionMatrix;

//main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int iCmdShow)
{
	//function prototype
	void initialize(void);
	void uninitialize(void);
	void display(void);


	//variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] =TEXT("RTRAssignment");
	bool bDone =false;

   //code
	//create log file 
    if(fopen_s(&gpFile,"Log.txt","w") != 0)
    {
    	MessageBox(NULL,TEXT("failed To create log file...\n"),TEXT("ERROR"),MB_OK);
    	exit(0);

    }
    else
    {
    	fprintf(gpFile, TEXT("Log File is Created successfully..\n"));
    }

     
	//initializing members of struct WNDCLASSEX
	wndclass.cbSize =sizeof(WNDCLASSEX);
	wndclass.style  =CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hInstance=hInstance;
	wndclass.hbrBackground =(HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hIcon =LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wndclass.lpfnWndProc=WndProc;
	wndclass.lpszMenuName=NULL;
	wndclass.lpszClassName=szAppName;

  //Register class
	if(!RegisterClassEx(&wndclass))
	{
		MessageBox(NULL,TEXT("RegisterClass Failed"),TEXT("ERROR"),MB_OK);
			fclose(gpFile);
            gpFile=NULL;
			exit(0);
	}

	UINT uiWidth = GetSystemMetrics(SM_CXSCREEN);
	UINT uiHeight= GetSystemMetrics(SM_CYSCREEN);

	UINT uiXPos = (uiWidth/2)-(WIN_WIDTH/2);
	UINT uiYPos = (uiHeight/2) -(WIN_HEIGHT/2);

	//create window
	hwnd =CreateWindow(szAppName,
			TEXT("OpenGL Programmable Pipeline Color Triangle"),
			WS_OVERLAPPEDWINDOW ,
			uiXPos,
			uiYPos,
			WIN_WIDTH,
			WIN_HEIGHT,
			NULL,
			NULL,
			hInstance,
			NULL);


    if(!hwnd)
		{
			MessageBox(NULL,TEXT("CreateWindow failed"),TEXT("ERROR"),MB_OK);
            fclose(gpFile);
            gpFile=NULL;
			exit(0);
		}

   ghwnd =hwnd;

   ShowWindow(hwnd,iCmdShow);
   SetForegroundWindow(hwnd);
   SetFocus(hwnd);

   //initialize
   initialize();

   while(bDone == false)
   {
   	 //Remark: PeekMessage is NOT a blooking function 
   	 if(PeekMessage(&msg,          //pointer to structure for window message
   	 	          (HWND)NULL,      //handle to a window
   	 	          0,               //First message in queue
   	 	          0,               //last message in queue (select all messages)
   	 	          PM_REMOVE))      //Remove message from queue after processing from PeekMessage
       {
       	  if(msg.message ==WM_QUIT)   //if current message is quit message then exit game loop
             bDone=true;              //causes game loop to exit
          else
          {
          	TranslateMessage(&msg);  //Translate virtual key message into character message
          	DispatchMessage(&msg);  //Dispatches message to Window procedure
          }
       }

       else                                     //if there is no message queue then do rendering
       { 
       	   //rendering function 
       	    display();
       	if(gbActiveWindow ==true)               //if window is foreground (receiving keystrokes)
       	 { 
       		if(gbEscapeKeyIsPressed ==true)     //if escape key is pressed
       	      bDone=true;
       	 }
       }
   
   }
 uninitialize();

  return((int)msg.wParam);
}



//WNDPROC
LRESULT CALLBACK WndProc(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
	//function prototype
    void resize(int,int);
    void ToggleFullscreen(void);
	void uninitialize(void);

	//variable deaclarations
	static WORD xMouse = NULL;
	static WORD yMouse =NULL;

	switch(iMsg)
	{
		case WM_ACTIVATE:
			if(HIWORD(wParam) == 0)               //if 0,the window is active
				gbActiveWindow =true;
			else
				gbActiveWindow =false;           //if non-zero the window is not active
		break;
		case WM_ERASEBKGND:
			return(0);
		case WM_SIZE:
			resize(LOWORD(lParam),HIWORD(lParam));
		break;
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_ESCAPE:
					if(gbEscapeKeyIsPressed == false)
						gbEscapeKeyIsPressed=true;
					
 				break;
                case 0x46:  //for f or F
                 if(gbFullscreen ==false)
                 {
                 	ToggleFullscreen();
                 	gbFullscreen=true;

                 }
                 else
                 {
                 	ToggleFullscreen();
                 	gbFullscreen =false;
                 }
                break;
                default:
                break;
			}
		break;
		case WM_CLOSE:
            uninitialize();
		break;
		case WM_DESTROY:
		PostQuitMessage(0);
		break;
		default:
		break;

	}
	return(DefWindowProc(hwnd,iMsg,wParam,lParam));
}
void ToggleFullscreen(void)
{


	//varible declarations
	MONITORINFO mi;

	//code
	if(gbFullscreen == false)
	{
		dwStyle =GetWindowLong(ghwnd,GWL_STYLE);
		if(dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO)};
			if(GetWindowPlacement(ghwnd,&wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd,MONITORINFOF_PRIMARY),&mi))
			{
				//if wpPrev is successfully filled with current window placement and 
				//if mi is successfully filled with primary monitor info then 
				//S1:Remove WS_OVERLAPPEDWINDOW style
				//S2: Set window position by aligning left-top point's XY coordinates to 
				//monitors left-top coordinates and setting window width and height to
				//monitors width and height (effectively making window full screen)
				//SWP_NOZORDER:Dont change th Z-Order
				//SWP_FRAMECHANGED :Forces recalculation of non-client area
				SetWindowLong(ghwnd,GWL_STYLE,dwStyle & ~WS_OVERLAPPEDWINDOW);    //S1
				SetWindowPos(ghwnd,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,    //S2
				            mi.rcMonitor.right - mi.rcMonitor.left,
                            mi.rcMonitor.bottom-mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
					
			}
			//make cursor disappear
			ShowCursor(FALSE);
		}

	}
	else
	{
       //screen is full screen so,toggle to previously saved dimension
		//S1: Add WS_OVERLAPPEDWINDOW to window style via SetWindowLong
		//S2: Set window placement to stored previous placement in wpPrev via SetWindowPlacement
		//S3: Force the placement effects of SetWindowPlacement by call to SetWindowPos With
		//   SWP_NOMOVE :Don't change left top point (i.e ignore third and fourth parameter)
		//   SWP_NOSIZE :Don't change dimensions of window (i.e ignore fifth and sixth parameter)
		//   SWP_NOZORDER:Don't change Z-order of the window and its children
		//   SWP_NOOWNERZORDER: Don't change Z-order of owner of the window(refered by ghwnd)
		//   SWP_FRAMECHANGED :Force recalculation of non-client area.
		//S4: Make Sure Cursor is visible
        SetWindowLong(ghwnd,GWL_STYLE,dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd,&wpPrev);
        SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        ShowCursor(TRUE);
	}

}

void initialize(void)
{
	//function prototype
	void uninitialize(void);
	void resize(int,int);


	PIXELFORMATDESCRIPTOR pfd;  //Information strucutre describing the format
	int iPixelFormatindex =-1;  //Index to Pixel format structure in the internal tables


	//fill pfd by zeros
	ZeroMemory((void *)&pfd,sizeof(PIXELFORMATDESCRIPTOR));

	//Initialize PIXELFORMATDESCRIPTOR with desired values of its attribute
	//pfd.nSize : Allows Microsoft to maintainmultiple versions of structure
	//pfd.nVersion :version infomation
	//pfd.dwFlags
	//       PFD_DRAW_TO_WINDOW :Indiacates real time rendering ie.draw on window
	//       PFD_SUPPORT_OPENGL : Support for OpenGL rendering 
	//       PFD_DOUBLEBUFFER :  use DoubleBuffer
	//pfd.iPixelType: Type Of pixel format to be chosen 
	//pfd.cColorBits :Specifies color depth in bits 
	//pfd.cRedBits:   Specifiies bit depth for red color
	//pfd.cGreenBits: Specifiies bit depth for green color
	//pfd.cBlueBits:  Specifiies bit depth for blue color
	//pfd.cAlphabits: Specifiies bit depth for alpha component
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion =1;    
    pfd.dwFlags =PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL |PFD_DOUBLEBUFFER;
    pfd.iPixelType =PFD_TYPE_RGBA;
    pfd.cColorBits =32;
    pfd.cRedBits   =8;
    pfd.cGreenBits =8;
    pfd.cBlueBits  =8;
    pfd.cAlphaBits =8;
    pfd.cDepthBits =32;

    ghdc =GetDC(ghwnd);

    //choose a pixel format which is best mathches with that o f'pfd'
    iPixelFormatindex =ChoosePixelFormat(ghdc,&pfd);
    if(iPixelFormatindex == 0)
    {
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }

    //set the pixel format choosen above
    if(SetPixelFormat(ghdc,iPixelFormatindex,&pfd) == false)
    {
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }

    //create OpenGL rendering context
    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }

    //make the  rendering context created above as  current hdc
    if(wglMakeCurrent(ghdc,ghrc) ==false)
    {
    	wglDeleteContext(ghrc);
    	ghrc =NULL;
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;

    }

      //GLEW Initialization code for GLSL (IMP: It must be here. Means after  creating OpenGL Context but before using any OpenGL Function )
    GLenum glew_error = glewInit();
    if(glew_error != GLEW_OK)
    {
    	wglDeleteContext(ghrc);
    	ghrc=NULL;
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }
  fprintf(gpFile, "%s\n",glGetString(GL_VERSION));
  fprintf(gpFile, "%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION) );

//************************VERTEX SHADER****************
//create shader
 
 gVertexShaderObject =glCreateShader(GL_VERTEX_SHADER);
 check_gl_error();

 const GLchar *vertexShaderSourceCode =
 "#version 430 core"  \
 "\n"   \
 "in vec4 vPosition;"  \
 "in vec4 vColor;"   \
 "out vec4 out_color;"  \
 "uniform mat4 u_mvp_matrix;"  \
 "void main(void)"  \
 "{"    \
 "gl_Position=u_mvp_matrix * vPosition;"  \
 "out_color = vColor;"  \
 "}";


 //give this source code to shader object
 
 glShaderSource(gVertexShaderObject,
   	               1,                                       //No.Of strings
   	               (const char **)&vertexShaderSourceCode,
   	               NULL); //NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string

 check_gl_error();

 //compile shader
 glCompileShader(gVertexShaderObject);
 check_gl_error();
 
 GLint iInfoLogLength =0; //length of error string
 GLint iShaderCompilationStatus =0 ;  //parametarized return value of compilation status
 char *szInfoLog= NULL;     //pointer variable to hold the error in a string

 //get the shader compilation status of vertex shader object


 glGetShaderiv(gVertexShaderObject,GL_COMPILE_STATUS,&iShaderCompilationStatus);
 check_gl_error();
 if(iShaderCompilationStatus ==GL_FALSE)
 { 
 	//get the length of error in a variable iInfoLogLength
 	glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
 	check_gl_error();
 	if(iInfoLogLength >0)
 	{
 		szInfoLog = (char *)malloc(iInfoLogLength);
 		if(szInfoLog !=NULL)
 		{
 			GLsizei written;
           
           //get the error 
         		//S1: error characters length
         		//S2: number of characters actually written
         		//S3: pointer variable to hold the error string
           glGetShaderInfoLog(gVertexShaderObject,
           	                  iInfoLogLength,         //S1
           	                  &written,              //S2
           	                  szInfoLog);            //S3

           check_gl_error();
           fprintf(gpFile, "Vertex Shader Compilation Log:%s\n",szInfoLog );
           free(szInfoLog);
           uninitialize();
           exit(0);
 		}
 	}
 }

//**************************FRAGMENT SHADER******************
 //create shader
 gFragmentShaderObject=glCreateShader(GL_FRAGMENT_SHADER);
 check_gl_error();

 //provide a source code to Fragment shader
 const GLchar *fragmentShaderSourceCode=
         "#version 430 core"
         "\n"   \
         "in vec4 out_color;" \
         "out vec4 FragColor;"  \
          "void main(void)"
          "{"
          "FragColor = out_color;"  \
          "}";

          //give above fragment shader source code to gFragmentShaderObject
          glShaderSource(gFragmentShaderObject,
          	               1,                     //NO.of Strings
          	               (const char **)&fragmentShaderSourceCode,  //source code address
          	               NULL);//NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string
  //compile shader
          glCompileShader(gFragmentShaderObject);
          check_gl_error();

          iInfoLogLength=0;      //re-initialize the variables for good programming practice
          iShaderCompilationStatus=0;  //re-initialize the variables for good programming practice

          szInfoLog= NULL;
//get the compilation status in parameterized variable
glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompilationStatus);
check_gl_error();

if(iShaderCompilationStatus == GL_FALSE)
{
	glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
	check_gl_error();
	if(iInfoLogLength >0)
	{
		szInfoLog=(char *)malloc(iInfoLogLength);
		if(szInfoLog != NULL)
	   {		
		GLsizei written;
		//get the error 
 		//S1: error characters length
 		//S2: number of characters actually written
 		//S3: pointer variable to hold the error string
      
       glGetShaderInfoLog(gFragmentShaderObject,
      	                iInfoLogLength,          //S1
      	                &written,               //S2
      	                szInfoLog);             //S3
       check_gl_error();
       fprintf(gpFile, "Fragment Shader Compilation Log:%s\n",szInfoLog);
       free(szInfoLog);
       uninitialize();
       exit(0);
      }
	}
}

//**************SHADER PROGRAM*****************
//create program
gShaderProgramObject = glCreateProgram();
check_gl_error();

//attach vertex shader program to shader program
glAttachShader(gShaderProgramObject,gVertexShaderObject);
check_gl_error();
//attach fragment shader program to shader program
glAttachShader(gShaderProgramObject,gFragmentShaderObject);
check_gl_error();
// //pre-link binding of shader program object  with vertex shader position
     //bind our attribute i.e VDG_ATTRIBUTE_VERTEX to "vPosition" variable in vertex shader source code
//also bind our enum  color attribute to "vColor" in fragment shader source code
glBindAttribLocation(gShaderProgramObject,
	                 VDG_ATTRIBUTE_VERTEX,
	                 "vPosition");
check_gl_error();

glBindAttribLocation(gShaderProgramObject,
	                 VDG_ATTRIBUTE_COLOR,
	                 "vColor");
check_gl_error();


//link shader
glLinkProgram(gShaderProgramObject);
check_gl_error();

GLint iShaderProgramLinkStatus =0;
iInfoLogLength=0;
szInfoLog=NULL;


//get the linking error status in parameterized variable
glGetProgramiv(gShaderProgramObject,
	            GL_LINK_STATUS, 
	            &iShaderProgramLinkStatus);
check_gl_error();

if(iShaderProgramLinkStatus == GL_FALSE)
{
	glGetProgramiv(gShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
	check_gl_error();
	if(iInfoLogLength >0)
	{
		szInfoLog=(char *)malloc(iInfoLogLength);
		if(szInfoLog !=NULL)
		{
			GLsizei written;

			//get the error 
         		//S1: error characters length
         		//S2: number of characters actually written
         		//S3: pointer variable to hold the error string
			glGetProgramInfoLog(gShaderProgramObject,
				                iInfoLogLength,     //S1
				                &written,          //S2
				                szInfoLog);       //S3
            check_gl_error();
            fprintf(gpFile, "Shader Program Compilation Log:%s\n", szInfoLog);
            free(szInfoLog);
            uninitialize();
            exit(0);
		}
	}
}

//get MVP uniform location
    //bind uniform location variable ,i.e here u_mvp_matrix, from shader source code to ours variable "
    //so that we can use our variable to refer the uniform location in gpu 
    gMVPUniform =glGetUniformLocation(gShaderProgramObject,"u_mvp_matrix");
    check_gl_error();
    //// *** vertices,colors,shaders,attribs,vbo,vao,initializations ***
 

//###############Start Of TRIANGLE Relatred Vao And Vbo###############

    const GLfloat triangleVertices[]=
    {
    	0.0f,1.0f,0.0f,    //apex
    	-1.0f,-1.0f,0.0f,  //left bottom
    	1.0f,-1.0f,0.0f    //right bottom
    };

    const GLfloat triangleColor[]=
    {
    	1.0f,0.0f,0.0f,    //apex
    	0.0f,1.0f,0.0f,    //left bottom
    	0.0f,0.0f,1.0f     //right bottom
    };
 

    glGenVertexArrays(1,&gVao_triangle);//i want to run following  instructions    letsay "x" no of instructions 
                             //so give me the key to run those instructions repeatedly.
    check_gl_error();
  
  glBindVertexArray(gVao_triangle);    //get binded with the above key 
                             //following operations will be regarding this vao
                             //because OpenGL is a state machine
   check_gl_error();

glGenBuffers(1,&gVbo_triangle_position);//i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
check_gl_error();

glBindBuffer(GL_ARRAY_BUFFER,gVbo_triangle_position);//get binded with that key i.e gVbo

  //now following operations will affect this gVbo
   //Following function will send our data from CPU to GPU
    //S1: send the data in gVbo area which comes under Array Buffer area in GPU
    //S2: size of data =no.of elements * size of data type in array
    //S3: actual data
    //S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 
glBufferData(GL_ARRAY_BUFFER,             //S1
	         sizeof(triangleVertices),    //S2
	         triangleVertices,           //S3
	         GL_STATIC_DRAW);            //S4

check_gl_error();

//following function will attach our enum variable to the above data her square data
//S1.our enum variable name so that it will get attached to the above data which is in above vbo,our enum variable has already attached to the variable in shader,so indirectly here we are attaching that shader varible to the data we are passing to GPU in above steps
//S2.here we tells that vertex position is made up of x,y,z
//S3.data is of float type
//S4.is this interleaved data(i.e all vertex attributes(i.e color,position,normal,texture) are packed in one buffer)?GL_FALSE means NO
//S5.offset for interleaved data
//S6.address of interleaved data buffer
glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,           //S1
	                   3,                           //S2
	                   GL_FLOAT,                    //S4
	                   GL_FALSE,                   //S5
	                   0,NULL);                   //S6

check_gl_error();

//get binded with our enum variable which is linked to "vPosition" variable from vertex shader code
glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
check_gl_error();

//unbind vbo_triangle_position
glBindBuffer(GL_ARRAY_BUFFER,0);

glGenBuffers(1,&gVbo_triangle_color);//i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
check_gl_error();

glBindBuffer(GL_ARRAY_BUFFER,gVbo_triangle_color);//get binded with that key i.e gVbo
check_gl_error();
//now following operations will affect this gVbo
   //Following function will send our data from CPU to GPU
    //S1: send the data in gVbo area which comes under Array Buffer area in GPU
    //S2: size of data =no.of elements * size of data type in array
    //S3: actual data
    //S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 

glBufferData(GL_ARRAY_BUFFER,                        //S1
	         sizeof(triangleColor),                  //S2
	         triangleColor,                         //S3
  	         GL_STATIC_DRAW );                      //S4
check_gl_error();
//following function will attach our enum variable to the above data her square data
//S1.our enum variable name so that it will get attached to the above data which is in above vbo,our enum variable has already attached to the variable in shader,so indirectly here we are attaching that shader varible to the data we are passing to GPU in above steps
//S2.here we tells that vertex color is made up of rgb
//S3.data is of float type
//S4.is this interleaved data(i.e all vertex attributes(i.e color,position,normal,texture) are packed in one buffer)?GL_FALSE means NO
//S5.offset for interleaved data
//S6.address of interleaved data buffer

glVertexAttribPointer(VDG_ATTRIBUTE_COLOR,      //S1
	                  3,                       //S2
	                  GL_FLOAT,               //S3
	                  GL_FALSE,              //S4
	                  0,                     //S5
	                  NULL);                //S6
check_gl_error();

glEnableVertexAttribArray(VDG_ATTRIBUTE_COLOR);
check_gl_error();

//unbind vbo_triangle_color
glBindBuffer(GL_ARRAY_BUFFER,0);
check_gl_error();

glBindVertexArray(0);
check_gl_error();

//#################End of TRIANGLE Related Vao and Vbo###################


//#################Start of Square Related Vao and Vbo###################

const GLfloat squareVertices[]=
{
	1.0f,1.0f,0.0f,    //right-top corner
	-1.0f,1.0f,0.0f,   //left-top corner
	-1.0f,-1.0f,0.0f,  //bottom-left corner
	1.0f,-1.0f,0.0f    //bottom-right corner
};


glGenVertexArrays(1,&gVao_square);//i want to run following  instructions    letsay "x" no of instructions 
                             //so give me the key to run those instructions repeatedly.
check_gl_error();

glBindVertexArray(gVao_square);//get binded with the above key 
                             //following operations will be regarding this vao
                             //because OpenGL is a state machine

check_gl_error();


glGenBuffers(1,&gVbo_square_position);//i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
check_gl_error();

glBindBuffer(GL_ARRAY_BUFFER,gVbo_square_position);//get binded with that key i.e gVbo
check_gl_error();

  //now following operations will affect this gVbo
   //Following function will send our data from CPU to GPU
    //S1: send the data in gVbo area which comes under Array Buffer area in GPU
    //S2: size of data =no.of elements * size of data type in array
    //S3: actual data
    //S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 


glBufferData(GL_ARRAY_BUFFER,                   //S1
	         sizeof(squareVertices),             //S2
	         squareVertices,                     //S3
	         GL_STATIC_DRAW);                    //S4

check_gl_error();

//following function will attach our enum variable to the above data her square data
//S1.our enum variable name so that it will get attached to the above data which is in above vbo,our enum variable has already attached to the variable in shader,so indirectly here we are attaching that shader varible to the data we are passing to GPU in above steps
//S2.here we tells that vertex position is made up of x,y,z
//S3.data is of float type
//S4.is this interleaved data(i.e all vertex attributes(i.e color,position,normal,texture) are packed in one buffer)?GL_FALSE means NO
//S5.offset for interleaved data
//S6.address of interleaved data buffer


glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,
	                  3,
	                  GL_FLOAT,
	                  GL_FALSE,
	                  0,
	                  NULL);
check_gl_error();

glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
check_gl_error();

glBindBuffer(GL_ARRAY_BUFFER,0);
check_gl_error();

//When you wnat to give only one color instead of creating Vbo just use following function to avoid overhead  
glVertexAttrib3f(VDG_ATTRIBUTE_COLOR,0.0f,0.0f,1.0f);
check_gl_error();

//glEnableVertexAttribArray(VDG_ATTRIBUTE_COLOR);
//check_gl_error();

glBindVertexArray(0);


glShadeModel(GL_SMOOTH);   //set the shading mdel

  glEnable(GL_DEPTH_TEST);    //enable depth test in the state machine
  glDepthFunc(GL_LEQUAL);      //Specify the depth comparison function .GL_LEQUAL:paases if incoming z value is less than
                              //or equal to stored z value
  glClearDepth(1.0f);     //specifies the clear value for depth buffer
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
  glEnable(GL_CULL_FACE);   //we will always cull faces for better performance

 //set background color to which it will display even if it will empty.
  glClearColor(0.0f,0.0f,0.0f,0.0f);///black

  //set orthographicMatrix to identity matrix
  gPerspectiveProjectionMatrix=mat4:: identity();
  resize(WIN_WIDTH,WIN_HEIGHT);
 
 }

 void display(void)
 {

 	//code
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);

 	//start using OpenGL progrma object
 	glUseProgram(gShaderProgramObject);
 	check_gl_error();

 	//***********************TRIANGLE *************************
     	//OpenGL drawing 
	 //set modelView & ModelViewPorjection matrices to identity
	 //mat4 is 4x4 matrix
  

  mat4 modelViewMatrix= mat4::identity();
  mat4 modelViewPorjectionMatrix=mat4::identity();
  mat4 translationMatrix=mat4::identity();
  

  translationMatrix=modelViewMatrix * vmath::translate(-1.5f,0.0f,-6.0f);
  modelViewMatrix=modelViewMatrix * translationMatrix;


  //multiply the modelView and perspective matrix to get modelViewProjection matrix
modelViewPorjectionMatrix=gPerspectiveProjectionMatrix * modelViewMatrix;
//ORDER IS IMPORTANT

 //pass above modelviewprojection matrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated  by using glGetUniformLocation()

glUniformMatrix4fv(gMVPUniform,
	               1,                //no.of matrices
	               GL_FALSE,         //do not take transpose of matrix
	               modelViewPorjectionMatrix);
check_gl_error();
//********************bind vao_triangle********************
glBindVertexArray(gVao_triangle);
check_gl_error();

glDrawArrays(GL_TRIANGLES,0,3); //3 vertices
check_gl_error();
//unbind vao
glBindVertexArray(0);
check_gl_error();


//************************SQUARE *******************8
//OpenGL drawing 
	 //set modelView & ModelViewPorjection matrices to identity
	 //mat4 is 4x4 matrix
modelViewMatrix = mat4::identity();
modelViewPorjectionMatrix=mat4::identity();
translationMatrix=mat4::identity();


translationMatrix=modelViewMatrix * vmath::translate(1.5f,0.0f,-6.0f);
modelViewMatrix=modelViewMatrix * translationMatrix;

  
  //multiply the modelView and orthographic matrix to get modelViewProjection matrix
modelViewPorjectionMatrix =gPerspectiveProjectionMatrix * modelViewMatrix;

 //pass above modelviewprojection matrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated  by using glGetUniformLocation()
glUniformMatrix4fv(gMVPUniform,
	               1,           //NO.of matrices
	               GL_FALSE,     //do not take transpose of matrix
	               modelViewPorjectionMatrix);
check_gl_error();

//*************bind vao_square****************
glBindVertexArray(gVao_square);
check_gl_error();

glDrawArrays(GL_TRIANGLE_FAN,0,4);
check_gl_error();

//unbind vao_square
glBindVertexArray(0);
check_gl_error();



glUseProgram(0);
check_gl_error();

SwapBuffers(ghdc);

 }


void resize(int width,int height)
{
	if(height ==0)
		height=1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);

	//glPerspective
	gPerspectiveProjectionMatrix=perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

}
void uninitialize(void)
{
	if(gbFullscreen == true)
	{
		dwStyle =GetWindowLong(ghwnd,GWL_STYLE);
		SetWindowLong(ghwnd,GWL_STYLE,dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd,&wpPrev);
		SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE |SWP_NOSIZE|SWP_NOZORDER |SWP_NOOWNERZORDER |SWP_FRAMECHANGED);
        ShowCursor(TRUE);

	}

	//destroy vao
	if(gVao_triangle)
	{
		glDeleteVertexArrays(1,&gVao_triangle);
		gVao_triangle=0;
	}
    if(gVao_square)
	{
		glDeleteVertexArrays(1,&gVao_square);
		gVao_square=0;
	}
      

	//destroy vbo
	if(gVbo_triangle_position)
	{
		glDeleteBuffers(1,&gVbo_triangle_position);
		gVbo_triangle_position=0;
	}
	if(gVbo_triangle_color)
	{
		glDeleteBuffers(1,&gVbo_triangle_color);
		gVbo_triangle_color=0;
	}
	if(gVbo_square_position)
	{
		glDeleteBuffers(1,&gVbo_square_position);
		gVbo_square_position=0;
	}
    //detach vertex shader from shader program object
    glDetachShader(gShaderProgramObject,gVertexShaderObject);

    //detach fragment shader from shader program object
    glDetachShader(gShaderProgramObject,gFragmentShaderObject);

    //delete  vertex shader object
    glDeleteShader(gVertexShaderObject);
    gVertexShaderObject=0;

    //delelte frgment shader object
    glDeleteShader(gFragmentShaderObject);
    gFragmentShaderObject=0;

    //delete shader progrma object
    glDeleteProgram(gShaderProgramObject);
    gShaderProgramObject=0;

    //unlink shader program
    glUseProgram(0);

    //deselect the rendering context
    wglDeleteContext(ghrc);
    ghrc=0;

    //delete the devise context
    ReleaseDC(ghwnd,ghdc);
    ghdc=0;

    if(gpFile)
    {
    	fprintf(gpFile, "Log File is closed successfully\n" );
    	fclose(gpFile);
    	gpFile=0;
    } 

    
}






























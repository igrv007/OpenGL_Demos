#include <windows.h>
#include <stdio.h>

#include <gl\glew.h> //for GLSL extensions IMP: This line should be before gl.h
#include <gl\GL.h>

#include "vmath.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

#define WIN_WIDTH  800
#define WIN_HEIGHT 600

using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX =0,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE,
};

//Callback function prototype
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

//global variable declaratinos
FILE *gpFile = NULL;

HWND ghwnd =NULL;
HDC ghdc   =NULL;
HGLRC ghrc =NULL;


DWORD dwStyle;
WINDOWPLACEMENT wpPrev ={sizeof(WINDOWPLACEMENT)};

bool gbActiveWindow       =false;
bool gbEscapeKeyIsPressed =false;
bool gbFullscreen         =false;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao;
GLuint gVbo;
GLuint gMVPUniform;

mat4 gOrthographicProjectionMatrix;

//main()

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int iCmdShow)
{
   //function prototype 
	void initialize(void);
	void uninitialize(void);
	void display(void);

	//variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] =TEXT("RTRAssignment");
	bool bDone= false;


	//code
	//create log file 
	if(fopen_s(&gpFile,"Log.txt","w") != 0)
	{
		MessageBox(NULL,TEXT("failed to create Log file..\n"),TEXT("ERROR"),MB_OK);
	    exit(0);
	}
	else
	{
		
		fprintf(gpFile, "Log file is opened successfully\n");
	}

 //initializing members of struct WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style  = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
	wndclass.cbClsExtra =0;
	wndclass.cbWndExtra =0;
	wndclass.hInstance  =hInstance;
	wndclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon        =LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor      =LoadCursor(NULL,IDC_ARROW);
	wndclass.hIconSm      =LoadIcon(NULL,IDI_APPLICATION);
	wndclass.lpfnWndProc  =WndProc;
	wndclass.lpszClassName=szClassName;
	wndclass.lpszMenuName =NULL;

	//Register Class
	if(!RegisterClassEx(&wndclass))
		MessageBox(NULL,TEXT("Register class failed ..\n"),TEXT("ERROR"),MB_OK);

	//create Window
	//parallel to glutInitWindowSize(),glutInitWindowPosition(),glutCreateWindow() altogether
	hwnd = CreateWindow(szClassName,
		                 TEXT("OpenGL Programmable Pipeline OrthoTriangle"),
		                 WS_OVERLAPPEDWINDOW,
		                 100,
		                 100,
		                 WIN_WIDTH,
		                 WIN_HEIGHT,
		                 NULL,
		                 NULL,
		                 hInstance,
		                 NULL);
  if(!hwnd)
  	MessageBox(NULL,TEXT("create windwo failed ..\n"),TEXT("ERROR"),MB_OK);

	ghwnd =hwnd;

	ShowWindow(hwnd,iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//initialize
	initialize();

	//Game loop
	//parallel to glutMainLoop();
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

  uninitialize();   //destroy resources

 return((int)msg.wParam);  //return from winmain

}

//WNDPROC
LRESULT CALLBACK WndProc(HWND hwnd,UINT iMsg ,WPARAM wParam, LPARAM lParam)
{
   //prototypes of functions called from WndProc
	void resize(int,int);      
	void ToggleFullscreen(void);
	void uninitialize(void);

	//variable declarations
	static WORD xMouse =NULL;
	static WORD yMouse =NULL;

	//code
	switch(iMsg)
	{
		//handler of an event :Activate or deactivate the window
		case WM_ACTIVATE:
		   if(HIWORD(wParam) == 0)  //if 0,the window is active
		   	gbActiveWindow =true;
		   else                     //if non-zero the window is not active
		     gbActiveWindow =false;
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
		       	  if(gbEscapeKeyIsPressed ==false)
		       	  	gbEscapeKeyIsPressed =true;
		       	break;
		       	case 0x46 :  //for 'f' or 'F'
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
		case WM_LBUTTONDOWN:
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

//Function definitions
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
    //GLEW Initialization code for GLSL  (IMP: It must be here. Means after  creating OpenGL Context but before using any OpenGL Function )
    GLenum glew_error =glewInit();
    if(glew_error != GLEW_OK)
    {
    	wglDeleteContext(ghrc);
    	ghrc=NULL;
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }
    fprintf(gpFile, "%s\n",glGetString(GL_VERSION));
    fprintf(gpFile, "%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    

    //**** VERTEX SHADER *****
    //create shader
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //provide source code to shader
    const GLchar *vertexShaderSourceCode=
         "#version 430 core" \
         "\n"     \
         "in vec4 vPosition;" \
         "uniform mat4 u_mvp_matrix;" \
         "void main(void)"  \
         "{" \
         "gl_Position = u_mvp_matrix * vPosition;" \
         "}";



     //give the above source code to shader object
      glShaderSource(gVertexShaderObject,   
      	              1,                   //No.of strings
                      (const char **)&vertexShaderSourceCode,
                      NULL);             //NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string

      //compile shader
      glCompileShader(gVertexShaderObject);
      GLint iInfoLogLength  = 0;        //length of error string 
      GLint iShaderCompiledStatus =0;   //parametarized return value of compilation status
      char *szInfoLog =NULL;            //pointer variable to hold the error in a string
      

      //get the compilation status of vertexShaderObject
      glGetShaderiv(gVertexShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
      if(iShaderCompiledStatus ==GL_FALSE)    //i.e there is a compilation error
      {
      	//get the length of error in a variable iInfoLogLength
      	 glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
         if(iInfoLogLength >0)
         {
         	szInfoLog  =(char *)malloc(iInfoLogLength);
         	if(szInfoLog != NULL)
         	{
         		GLsizei written;
         		
         		//get the error 
         		//S1: error characters length
         		//S2: number of characters actually written
         		//S3: pointer variable to hold the error string
         		glGetShaderInfoLog(gVertexShaderObject,
         			               iInfoLogLength,       //S1
         			               &written,             //S2
         			               szInfoLog);           //S3
         		fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
         	    free(szInfoLog);
         	    uninitialize();
         	    exit(0);
         	}
         }
      }

      // ***** FRAGMENT SHADER *****
      //create shader
      gFragmentShaderObject =glCreateShader(GL_FRAGMENT_SHADER);

      //provide source code to shader
      const GLchar *fragmentShaderSourceCode=
      "#version 430 core" \
      "\n" \
      "out vec4 FragColor;"\
      "void main(void)"  \
      "{" \
      "FragColor =vec4(1.0,1.0,1.0,1.0);" \
      "}";

     //give above fragment shader programm to FrgmentShaderObject
      glShaderSource(gFragmentShaderObject,
      	             1,                     //no.of strings
      	             (const char **)&fragmentShaderSourceCode,
      	              NULL);                 //NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string
     //compile shader 
      glCompileShader(gFragmentShaderObject);
       iInfoLogLength =0;          //re-initialize the variables for good programming practice
       iShaderCompiledStatus =0;   //re-initialize the variables for good programming practice
     //get the compilation status in parameterized variable
     glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
     if(iShaderCompiledStatus ==GL_FALSE)    //i.e there is a compilation error
     {
     	//get the error length in parameterized return value
     	glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
     	if(iInfoLogLength > 0)
     	{
     		szInfoLog =(char *)malloc(iInfoLogLength);
     		if(szInfoLog !=NULL)
     		{
     			GLsizei written;
     			
     			//get the error 
         		//S1: error characters length
         		//S2: number of characters actually written
         		//S3: pointer variable to hold the error string
         		glGetShaderInfoLog(gFragmentShaderObject,
         			                iInfoLogLength,        //S1
         			                &written,              //S2
         			                szInfoLog);            //S3

         		fprintf(gpFile, "Fragment Shader Compilation Log :%s\n", szInfoLog );
         		free(szInfoLog);
         		uninitialize();
         		exit(0);
     		}
     	}
     }

     // ****** SHADER PROGRAM ******
     //Create  shader program
     gShaderProgramObject =glCreateProgram();

     //attach vertex shader to shader program
     glAttachShader(gShaderProgramObject,gVertexShaderObject);

     //attach fragment shader to shader program
     glAttachShader(gShaderProgramObject,gFragmentShaderObject);

     //pre-link binding of shader program object  with vertex shader position
     //bind our attribute i.e VDG_ATTRIBUTE_VERTEX to "vPosition" variable in vertex shader source code
     glBindAttribLocation(gShaderProgramObject,
     	                  VDG_ATTRIBUTE_VERTEX,
     	                  "vPosition");

     //link shader
     glLinkProgram(gShaderProgramObject);
     GLint iShaderProgramLinkStatus =0;
     //get the linking error status in parameterized variable
     glGetProgramiv(gShaderProgramObject,GL_LINK_STATUS,&iShaderProgramLinkStatus);
    
    if(iShaderProgramLinkStatus ==GL_FALSE) //i.e there is a linking error
    {
    	//get the length of error string in a parameterized variable
    	glGetProgramiv(gShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
    	if(iInfoLogLength > 0)
    	{
    		szInfoLog = (char *)malloc(iInfoLogLength);
    		if(szInfoLog != NULL)
    		{
    			GLsizei written;

     			//get the error 
         		//S1: error characters length
         		//S2: number of characters actually written
         		//S3: pointer variable to hold the error string
         		glGetProgramInfoLog(gShaderProgramObject,
         			                iInfoLogLength,          //S1
         			                &written,                //S2
         			                szInfoLog);              //S3
         		fprintf(gpFile, "Shader Program Link  Log :%s\n",szInfoLog );
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

    // *** vertices,colors,shaders,attribs,vbo,vao,initializations ***
    const GLfloat triangleVertices[] =
    {
    	0.0f,50.0f,0.0f,  //Apex
    	-50.0f,-50.0f,0.0f, //left-bottom
    	50.0f,-50.0f,0.0f //right-bottom

    };

    glGenVertexArrays(1,    //No. of vao
    	              &gVao) ;//i want to run following  instructions letsay "x" no of instructions 
                             //so give me the key to run those instructions repeatedly.
    glBindVertexArray(gVao);//get binded with the above key 
                             //following operations will be regarding this vao
                             //because OpenGL is a state machine

    glGenBuffers(1,&gVbo);   //i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
    glBindBuffer(GL_ARRAY_BUFFER,gVbo);//get binded with that key i.e gVbo
                                         //now following operations will affect this gVbo
   
   //Following function will send our data from CPU to GPU
    //S1: send the data in gVbo area which comes under Array Buffer area in GPU
    //S2: size of data =no.of elements * size of data type in array
    //S3: actual data
    //S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 
    glBufferData(GL_ARRAY_BUFFER,    //S1
    	          sizeof(triangleVertices),  //S2
    	          triangleVertices,           //S3
    	          GL_STATIC_DRAW);             //S4
   
   glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,
   	                      3,
   	                      GL_FLOAT,
   	                      GL_FALSE,
   	                      0,
   	                      NULL);
  

  //get binded with our enum variable which is linked to "vPosition" variable from vertex shader code
   glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

  //Unbind the GL_ARRAY_BUFFER by passing "0" as second parameter
   glBindBuffer(GL_ARRAY_BUFFER,0);
   
   //Unbind the vao 
   glBindVertexArray(0);

   glShadeModel(GL_SMOOTH); //set the shading model

   glEnable(GL_DEPTH_TEST);   //Enable Depth test in the state machine
   glDepthFunc(GL_LEQUAL);    //Specify the depth comparison function .GL_LEQUAL:paases if incoming z value is less than
                              //or equal to stored z value
   glClearDepth(1.0f);     //specifies the clear value for depth buffer
   glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

   glEnable(GL_CULL_FACE);  //we will always cull faces for better performance

   //set background color to which it will display even if it will empty.
    glClearColor(0.0f,0.0f,1.0f,0.0f); //blue

    //set orthographicMatrix to identity matrix
    gOrthographicProjectionMatrix = mat4::identity();

    ///resize
    resize(WIN_WIDTH,WIN_HEIGHT);

}

void display(void)
{

	//code 
	glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  
     //start using OpenGL Program object 
	glUseProgram(gShaderProgramObject);

	//OpenGL Drawing 
	//set modelView & modelViewProjection matrices to identity
	mat4 modelViewMatrix =mat4::identity();             //mat4 is 4x4 matrix
	mat4 modelViewProjectionMatrix = mat4::identity();

	//multiply the modelView and orthographic matrix to get modelViewProjection matrix
	modelViewProjectionMatrix = gOrthographicProjectionMatrix * modelViewMatrix ; //ORDER IS IMPORTANT
    
    //pass above modelviewprojection matrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated  by using glGetUniformLocation()
    glUniformMatrix4fv(gMVPUniform,1,GL_FALSE,modelViewProjectionMatrix);

    //*** bind vao ****
    glBindVertexArray(gVao);

    //here we are telling shaders to draw triangle using the data we had pushed in initiliazation
     glDrawArrays(GL_TRIANGLES,0,3); //3(each with its x,y,z) vertices in triangle vertices array

     //****unbind vao***
     glBindVertexArray(0);

     //stop using OpenGL program object
     glUseProgram(0);

     SwapBuffers(ghdc);
}

void resize(int width,int height)
{
	if (height ==0)
		height =1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);

	// PMG correction -> typecast width and height by float or GLfloat

	//glOrtho(left,right ,bottom,top,near,far)
	 if(width <= height)
    gOrthographicProjectionMatrix=ortho((-100.0f * ((GLfloat)height / (GLfloat)width)),(100.0f * ((GLfloat)height /(GLfloat)width)),-100.0f ,100.0f  ,-100.0f,100.0f);
  else
    gOrthographicProjectionMatrix=ortho((-100.0f * ((GLfloat)width /(GLfloat)height)),(100.0f *((GLfloat)width /(GLfloat)height)),-100.0f,100.0f,-100.0f,100.0f);
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
	if(gVao)
	{
		glDeleteVertexArrays(1,&gVao);
		gVao=0;
	}

	//destroy vbo
	if(gVbo)
	{
		glDeleteBuffers(1,&gVbo);
		gVbo=0;
	}

	//detach vertex shader from shader program object
	glDetachShader(gShaderProgramObject,gVertexShaderObject);

	//detach fragment shader from shader program object
	glDetachShader(gShaderProgramObject,gFragmentShaderObject);

	//delete vertex shader object
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject=0;

	//deleta fragmetn shade object
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject=0;

	//delete shadeer program object
	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject=0;

	//unlink shader program
	glUseProgram(0);

	//Deselect the rendering conrtext
	wglMakeCurrent(NULL,NULL);

	//delete th erendering context
	wglDeleteContext(ghrc);
	ghrc=0;

	//delete the devise context
	ReleaseDC(ghwnd,ghdc);
	ghdc=NULL;

	if(gpFile)
	{
		fprintf(gpFile, "Log file is successfully closed\n");
		fclose(gpFile);
		gpFile=NULL;
	}
}






















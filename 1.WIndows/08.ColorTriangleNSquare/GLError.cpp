#include "GLError.h"

void _check_gl_error(const char *file, int line)
{

	GLenum err;
	//int retCode=0;

	err= glGetError();
  
  if(err !=GL_NO_ERROR)
  {
  	
  	fprintf(gpFile, "glError in file:%s at line :%d ERRORCODE:%d\n",file,line,err);
  }
}


#include <stdio.h>
#include <GL/gl.h>



#ifndef GLERROR_H
#define GLERROR_H



void _check_gl_error(const char *file,int line);

#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
extern FILE *gpFile;

void _check_gl_error(const char *file, int line)
{

	GLint err;
	//int retCode=0;

	err= glGetError();
  
  if(err !=GL_NO_ERROR)
  {
  	
  	fprintf(gpFile, "glError in file:%s at line :%d ERRORCODE:%d\n",file,line,err);
  }
}
#endif //GLERROR_H


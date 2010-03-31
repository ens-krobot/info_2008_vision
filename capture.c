// Programme permettant de voir l'image d'une webcam
// et d'en faire des shots

#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
  CvCapture *capture = NULL;
  IplImage *frame = NULL;
  unsigned int compteur = 0, source = 0, ncams;
  char c, file[51];

  ncams = (argc == 2 ? atoi(argv[1]) : 1);
  capture = cvCaptureFromCAM(source);

  printf("Programme de capture d'image depuis une webcam\n\n");
  printf("Commandes :\n");
  printf("\tq : quitter\n");
  printf("\tc : cycler au travers des différentes webcam coonectées\n");
  printf("\tEspace : sauvegarder une image\n\n");
  printf("\t\tPasser en argument le nombre de webcams si plus d'une.\n\n\n");

  cvNamedWindow("Capture", 0);

  while (1)
  {
    frame = cvQueryFrame(capture);
    if(frame==NULL)
    {
      printf("Impossible de lire l'image !\n");
    }
    else
    {
      cvShowImage("Capture", frame);

      c = (char)cvWaitKey(10);

      if (c=='q') break;
      switch (c)
      {
      case 'c' :
	source = (source + 1 ) % ncams;
	cvReleaseCapture(&capture);
	capture = cvCaptureFromCAM(source);
	break;
      
      case ' ' :
	compteur++;
	sprintf(file, "img%d.jpg", compteur);
	cvSaveImage(file,frame);
      }
    }
  }

  cvReleaseCapture(&capture);
  cvDestroyWindow("Capture");

  return 0;
}

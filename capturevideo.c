// Programme permettant de récupérer une video par la webcam
// sous forme d'une série d'images tiff

#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
  CvCapture *capture = NULL;
  IplImage *frame = NULL;
  unsigned int compteur = 0, source = 0, ncams;
  char c, file[60], basename[51], ext[5];
  int isFilming = 0;
  double t;

  ncams = (argc == 2 ? atoi(argv[1]) : 1);
  capture = cvCaptureFromCAM(source);

  printf("Programme de capture de video depuis une webcam\n\n");
  printf("Commandes :\n");
  printf("\tq : quitter\n");
  printf("\tc : cycler au travers des différentes webcam coonectées\n");
  printf("\t\t(ne marche que quand le film n'est pas en cours d'acquisition\n");
  printf("\tEspace : commencer/areter de filmer\n\n");
  printf("\t\tPasser en argument le nombre de webcams si plus d'une.\n\n\n");

  printf("\n\nNom de la série d'images : ");
  scanf("%s", basename);
  printf("\nExtension des images : ");
  scanf("%s", ext);

  cvNamedWindow("Capture", 0);

  t = (double) cvGetTickCount();

  while (1)
  {
    // On récupère une image
    frame = cvQueryFrame(capture);
    if(frame==NULL)
    {
      printf("Impossible de lire l'image !\n");
    }
    else
    {
      // On l'affiche dans la fenêtre
      cvShowImage("Capture", frame);

      c = (char)cvWaitKey(5);

      // On gère l'appui sur une touche du clavier
      if (c=='q') break;
      switch (c)
      {
      case 'c' :
	if (!isFilming)
	{
	  source = (source + 1 ) % ncams;
	  cvReleaseCapture(&capture);
	  capture = cvCaptureFromCAM(source);
	  break;
	}
      
      case ' ' :
	isFilming = !isFilming;
      }

      // Si on doit filmer, on vérifie qu'il est temps de prendre une image
      // On filme a 25 images par secondes
      if (isFilming && ((double)cvGetTickCount() - t >= 0.04))
      {
	// On prend une image et on remet à jour le compteur de temps
	compteur++;
	sprintf(file, "%s%.3d.%s", basename, compteur, ext);
	cvSaveImage(file, frame);
      }
    }
  }

  cvReleaseCapture(&capture);
  cvDestroyWindow("Capture");

  return 0;
}

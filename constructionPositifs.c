// Programme permettant de traiter une série d'images et de créer une liste
// des fichiers avec les occurences d'un objet dans celles-ci

#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <string.h>

IplImage *image = NULL;
CvPoint origin;
CvRect selection;
int select_object = 0;

// Fonction de callback pour gérer la selection à la souris
void on_mouse(int event, int x, int y, int flags, void* param)
{
  if (image != NULL)
  {
    if (image->origin)
    {
      x = image->width - x;
      y = image->height - y;
    }

    // Mise à jour de la zone de sélection
    if (select_object)
    {
      selection.x = MIN(x, origin.x);
      selection.y = MIN(y, origin.y);
      selection.width = selection.x + CV_IABS(x - origin.x);
      selection.height = selection.y + CV_IABS(y-origin.y);

      selection.x = MAX(selection.x, 0);
      selection.y = MAX(selection.y, 0);
      selection.width = MIN(selection.width, image->width);
      selection.height = MIN(selection.height, image->height);
      selection.width -= selection.x;
      selection.height -= selection.y;
    }

    // Gestion des évènements souris
    switch(event)
    {
    case CV_EVENT_LBUTTONDOWN:
      // On commence une phase de sélection
      origin = cvPoint(x,y);
      selection = cvRect(x,y,0,0);
      select_object = 1;
      break;
    case CV_EVENT_LBUTTONUP:
      // On termine une phase de sélection
      select_object = 0;
      if (selection.width > 0 && selection.height > 0)
      {
	// action à effectuer après la sélection
	// On ne fait rien, l'action est exécutée par la boucle principale
      }
      break;
    }
  }

  return;
}


// Fonction principale
int main(int argc, char** argv)
{
  IplImage *frame = NULL;
  int i, j, nbrObjets, quit=0, imgsuiv=0;
  CvRect objets[10];
  char c, file[51];
  FILE *fichier;

  printf("Programme de generation d'une liste d'images positives\n");
  printf("pour la generation d'une base de donnees de reconnaissance d'objet\n\n");
  printf("Utilisation : passer la liste d'images a traiter en argument.\n\n");
  printf("Commandes :\n");
  printf("\tq : quitter\n");
  printf("\t<ESPACE> : valider la selection courante\n");
  printf("\t<ESC> : annuler les selections de l'image courante\n");
  printf("\t<ENTREE> : valider les selections et passer a l'image suivante\n\n\n");

  if (argc < 2)
  {
    printf("Pas assez d'arguments\n");
    return 1;
  }

  printf("Nom du fichier a ecrire (le fichier sera ecrase) : ");
  scanf("%s", file);
  printf("\n\n\n");

  if ( (fichier = fopen(file, "w+")) == NULL)
  {
    printf("Impossible d'ouvrir le fichier destination !\n");
    return 1;
  }

  cvNamedWindow("Image courante", 0);
  cvSetMouseCallback("Image courante", on_mouse, 0);

  for (i=1; i < argc && !quit; i++)
  {
    printf("Image : %s\n", argv[i]);
    printf("===================================================\n\n");

    if ( (image = cvLoadImage(argv[i], 1)) == NULL )
    {
      printf("\tImpossible de charger l'image...\n\n\n\n");
      continue;
    }
    cvShowImage("Image courante", image);

    // On alloue les buffers si ce n'est pa déjà fait
    if (frame == NULL)
    {
      frame = cvCreateImage(cvGetSize(image), 8, 3);
      frame->origin = image->origin;
    }

    // il n'y a pas d'objet 
    nbrObjets = 0;
    // On efface une éventuelle sélection
    selection.width = 0;
    selection.height = 0;
    // On boucle jusqu'à devoir passer à l'image suivante
    imgsuiv=0;

    while (!imgsuiv)
    {
      // On fait le rendu de l'image en ajoutant la zone de selection
      // On travaille sur une copie de l'image
      cvCopy(image, frame, 0);
      // On rend la zone de selection
      if( selection.width > 0 && selection.height > 0 )
      {
	cvSetImageROI(frame, selection);
        cvXorS(frame, cvScalarAll(255), frame, 0);
        cvResetImageROI(frame);
      }
      
      // On affiche l'image
      cvShowImage("Image courante", frame);

      // On répond aux entrées utilisateur
      c = (char)cvWaitKey(10);

      switch (c)
      {
        case 'q' :
	  // On passe à l'image suivante
	  imgsuiv=1;
	  // En fait, non, on quitte
	  quit=1;
	  break;

        case 27 : // <ESC>
	  nbrObjets = 0;
	  break;

        case 10 : // <ENTREE>
	  fprintf(fichier, "%s", argv[i]);
	  if (nbrObjets > 0)
	  {
	    fprintf(fichier, " %d", nbrObjets);
	    for (j = 0; j < nbrObjets; j++)
	      fprintf(fichier, " %d %d %d %d",
		      objets[j].x, objets[j].y, objets[j].width, objets[j].height);
	  }
	  fprintf(fichier, "\n");
	  // On passe à l'image suivante
	  imgsuiv=1;
	  break;
      
        case 32 : // <ESPACE>
	  if (selection.width > 0 && selection.height > 0)
	  {
	    objets[nbrObjets] = selection;
	    nbrObjets++;
  	    printf("\t%d. rect x=%d\ty=%d\t\twidth=%d\theight=%d\n",
		   nbrObjets, selection.x, selection.y, selection.width, selection.height);
	  }
	  // On efface la selection courante de l'ecran
	  selection.width = 0;
	  selection.height = 0;
	  break;
      }
    }
    printf("\n\n\n");
  }
 
  fclose(fichier);
  
  cvDestroyWindow("Image courante");

  return 0;
}

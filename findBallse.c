#include "cv.h"
#include "highgui.h"
#include <math.h>
#include <stdio.h>

#define N_COULEURS 3
#define min(a,b) (((a) < (b)) ? (a) : (b))

typedef struct
{
  int centH, radH, minS, maxS, minV, maxV;
  int threshold;
  int minCont, maxCont;
} params_couleur;

params_couleur balles[N_COULEURS];
CvScalar couleurs[N_COULEURS];

// Load the source image. HighGUI use.
IplImage *image01 = 0, *image02 = 0, *image03 = 0, *imCont = 0, *imFill = 0, *imHSV = 0;

int affichage = 0;
int slider_pos;

void process_image();
int in_radius(int val, int center, int radius, int max);

int main( int argc, char** argv )
{
  int source, c, quit=0;
  CvCapture *capture = NULL;
  char *fileName;
  FILE *fichier;

  // Traitement des paramètres de ligne de commande
  if (argc < 2)
  {
    printf("argv[1] : chemin du fichier de paramètres\n");
    printf("argv[2] : (optionnel) si différent de 0, affiche une fenêtre\n");
    printf("argv[3] : (optionnel) numéro de la webcam à utiliser\n");
    return 1;
  }
  fileName = argv[1];
  affichage = argc >= 3 ? atoi(argv[2]) : 0;
  source = argc >= 4 ? atoi(argv[3]) : 0;

  // Ouvre la webcam
  capture = cvCaptureFromCAM(source);

  // Create window
  if (affichage != 0)
    cvNamedWindow("Result", 1);

  // Récupère une image de la webcam
  image01 = cvQueryFrame(capture);

  // Création de l'image en niveaux de gris à la taile de l'image prise par la webcam
  image03 = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 1);

  // Même chose avec l'image pour conversion
  imHSV = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 3);

  while (!quit)
  {
    // Récupère une image de la webcam
    image01 = cvQueryFrame(capture);

    process_image();

    // Show the image
    if (affichage != 0)
      cvShowImage("Result", imCont);

    // Wait for a key stroke; the same function arranges events processing                
    c = (char)cvWaitKey(10);

    switch (c)
    {
      case 'q':
	quit = 1;
	break;
    }

    cvReleaseImage(&imCont);
    cvReleaseImage(&imFill);
  }


  // On release la mémoire
  cvReleaseCapture(&capture);

  cvReleaseImage(&image02);
  cvReleaseImage(&image03);
  cvReleaseImage(&imHSV);
  cvReleaseImage(&imFill);
  cvReleaseImage(&imCont);

  if (affichage != 0)
    cvDestroyWindow("Result");

  return 0;
}

// This function the balls,
void process_image()
{
    CvMemStorage* stor;
    CvSeq* cont;
    CvBox2D32f* box;
    CvPoint* PointArray;
    CvPoint2D32f* PointArray2D32f;
    int i, j, meanRad;
    
    // Changement d'espace de couleur
    cvCvtColor(image01, imHSV, CV_BGR2HSV);

    // Génération de l'image avec les résultats
    if (affichage != 0)
    {
      imCont = cvCloneImage(image01);
      cvZero(imCont);
      // On fixe la ROI de l'image pour ne pas dessiner à l'extérieur
      imCont->roi = (_IplROI*)malloc(sizeof(IplROI));
      imCont->roi->coi = 0;
      imCont->roi->xOffset = 0;
      imCont->roi->yOffset = 0;
      imCont->roi->width = imCont->width;
      imCont->roi->height = imCont->height;
    }

    // On fait le traitement pour chaque couleur de balle
    for (j=0; j < N_COULEURS; j++)
    {
      imFill = cvCloneImage(image01);

      for (i=0 ; i < imFill->width*imFill->height; i++)
      {
        if (in_radius(imHSV->imageData[3*i], balles[j].centH, balles[j].radH, 180) &&
	    (uchar)imHSV->imageData[3*i+1] >= balles[j].minS && (uchar)imHSV->imageData[3*i+1] <= balles[j].maxS &&
	    (uchar)imHSV->imageData[3*i+2] >= balles[j].minV && (uchar)imHSV->imageData[3*i+2] <= balles[j].maxV )
        {
	  // On laisse la couleur du pixel
        }
        else
        {
	  // sinon on l'efface
	  imFill->imageData[3*i  ] = 0;
	  imFill->imageData[3*i+1] = 0;
	  imFill->imageData[3*i+2] = 0;
        }
      }

      // Conversion en niveaux de gris de l'image filtrée
      cvCvtColor(imFill, image03, CV_BGR2GRAY);

      // Create the destination images
      image02 = cvCloneImage( image03 );

      // Create dynamic memory storage and sequence. 
      stor = cvCreateMemStorage(0);
      cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint) , stor);
    
      // Threshold the source image. This needful for cvFindContours().
      cvThreshold( image03, image02, slider_pos, 255, CV_THRESH_BINARY );
    
      // Find all contours.
      cvFindContours( image02, stor, &cont, sizeof(CvContour), 
                      CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
    
      // Clear image. IPL use.
      cvZero(image02);
    
      // This cycle draw all contours and approximate it by ellipses.
      for(;cont;cont = cont->h_next)
      {   
          int i; // Indicator of cycle.
          int count = cont->total; // This is number point in contour
          CvPoint center;
          CvSize size;
        
          // Number point must be more than or equal to 6 (for cvFitEllipse_32f).        
          if( count < 6 )
              continue;
        
          // Alloc memory for contour point set.    
          PointArray = (CvPoint*)malloc( count*sizeof(CvPoint) );
          PointArray2D32f= (CvPoint2D32f*)malloc( count*sizeof(CvPoint2D32f) );
        
          // Alloc memory for ellipse data.
          box = (CvBox2D32f*)malloc(sizeof(CvBox2D32f));
        
          // Get contour point set.
          cvCvtSeqToArray(cont, PointArray, CV_WHOLE_SEQ);
        
          // Convert CvPoint set to CvBox2D32f set.
          for(i=0; i<count; i++)
          {
              PointArray2D32f[i].x = (float)PointArray[i].x;
              PointArray2D32f[i].y = (float)PointArray[i].y;
          }
        
          // Fits ellipse to current contour.
          cvFitEllipse(PointArray2D32f, count, box);

          // Convert ellipse data from float to integer representation.
          center.x = cvRound(box->center.x);
          center.y = cvRound(box->center.y);
          size.width = cvRound(box->size.width*0.5);
          size.height = cvRound(box->size.height*0.5);
          box->angle = -box->angle;
        
	  // On ne dessine le contour et l'ellipse que si son rayon est dans les bornes
	  meanRad = (size.width + size.height) / 2;

	  if (meanRad >= balles[j].minCont && meanRad <= balles[j].maxCont)
	  {
            // Draw current contour.
            cvDrawContours(imCont,cont,CV_RGB(255,255,255),CV_RGB(255,255,255),0,1,8,cvPoint(0,0));
     
	    // Draw ellipse.
	    cvEllipse(imCont, center, size,
                      box->angle, 0, 360,
                      CV_RGB(255,0,0), 1, CV_AA, 0);
	  }
        
          // Free memory.
	  free(imCont->roi); imCont->roi = NULL;
          free(PointArray);
          free(PointArray2D32f);
          free(box);

      }
   
      // On libère la mémoire
      cvReleaseMemStorage(&stor);
      cvReleaseImage(&image02);
      cvReleaseImage(&imFill);
    }
}

int in_radius(int val, int center, int radius, int max)
{
  int d;
  d = abs(val - center);
  return min(d,max - d) <= radius;
}

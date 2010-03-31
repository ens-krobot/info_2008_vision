#include "cv.h"
#include "highgui.h"
#include <math.h>
#include <stdio.h>

int slider_pos = 70, minCont = 20, maxCont = 100;
int cenH=90, radH=180, minS=0, maxS=255, minV=0, maxV=255;

// Load the source image. HighGUI use.
IplImage *image01 = 0, *image02 = 0, *image03 = 0;
IplImage *imH = 0, *imS = 0, *imV = 0, *imHSV = 0, *imFil = 0, *imCont = 0;

void process_image(int h);
int in_radius(int val, int center, int radius, int max);

int main( int argc, char** argv )
{
  int ncams, source, c, quit=0;
  CvCapture *capture = NULL;

  // Ouvre la webcam
  ncams = (argc >= 2 ? atoi(argv[1]) : 1);
  source = (argc >= 3 ? atoi(argv[2]) : 0);
  capture = cvCaptureFromCAM(source);
   
  // Create windows.
  cvNamedWindow("H",1);
  cvNamedWindow("S",1);
  cvNamedWindow("V",1);
  cvNamedWindow("Source", 1);
  cvNamedWindow("Filtered",1);
  cvNamedWindow("Result", 1);

  // Create toolbars. HighGUI use.
  cvCreateTrackbar( "Threshold", "Result", &slider_pos, 255, NULL);
  cvCreateTrackbar( "Min radius", "Result", &minCont, 255, NULL);
  cvCreateTrackbar( "Max radius", "Result", &maxCont, 255, NULL);
 
  cvCreateTrackbar("Center", "H", &cenH, 180, NULL);
  cvCreateTrackbar("Radius", "H", &radH, 180, NULL);
  cvCreateTrackbar("Min", "S", &minS, 255, NULL);
  cvCreateTrackbar("Max", "S", &maxS, 255, NULL);
  cvCreateTrackbar("Min", "V", &minV, 255, NULL);
  cvCreateTrackbar("Max", "V", &maxV, 255, NULL);

  // Récupère une image de la webcam
  image01 = cvQueryFrame(capture);

  // Création de l'image03 à la taile de l'image prise par la webcam
  image03 = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 1);
  imH = cvCloneImage(image03); cvZero(imH);
  imS = cvCloneImage(image03); cvZero(imS);
  imV = cvCloneImage(image03); cvZero(imV);

  // Même chose avec image05
  imHSV = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 3);

  while (!quit)
  {
    // Récupère une image de la webcam
    image01 = cvQueryFrame(capture);

    process_image(0);

    // Show the images.
    cvShowImage("H", imH);
    cvShowImage("S", imS);
    cvShowImage("V", imV);
    cvShowImage("Source", image01);
    cvShowImage("Filtered", imFil);
    cvShowImage("Result", imCont);

    // Wait for a key stroke; the same function arranges events processing                
    c = (char)cvWaitKey(10);

    switch (c)
    {
      case 'c':
	source = (source+1) % ncams;
	cvReleaseCapture(&capture);
	capture = cvCaptureFromCAM(source);
	cvReleaseImage(&image03);
	image01 = cvQueryFrame(capture);
	image03 = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 1);
	imHSV = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 3);
        imH = cvCloneImage(image03);
	imS = cvCloneImage(image03);
	imV = cvCloneImage(image03);
	break;
      case 'p':
	printf("\nH (center,radius) : (%d,%d)\n", cenH, radH);
	printf("S (min,max) : (%d,%d)\n", minS, maxS);
	printf("V (min,max) : (%d,%d)\n", minV, maxV);
	printf("Threshold : %d\n", slider_pos);
	printf("Sort radius (min,max) : (%d,%d)\n", minCont, maxCont);
	break;
      case 'q':
	quit = 1;
	break;
    }

    cvReleaseImage(&imCont);
    cvReleaseImage(&imFil);
  }


  // On release la mémoire
  cvReleaseCapture(&capture);

  cvReleaseImage(&image02);
  cvReleaseImage(&image03);
  cvReleaseImage(&imH);
  cvReleaseImage(&imS);
  cvReleaseImage(&imV);
  cvReleaseImage(&imHSV);
  cvReleaseImage(&imFil);
  cvReleaseImage(&imCont);

  cvDestroyWindow("H");
  cvDestroyWindow("S");
  cvDestroyWindow("V");
  cvDestroyWindow("Source");
  cvDestroyWindow("Result");

  return 0;
}

// Define trackbar callback functon. This function find contours,
// draw it and approximate it by ellipses.
void process_image(int h)
{
    CvMemStorage* stor;
    CvSeq* cont;
    CvBox2D32f* box;
    CvPoint* PointArray;
    CvPoint2D32f* PointArray2D32f;
    int i, meanRad;
    
    // Filtre de couleur
    cvCvtColor(image01, imHSV, CV_BGR2HSV);    
    imFil = cvCloneImage(image01);
    imCont = cvCloneImage(image01);

    // Génération des images H,S,V
    for (i=0 ; i < imHSV->width*imHSV->height; i++)
    {
      imH->imageData[i] = in_radius(imHSV->imageData[3*i], cenH, radH, 180) ? 255 : 0;
      imS->imageData[i] = (((uchar)imHSV->imageData[3*i+1] >= minS) && ((uchar)imHSV->imageData[3*i+1] <= maxS)) ? 255 : 0;
      imV->imageData[i] = (((uchar)imHSV->imageData[3*i+2] >= minV) && ((uchar)imHSV->imageData[3*i+2] <= maxV)) ? 255 : 0;
    }

    for (i=0 ; i < imFil->width*imFil->height; i++)
    {
      if (in_radius(imHSV->imageData[3*i], cenH, radH, 180) &&
	  (uchar)imHSV->imageData[3*i+1] >= minS && (uchar)imHSV->imageData[3*i+1] <= maxS &&
	  (uchar)imHSV->imageData[3*i+2] >= minV && (uchar)imHSV->imageData[3*i+2] <= maxV )
      {
	// On laisse la couleur du pixel
      }
      else
      {
	// sinon on l'efface
	imFil->imageData[3*i  ] = 0;
	imFil->imageData[3*i+1] = 0;
	imFil->imageData[3*i+2] = 0;
      }
    }

    // Conversion en niveaux de gris de l'image filtrée
    cvCvtColor(imFil, image03, CV_BGR2GRAY);

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
    
    // Clear images. IPL use.
    cvZero(image02);
    cvZero(imCont);

    // On fixe la ROI de l'image pour ne pas dessiner à l'extérieur
    imCont->roi = (_IplROI*)malloc(sizeof(IplROI));
    imCont->roi->coi = 0;
    imCont->roi->xOffset = 0;
    imCont->roi->yOffset = 0;
    imCont->roi->width = imCont->width;
    imCont->roi->height = imCont->height;
    
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

	if (meanRad >= minCont && meanRad <= maxCont)
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
}

#define min(a,b) (((a) < (b)) ? (a) : (b))

int in_radius(int val, int center, int radius, int max)
{
  int d;
  d = abs(val - center);
  return min(d,max - d) <= radius;
}

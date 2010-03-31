/********************************************************************************
*
*
*  This program is demonstration for ellipse fitting. Program finds 
*  contours and approximate it by ellipses.
*
*  Trackbar specify threshold parametr.
*
*  White lines is contours. Red lines is fitting ellipses.
*
*
*  Autor:  Denis Burenkov.
*
*
*
********************************************************************************/
#ifdef _CH_
#pragma package <opencv>
#endif

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#endif

int slider_pos = 70;

// Load the source image. HighGUI use.
IplImage *image01 = 0, *image02 = 0, *image03 = 0, *image04 = 0, *image05 = 0, *image06 = 0;

void process_image(int h);

int main( int argc, char** argv )
{
  int ncams, source, c, quit=0, i;
  CvCapture *capture = NULL;

  // Ouvre la webcam
  ncams = (argc >= 2 ? atoi(argv[1]) : 1);
  source = (argc >= 3 ? atoi(argv[2]) : 0);
  capture = cvCaptureFromCAM(source);
   
  // Create windows.
  cvNamedWindow("HSV",1);
  cvNamedWindow("Source", 1);
  cvNamedWindow("Result", 1);

  // Create toolbars. HighGUI use.
  cvCreateTrackbar( "Threshold", "Result", &slider_pos, 255, process_image );
 
  // Récupère une image de la webcam
  image01 = cvQueryFrame(capture);

  // Création de l'image03 à la taile de l'image prise par la webcam
  image03 = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 1);

  // Même chose avec image05
  image05 = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 3);

  while (!quit)
  {
    // Récupère une image de la webcam
    image01 = cvQueryFrame(capture);

    // Conversion en niveaux de gris
    cvCvtColor(image01, image03, CV_BGR2GRAY);

    // Filtre de couleur
    cvCvtColor(image01, image05, CV_BGR2HSV);    
    cvReleaseImage(&image06);
    image06 = cvCloneImage(image03);
    
    for (i=0 ; i < image06->width*image06->height; i++)
    {
      image06->imageData[i] = ((image05->imageData[3*i] > 90 && image05->imageData[3*i] < 110 && image05->imageData[3*i+1] > 20) ? 255 : 0);
    }
      

    cvReleaseImage(&image02);
    cvReleaseImage(&image04);

    // Create the destination images
    image02 = cvCloneImage( image03 );
    image04 = cvCloneImage( image03 );  

    // Show the image.
    cvShowImage("HSV", image06);
    cvShowImage("Source", image01);

    process_image(0);

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
	image05 = cvCreateImage(cvSize(image01->width,image01->height), IPL_DEPTH_8U, 3);
	break;
      case 'q':
	quit = 1;
	break;
    }

  }


  // On release la mémoire
  cvReleaseCapture(&capture);

  cvReleaseImage(&image01);
  cvReleaseImage(&image02);
  cvReleaseImage(&image03);
  cvReleaseImage(&image04);
  cvReleaseImage(&image05);

  cvDestroyWindow("HSV");
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
    cvZero(image04);
    
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
        
        // Draw current contour.
        cvDrawContours(image04,cont,CV_RGB(255,255,255),CV_RGB(255,255,255),0,1,8,cvPoint(0,0));
        
        // Convert ellipse data from float to integer representation.
        center.x = cvRound(box->center.x);
        center.y = cvRound(box->center.y);
        size.width = cvRound(box->size.width*0.5);
        size.height = cvRound(box->size.height*0.5);
        box->angle = -box->angle;
     
        // Draw ellipse.
        /*cvEllipse(image04, center, size,
                  box->angle, 0, 360,
                  CV_RGB(0,0,255), 1, CV_AA, 0);*/
        
        // Free memory.          
        free(PointArray);
        free(PointArray2D32f);
        free(box);

    }
    
    // Show image. HighGUI use.
    cvShowImage( "Result", image04 );

    cvReleaseMemStorage(&stor);
}

#ifdef _EiC
main(1,"fitellipse.c");
#endif

#include <opencv2/opencv.hpp>
#include <ctime>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <string>
using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{

    VideoCapture cap("data/test_video.mp4");
    cap.set(CV_CAP_PROP_FRAME_WIDTH,480);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,270);
    namedWindow("vision",CV_WINDOW_AUTOSIZE);
    namedWindow("red",CV_WINDOW_AUTOSIZE);
//    namedWindow("blue",CV_WINDOW_AUTOSIZE);
//    namedWindow("green",CV_WINDOW_AUTOSIZE);
    namedWindow("yellow",CV_WINDOW_AUTOSIZE);

    while(1){
    	Mat frame,im,hsv,mask1,mask2,bf;
    	cap>>frame;
    	resize(frame,im,Size(480,270));
    	bilateralFilter(im,bf,9,75,75);
    	cvtColor(bf,hsv,CV_BGR2HSV);

	    Mat redMask,blueMask,greenMask,yellowMask;
	    inRange(hsv,Vec3b(110,80,80),Vec3b(130,255,255),blueMask);
	    inRange(hsv,Vec3b(50,80,80),Vec3b(70,255,255),greenMask);
	    inRange(hsv,Vec3b(20,80,80),Vec3b(40,255,255),yellowMask);
	    Mat tempMask1,tempMask2;
	    inRange(hsv,Vec3b(0,120,120),Vec3b(10,255,255),tempMask1);
	    inRange(hsv,Vec3b(170,120,120),Vec3b(180,255,255),tempMask2);

	    bitwise_or(tempMask1,tempMask2,redMask);

	    Mat redf,bluef,greenf,yellowf;

    	bitwise_and(im,im,redf,redMask);

//    	bitwise_and(im,im,bluef,blueMask);

//    	bitwise_and(im,im,greenf,greenMask);

    	bitwise_and(im,im,yellowf,yellowMask);
    	//red contours
    	vector<vector<Point> > redContours;
    	vector<Vec4i> redHierarchy;
    	Mat redf8;
    	cvtColor(redf,redf8,CV_RGB2GRAY);
  		findContours( redf8, redContours, redHierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  		for( int i = 0; i< redContours.size(); i++ ){
	    //	       drawContours( im, redContours, i, color, 1, 8, redHierarchy, 0, Point() );
	       RotatedRect redRect=minAreaRect(redContours[i]);

			Point2f vertices[4];
			redRect.points(vertices);
			for (int j = 0; j < 4; j++)
			    line(im, vertices[j], vertices[(j+1)%4], Scalar(0,255,0));
	    }

	    //yellow contours
    	vector<vector<Point> > yellowContours;
    	vector<Vec4i> yellowHierarchy;
    	Mat yellowf8;
    	cvtColor(yellowf,yellowf8,CV_RGB2GRAY);
  		findContours( yellowf8, yellowContours, yellowHierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  		for( int i = 0; i< yellowContours.size(); i++ )
	    {
	       RotatedRect yellowRect=minAreaRect(yellowContours[i]);
			Point2f vertices[4];
			yellowRect.points(vertices);
			for (int j = 0; j < 4; j++)
			    line(im, vertices[j], vertices[(j+1)%4], Scalar(255,0,0));
	    }


/*    	Mat erodeRed,erodeBlue,erodeGreen,erodeYellow;
    	int erosion_size=1;
    	Mat element = getStructuringElement( MORPH_ELLIPSE,
                                       Size( 2*erosion_size + 1, 2*erosion_size+1 ) );
    	erode(redf,erodeRed,element);
    	erode(bluef,erodeBlue,element);
    	erode(greenf,erodeGreen,element);
    	erode(yellowf,erodeYellow,element);
        imshow("vision", im);
        imshow("red",erodeRed);
        imshow("blue", erodeBlue);
        imshow("green",erodeGreen);
        imshow("yellow",erodeYellow);*/

        imshow("vision", im);
        imshow("red",redf);
//        imshow("blue", bluef);
//        imshow("green",greenf);
        imshow("yellow",yellowf);
        waitKey(1);

    }
	return 0;
}
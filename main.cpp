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

    //read xml file
    FileStorage fs("data/sterer_calibration_data.xml");
    if(!fs.isOpened()){
        cout<<"can't find stereo calibration file"<<endl;
        return -1;
    }
    Mat cameraMatrix1,distCoeffs1,
        cameraMatrix2,distCoeffs2,R,T,E,F;
    fs["Camera_Matrix_1"]>>cameraMatrix1;
    fs["Distortion_Coefficients_1"]>>distCoeffs1;
    fs["Camera_Matrix_2"]>>cameraMatrix2;
    fs["Distortion_Coefficients_2"]>>distCoeffs2;
    fs["R"]>>R;
    fs["T"]>>T;
    fs["E"]>>E;
    fs["F"]>>F;

    VideoCapture cap1(0);
    VideoCapture cap2(1);
    if ( !cap1.isOpened() )
    {
         cout << "Cannot open camera 1" << endl;
         return -1;
    }

    if ( !cap2.isOpened() )
    {
         cout << "Cannot open camera 2" << endl;
         return -1;
    }


    cap1.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap1.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    cap1.set(CV_CAP_PROP_FPS,60);
    cap2.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap2.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    cap2.set(CV_CAP_PROP_FPS,60);
/*
    cap1.set(CV_CAP_PROP_FRAME_WIDTH,320);
    cap1.set(CV_CAP_PROP_FRAME_HEIGHT,240);
    cap1.set(CV_CAP_PROP_FPS,60);
    cap2.set(CV_CAP_PROP_FRAME_WIDTH,320);
    cap2.set(CV_CAP_PROP_FRAME_HEIGHT,240);
    cap2.set(CV_CAP_PROP_FPS,60);
*/

    namedWindow("cam1",CV_WINDOW_AUTOSIZE);
    namedWindow("cam2",CV_WINDOW_AUTOSIZE);
    moveWindow("cam1",320,0);
    moveWindow("cam2",640,0);
    moveWindow("stereo",960,0);
    while(1)
    {
        Mat frame1,frame2;
        Mat stereoFrame;
        if(cap1.grab())
            cap1.retrieve(frame1);
        if(cap2.grab())
            cap2.retrieve(frame2);
        hconcat(frame1,frame2,stereoFrame);
        //imshow("cam1", frame1);
        //imshow("cam2", frame2);
        imshow("stereo", stereoFrame);
        char key=waitKey(10);
        switch(key){
            case 27: //esc
                cout<<"esc pressed, exit"<<endl;
                return 0;
            break;
            case ' ':
                time_t ct=time(NULL);
                char* currentTime=ctime(&ct);
                cout<<"taking snapshot! frame:"<<currentTime<<endl;
                imwrite(string("./images/cam1_")+currentTime+".jpg",frame1);
                imwrite(string("./images/cam2_")+currentTime+".jpg",frame2);
            break;

        }
    }

    return 0;

}

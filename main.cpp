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
    bool recordMode=false,stopRecording=false,recordingStarted=false;
    int image_width,image_height,fps,rectify;
    string input1,input2;
    VideoWriter* videoRecorder=NULL;
    //read xml file
    FileStorage fsCalibration("data/stereo_calibration_data.xml",FileStorage::READ);
    if(!fsCalibration.isOpened()){
        cout<<"can't find stereo calibration file"<<endl;
        return -1;
    }
    Mat cameraMatrix1,distCoeffs1,
        cameraMatrix2,distCoeffs2,R,T,E,F;
    fsCalibration["Camera_Matrix_1"]>>cameraMatrix1;
    fsCalibration["Distortion_Coefficients_1"]>>distCoeffs1;
    fsCalibration["Camera_Matrix_2"]>>cameraMatrix2;
    fsCalibration["Distortion_Coefficients_2"]>>distCoeffs2;
    fsCalibration["R"]>>R;
    fsCalibration["T"]>>T;
    fsCalibration["E"]>>E;
    fsCalibration["F"]>>F;
    FileStorage fsConfig("data/vision_config.xml",FileStorage::READ);
    fsConfig["image_Width"]>>image_width;
    fsConfig["image_Height"]>>image_height;
    fsConfig["Input1"]>>input1;
    fsConfig["Input2"]>>input2;
    fsConfig["fps"]>>fps;
    fsConfig["rectify"]>>rectify;

    VideoCapture cap1(stoi(input1));
    VideoCapture cap2(stoi(input2));
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


    Mat mapOne1, mapOne2, mapTwo1, mapTwo2;
    Size imageSize=Size(image_width,image_height);
    initUndistortRectifyMap(cameraMatrix1, distCoeffs1, Mat(),
        getOptimalNewCameraMatrix(cameraMatrix1, distCoeffs1, imageSize, 1, imageSize, 0),
        imageSize, CV_16SC2, mapOne1, mapOne2);
    initUndistortRectifyMap(cameraMatrix2, distCoeffs2, Mat(),
        getOptimalNewCameraMatrix(cameraMatrix2, distCoeffs2, imageSize, 1, imageSize, 0),
        imageSize, CV_16SC2, mapTwo1, mapTwo2);

    StereoSGBM sgbm;
    sgbm.SADWindowSize = 5;
    sgbm.numberOfDisparities = 192;
    sgbm.preFilterCap = 4;
    sgbm.minDisparity = -64;
    sgbm.uniquenessRatio = 1;
    sgbm.speckleWindowSize = 150;
    sgbm.speckleRange = 2;
    sgbm.disp12MaxDiff = 10;
    sgbm.fullDP = false;
    sgbm.P1 = 600;
    sgbm.P2 = 240;

    cap1.set(CV_CAP_PROP_FRAME_WIDTH,image_width);
    cap1.set(CV_CAP_PROP_FRAME_HEIGHT,image_height);
    cap1.set(CV_CAP_PROP_FPS,fps);
    cap2.set(CV_CAP_PROP_FRAME_WIDTH,image_width);
    cap2.set(CV_CAP_PROP_FRAME_HEIGHT,image_height);
    cap2.set(CV_CAP_PROP_FPS,fps);

    namedWindow("stereo",CV_WINDOW_AUTOSIZE);
    moveWindow("stereo",960,0);
    if(rectify){
        namedWindow("rectify",CV_WINDOW_AUTOSIZE);
        moveWindow("rectify",960,0);
    }
    namedWindow("depth map",CV_WINDOW_AUTOSIZE);
    moveWindow("depth map",960,0);
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


        Mat g1,g2,depthMap,depthMapNormalized;
        if(recordMode){
            if(!recordingStarted){
                if(!videoRecorder){
                    time_t ct=time(NULL);
                    char* currentTime=ctime(&ct);
                    videoRecorder=new VideoWriter (string("data/videos/")+currentTime+".avi",
                     CV_FOURCC('D','I','V','X'), fps, Size(image_width*2,image_height), true);
                     cout<<"started recording"<<endl;
                }
                recordingStarted=true;
            }else if(stopRecording){
                delete videoRecorder;
                cout<<"stopped recording"<<endl;
                videoRecorder=NULL;
                recordMode=false;

            }else{
                videoRecorder->write(stereoFrame);
            }

        }else{
                if(rectify){
                    Mat rframe1,rframe2,rstereoFrame;
                    remap(frame1, rframe1, mapOne1, mapOne2, INTER_LINEAR);
                    remap(frame2, rframe2, mapTwo1, mapTwo2, INTER_LINEAR);
                    hconcat(rframe1,rframe2,rstereoFrame);
                    imshow("rectify", rstereoFrame);

                    cvtColor(rframe1, g1, CV_BGR2GRAY);
                    cvtColor(rframe2, g2, CV_BGR2GRAY);
        //            cvtColor(frame1, g1, CV_BGR2GRAY);
        //            cvtColor(frame2, g2, CV_BGR2GRAY);
                }else{
                    cvtColor(frame1, g1, CV_BGR2GRAY);
                    cvtColor(frame2, g2, CV_BGR2GRAY);
                }
                sgbm(g1, g2, depthMap);
                normalize(depthMap, depthMapNormalized, 0, 255, CV_MINMAX, CV_8U);
                imshow("depth map",depthMapNormalized);
        }

        char key=waitKey(10);
        switch(key){
            case 27: //esc
                cout<<"esc pressed, exit"<<endl;
                return 0;
            break;
            case ' ':{
                time_t ct=time(NULL);
                char* currentTime=ctime(&ct);
                cout<<"taking snapshot! frame:"<<currentTime<<endl;
                imwrite(string("./images/cam1_")+currentTime+".jpg",frame1);
                imwrite(string("./images/cam2_")+currentTime+".jpg",frame2);
            }
            break;
            case 'r':
                 recordMode=true;
                 stopRecording=false;
                 recordingStarted=false;
            break;
            case 's':
                stopRecording=true;
            break;

        }
    }

    return 0;

}

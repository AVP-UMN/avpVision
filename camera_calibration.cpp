#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

using namespace cv;
using namespace std;

class Settings
{
public:
    Settings() : goodInput(false) {}
    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
    enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

    void write(FileStorage& fs) const                        //Write serialization for this class
    {
        fs << "{" << "BoardSize_Width"  << boardSize.width
                  << "BoardSize_Height" << boardSize.height
                  << "Square_Size"         << squareSize
                  << "Calibrate_Pattern" << patternToUse
                  << "Calibrate_NrOfFrameToUse" << nrFrames
                  << "Calibrate_FixAspectRatio" << aspectRatio
                  << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
                  << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

                  << "Write_DetectedFeaturePoints" << bwritePoints
                  << "Write_extrinsicParameters"   << bwriteExtrinsics
                  << "Write_outputFileName"  << outputFileName

                  << "Show_UndistortedImage" << showUndistorsed

                  << "Input_FlipAroundHorizontalAxis" << flipVertical
                  << "Input_Delay" << delay
                  << "Input1" << input1
                  << "Input2" << input2
           << "}";
    }
    void read(const FileNode& node)                          //Read serialization for this class
    {
        node["BoardSize_Width" ] >> boardSize.width;
        node["BoardSize_Height"] >> boardSize.height;
        node["Calibrate_Pattern"] >> patternToUse;
        node["Square_Size"]  >> squareSize;
        node["Calibrate_NrOfFrameToUse"] >> nrFrames;
        node["Calibrate_FixAspectRatio"] >> aspectRatio;
        node["Write_DetectedFeaturePoints"] >> bwritePoints;
        node["Write_extrinsicParameters"] >> bwriteExtrinsics;
        node["Write_outputFileName"] >> outputFileName;
        node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
        node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
        node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
        node["Show_UndistortedImage"] >> showUndistorsed;
        node["Input1"] >> input1;
        node["Input2"] >> input2;
        node["Input_Delay"] >> delay;
        interprate();
    }
    void interprate()
    {
        goodInput = true;
        if (boardSize.width <= 0 || boardSize.height <= 0)
        {
            cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
            goodInput = false;
        }
        if (squareSize <= 10e-6)
        {
            cerr << "Invalid square size " << squareSize << endl;
            goodInput = false;
        }
        if (nrFrames <= 0)
        {
            cerr << "Invalid number of frames " << nrFrames << endl;
            goodInput = false;
        }

        if (input1.empty()||input2.empty())      // Check for valid input
                inputType = INVALID;
        else
        {
            if (input1[0] >= '0' && input2[0] <= '9')
            {
                stringstream ss(input1);
                ss >> cameraID1;
                stringstream sss(input2);
                sss >> cameraID2;
                inputType = CAMERA;
            }
            else
            {
                if (readStringList(input1, imageList1)&&readStringList(input2, imageList2)&&(imageList1.size()==imageList2.size()))
                    {
                        inputType = IMAGE_LIST;
                        nrFrames = (nrFrames < (int)imageList1.size()) ? nrFrames : (int)imageList1.size();
                    }
                else
                    inputType = VIDEO_FILE;
            }
            if (inputType == CAMERA){
                inputCapture1.open(cameraID1);
                inputCapture2.open(cameraID2);
            }
            if (inputType == VIDEO_FILE){
                inputCapture1.open(input1);
                inputCapture2.open(input2);
            }
            if (inputType != IMAGE_LIST && !inputCapture1.isOpened() && !inputCapture2.isOpened())
                    inputType = INVALID;
        }
        if (inputType == INVALID)
        {
            cerr << " Inexistent input! " <<endl;
            goodInput = false;
        }

        flag = 0;
        if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
        if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;


        calibrationPattern = NOT_EXISTING;
        if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
        if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
        if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
        if (calibrationPattern == NOT_EXISTING)
            {
                cerr << " Inexistent camera calibration mode: " << patternToUse << endl;
                goodInput = false;
            }
        atImageList = 0;

    }
    Mat nextImageFrom1()
    {
        Mat result;
        if( inputCapture1.isOpened() )
        {
            Mat view0;
            inputCapture1 >> view0;
            view0.copyTo(result);
        }
        else if( atImageList < (int)imageList1.size() )
            result = imread(imageList1[atImageList], CV_LOAD_IMAGE_COLOR);

        return result;
    }
    Mat nextImageFrom2()
    {
        Mat result;
        if( inputCapture2.isOpened() )
        {
            Mat view0;
            inputCapture2 >> view0;
            view0.copyTo(result);
        }
        else if( atImageList < (int)imageList2.size() )
            result = imread(imageList2[atImageList++], CV_LOAD_IMAGE_COLOR);

        return result;
    }

    static bool readStringList( const string& filename, vector<string>& l )
    {
        l.clear();
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() )
            return false;
        FileNode n = fs.getFirstTopLevelNode();
        if( n.type() != FileNode::SEQ )
            return false;
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it )
            l.push_back((string)*it);
        return true;
    }
public:
    Size boardSize;            // The size of the board -> Number of items by width and height
    Pattern calibrationPattern;// One of the Chessboard, circles, or asymmetric circle pattern
    float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
    int nrFrames;              // The number of frames to use from the input for calibration
    float aspectRatio;         // The aspect ratio
    int delay;                 // In case of a video input
    bool bwritePoints;         //  Write detected feature points
    bool bwriteExtrinsics;     // Write extrinsic parameters
    bool calibZeroTangentDist; // Assume zero tangential distortion
    bool calibFixPrincipalPoint;// Fix the principal point at the center
    bool flipVertical;          // Flip the captured images around the horizontal axis
    string outputFileName;      // The name of the file where to write
    bool showUndistorsed;       // Show undistorted images after calibration
    string input1;               // cam1 input
    string input2;               // cam2 input




    int cameraID1;
    int cameraID2;
    vector<string> imageList1;
    vector<string> imageList2;
    int atImageList;
    VideoCapture inputCapture1;
    VideoCapture inputCapture2;
    InputType inputType;
    bool goodInput;
    int flag;

private:
    string patternToUse;


};

static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix1, Mat& distCoeffs1,vector<vector<Point2f> > imagePoints1,
                                                        Mat&  cameraMatrix2, Mat& distCoeffs2,vector<vector<Point2f> > imagePoints2 );

int main(int argc, char* argv[])
{
    Settings s;
    const string inputSettingsFile = argc > 1 ? argv[1] : "data/stereo_calibration.xml";
    FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
    if (!fs.isOpened())
    {
        cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
        return -1;
    }
    fs["Settings"] >> s;
    fs.release();                                         // close Settings file

    if (!s.goodInput)
    {
        cout << "Invalid input detected. Application stopping. " << endl;
        return -1;
    }

    vector<vector<Point2f> > imagePoints1,imagePoints2;
    Mat cameraMatrix1, distCoeffs1,cameraMatrix2, distCoeffs2;
    Size imageSize;
    int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
    clock_t prevTimestamp = 0;
    const Scalar RED(0,0,255), GREEN(0,255,0);
    const char ESC_KEY = 27;

    for(int i = 0;;++i)
    {
      Mat view1,view2;
      bool blinkOutput = false;

      view1 = s.nextImageFrom1();
      view2 = s.nextImageFrom2();
      //-----  If no more image, or got enough, then stop calibration and show result -------------
      if( mode == CAPTURING && imagePoints1.size() >= (unsigned)s.nrFrames && imagePoints2.size() == imagePoints1.size())
      {
          if( runCalibrationAndSave(s, imageSize,  cameraMatrix1, distCoeffs1, imagePoints1,cameraMatrix2, distCoeffs2, imagePoints2))
              mode = CALIBRATED;
          else
              mode = DETECTION;
      }
      if(view1.empty() || view2.empty())          // If no more images then run calibration, save and stop loop.
      {
            if( imagePoints1.size() > 0 && imagePoints2.size() > 0 )
                runCalibrationAndSave(s, imageSize,  cameraMatrix1, distCoeffs1, imagePoints1,cameraMatrix2, distCoeffs2, imagePoints2);
            break;
      }


        imageSize = view1.size();  // Format input image.
        if( s.flipVertical ){
             flip( view1, view1, 0 );
             flip( view2, view2, 0 );
        }

        vector<Point2f> pointBuf1,pointBuf2;

        bool found;
        switch( s.calibrationPattern ) // Find feature points on the input format
        {
        case Settings::CHESSBOARD:
            found = findChessboardCorners( view1, s.boardSize, pointBuf1,
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE)
                &&  findChessboardCorners( view2, s.boardSize, pointBuf2,
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case Settings::CIRCLES_GRID:
            found = findCirclesGrid( view1, s.boardSize, pointBuf1 ) && findCirclesGrid( view2, s.boardSize, pointBuf2 );
            break;
        case Settings::ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid( view1, s.boardSize, pointBuf1, CALIB_CB_ASYMMETRIC_GRID )
                    && findCirclesGrid( view2, s.boardSize, pointBuf2, CALIB_CB_ASYMMETRIC_GRID );
            break;
        default:
            found = false;
            break;
        }

        if ( found)                // If done with success,
        {
              // improve the found corners' coordinate accuracy for chessboard
                if( s.calibrationPattern == Settings::CHESSBOARD)
                {
                    Mat viewGray1,viewGray2;
                    cvtColor(view1, viewGray1, COLOR_BGR2GRAY);
                    cornerSubPix( viewGray1, pointBuf1, Size(11,11),
                        Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
                    cvtColor(view2, viewGray2, COLOR_BGR2GRAY);
                    cornerSubPix( viewGray2, pointBuf2, Size(11,11),
                        Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
                }

                if( mode == CAPTURING &&  // For camera only take new samples after delay time
                    (!(s.inputCapture1.isOpened() && s.inputCapture2.isOpened())
                        || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC) )
                {
                    imagePoints1.push_back(pointBuf1);
                    imagePoints2.push_back(pointBuf2);
                    prevTimestamp = clock();
                    blinkOutput = s.inputCapture1.isOpened()&&s.inputCapture2.isOpened();
                }

                // Draw the corners.
                drawChessboardCorners( view1, s.boardSize, Mat(pointBuf1), found );
                drawChessboardCorners( view2, s.boardSize, Mat(pointBuf2), found );
        }
        Mat view;
        hconcat(view1,view2,view);
        //----------------------------- Output Text ------------------------------------------------
        string msg = (mode == CAPTURING) ? "100/100,100/100" :
                      mode == CALIBRATED ? "UnCalibrated" : "Press 'g' to start";
        int baseLine = 0;
        Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
        Point textOrigin(view.cols - 2*textSize.width - 10, view.rows - 2*baseLine - 10);

        if( mode == CAPTURING )
        {
            if(s.showUndistorsed)
                msg = format( "%d/%d Undist", (int)imagePoints1.size(), s.nrFrames );
            else
                msg = format( "%d/%d %d/%d", (int)imagePoints1.size(), s.nrFrames , (int)imagePoints1.size(), s.nrFrames);
        }

        putText( view, msg, textOrigin, 1, 1, mode == CALIBRATED ?  GREEN : RED);

        if( blinkOutput )
            bitwise_not(view, view);

        //------------------------- Video capture  output  undistorted ------------------------------
        if( mode == CALIBRATED && s.showUndistorsed )
        {
            Mat temp1 = view1.clone();
            Mat temp2 = view2.clone();
            undistort(temp1, view1, cameraMatrix1, distCoeffs1);
            undistort(temp2, view2, cameraMatrix2, distCoeffs2);
            hconcat(view1,view2,view);
            putText(view,"Calibrated",textOrigin,1,1,GREEN);
        }

        //------------------------------ Show image and check for input commands -------------------
        imshow("Image View", view);
        char key = (char)waitKey(s.inputCapture1.isOpened()&&s.inputCapture2.isOpened() ? 50 : s.delay);

        if( key  == ESC_KEY )
            break;

        if( key == 'u' && mode == CALIBRATED )
           s.showUndistorsed = !s.showUndistorsed;

        if( s.inputCapture1.isOpened() && s.inputCapture2.isOpened() && key == 'g' )
        {
            mode = CAPTURING;
            imagePoints1.clear();
            imagePoints2.clear();
        }
    }

    // -----------------------Show the undistorted image for the image list ------------------------
    if( s.inputType == Settings::IMAGE_LIST && s.showUndistorsed )
    {
        Mat view1, rview1, map11, map21;
        Mat view2, rview2, map12, map22;
        initUndistortRectifyMap(cameraMatrix1, distCoeffs1, Mat(),
            getOptimalNewCameraMatrix(cameraMatrix1, distCoeffs1, imageSize, 1, imageSize, 0),
            imageSize, CV_16SC2, map11, map21);
        initUndistortRectifyMap(cameraMatrix2, distCoeffs2, Mat(),
            getOptimalNewCameraMatrix(cameraMatrix2, distCoeffs2, imageSize, 1, imageSize, 0),
            imageSize, CV_16SC2, map12, map22);

        for(int i = 0; i < (int)s.imageList1.size(); i++ )
        {
            view1 = imread(s.imageList1[i], 1);
            view2 = imread(s.imageList2[i], 1);
            if(view1.empty()||view2.empty())
                continue;
            remap(view1, rview1, map11, map21, INTER_LINEAR);
            remap(view2, rview2, map12, map22, INTER_LINEAR);
            Mat rview;
            hconcat(rview1,rview2,rview);
            imshow("Image View", rview);
            char c = (char)waitKey();
            if( c  == ESC_KEY || c == 'q' || c == 'Q' )
                break;
        }
    }


    return 0;
}
/*
static double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                         const vector<vector<Point2f> >& imagePoints,
                                         const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                         const Mat& cameraMatrix , const Mat& distCoeffs,
                                         vector<float>& perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i )
    {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);

        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}*/

static void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
                                     Settings::Pattern patternType /*= Settings::CHESSBOARD*/)
{
    corners.clear();

    switch(patternType)
    {
    case Settings::CHESSBOARD:
    case Settings::CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; ++i )
            for( int j = 0; j < boardSize.width; ++j )
                corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));
        break;

    case Settings::ASYMMETRIC_CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
            for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float((2*j + i % 2)*squareSize), float(i*squareSize), 0));
        break;
    default:
        break;
    }
}

static bool runCalibration( Settings& s, Size& imageSize,
                            Mat& cameraMatrix1, Mat& distCoeffs1, vector<vector<Point2f> > imagePoints1,
                            Mat& cameraMatrix2, Mat& distCoeffs2, vector<vector<Point2f> > imagePoints2,
                            Mat& R, Mat& T, Mat& E, Mat& F
                            /*vector<float>& reprojErrs,  double& totalAvgErr*/)
{

    cameraMatrix1 = Mat::eye(3, 3, CV_64F);
    cameraMatrix2 = Mat::eye(3, 3, CV_64F);
    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO ){
        cameraMatrix1.at<double>(0,0) = 1.0;
        cameraMatrix2.at<double>(0,0) = 1.0;
    }

    distCoeffs1 = Mat::zeros(8, 1, CV_64F);
    distCoeffs2 = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);

    objectPoints.resize(imagePoints1.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    double rms = stereoCalibrate(objectPoints, imagePoints1,imagePoints2,  cameraMatrix1,distCoeffs1,
                                    cameraMatrix2, distCoeffs2, imageSize, R, T, E, F,
                                    cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5),s.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;

    bool ok = checkRange(cameraMatrix1) && checkRange(distCoeffs1) && checkRange(cameraMatrix2) && checkRange(distCoeffs2);

/*    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                             rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);*/

    return ok;
}

// Print camera parameters to the output file
/*

        saveCameraParams( s, imageSize, cameraMatrix1, distCoeffs1, imagePoints1,
                                        cameraMatrix2, distCoeffs2, imagePoints2);
*/
static void saveCameraParams( Settings& s, Size& imageSize,
                              Mat& cameraMatrix1, Mat& distCoeffs1, const vector<vector<Point2f> >& imagePoints1,
                              Mat& cameraMatrix2, Mat& distCoeffs2, const vector<vector<Point2f> >& imagePoints2,
								Mat& R,Mat& T,Mat& E,Mat& F)
{
    FileStorage fs( s.outputFileName, FileStorage::WRITE );

    time_t tm;
    time( &tm );
    struct tm *t2 = localtime( &tm );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_Time" << buf;
    fs << "Input1" << s.input1;
    fs << "Input2" << s.input2;
    fs << "image_Width" << imageSize.width;
    fs << "image_Height" << imageSize.height;
    fs << "board_Width" << s.boardSize.width;
    fs << "board_Height" << s.boardSize.height;
    fs << "square_Size" << s.squareSize;

    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        fs << "FixAspectRatio" << s.aspectRatio;

    if( s.flag )
    {
        sprintf( buf, "flags: %s%s%s%s",
            s.flag & CV_CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "",
            s.flag & CV_CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "",
            s.flag & CV_CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "",
            s.flag & CV_CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "" );
        cvWriteComment( *fs, buf, 0 );

    }

    fs << "flagValue" << s.flag;

    fs << "Camera_Matrix_1" << cameraMatrix1;
    fs << "Distortion_Coefficients_1" << distCoeffs1;
	fs << "Camera_Matrix_2" << cameraMatrix2;
    fs << "Distortion_Coefficients_2" << distCoeffs2;

	fs << "R" << R;
	fs << "T" << T;
	fs << "E" << E;
	fs << "F" << F;

/*
    fs << "Avg_Reprojection_Error" << totalAvgErr;
    if( !reprojErrs.empty() )
        fs << "Per_View_Reprojection_Errors" << Mat(reprojErrs);

    if( !rvecs.empty() && !tvecs.empty() )
    {
        CV_Assert(rvecs[0].type() == tvecs[0].type());
        Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
        for( int i = 0; i < (int)rvecs.size(); i++ )
        {
            Mat r = bigmat(Range(i, i+1), Range(0,3));
            Mat t = bigmat(Range(i, i+1), Range(3,6));

            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
            // .t() is MatExpr (not Mat) so we can use assignment operator
            r = rvecs[i].t();
            t = tvecs[i].t();
        }
        cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
        fs << "Extrinsic_Parameters" << bigmat;
    }

    if( !imagePoints.empty() )
    {
        Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
        for( int i = 0; i < (int)imagePoints.size(); i++ )
        {
            Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
            Mat imgpti(imagePoints[i]);
            imgpti.copyTo(r);
        }
        fs << "Image_points" << imagePtMat;
    }*/
}

bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix1, Mat& distCoeffs1,vector<vector<Point2f> > imagePoints1,
                                                        Mat&  cameraMatrix2, Mat& distCoeffs2,vector<vector<Point2f> > imagePoints2 )
{
    Mat R,T,E,F;
    bool ok = runCalibration(s,imageSize, cameraMatrix1, distCoeffs1, imagePoints1,
                                cameraMatrix2, distCoeffs2, imagePoints2, R,T,E,F);
/*    cout << (ok ? "Calibration succeeded" : "Calibration failed")
        << ". avg re projection error = "  << totalAvgErr ;
*/
    if( ok )
        saveCameraParams( s, imageSize, cameraMatrix1, distCoeffs1, imagePoints1,
                                        cameraMatrix2, distCoeffs2, imagePoints2,
										R,T,E,F);
    return ok;
}

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ntcore.h>
#include "networktables/NetworkTable.h"
#include <iostream>

using namespace cv;
using namespace std;


vector<vector<Point> > contours;
vector<vector<Point> > filteredContours;

vector<vector<Point> > contours2;
vector<vector<Point> > filteredContours2;

void filterContours(std::vector<std::vector<cv::Point> > &inputContours, double minArea, double minPerimeter, double minWidth, double maxWidth, double minHeight, double maxHeight, double solidity[], double maxVertexCount, double minVertexCount, double minRatio, double maxRatio, std::vector<std::vector<cv::Point> > &output) {
        std::vector<cv::Point> hull;
        output.clear();
        for (std::vector<cv::Point> contour: inputContours) {
            cv::Rect bb = boundingRect(contour);
            if (bb.width < minWidth || bb.width > maxWidth) continue;
            if (bb.height < minHeight || bb.height > maxHeight) continue;
            double area = cv::contourArea(contour);
            if (area < minArea) continue;
            if (arcLength(contour, true) < minPerimeter) continue;
            cv::convexHull(cv::Mat(contour, true), hull);
            double solid = 100 * area / cv::contourArea(hull);
            if (solid < solidity[0] || solid > solidity[1]) continue;
            if (contour.size() < minVertexCount || contour.size() > maxVertexCount)    continue;
            double ratio = bb.width / bb.height;
            if (ratio < minRatio || ratio > maxRatio) continue;
            output.push_back(contour);
        }
    }

int main(int argc, char*argv[]){
//Network Table setup

    NetworkTable::SetClientMode();
    NetworkTable::SetIPAddress("roboRIO-3482-frc.local");

    auto table = NetworkTable::GetTable("Vision");

    VideoCapture cap(0);
    VideoCapture cap2("http://roboRIO-3482-frc.local:1181/?action=stream");

    if(!cap.isOpened()){
	cout << "Cannot open video cam 1" << endl;
	return -1;
    }
    if(!cap2.isOpened()){
	cout << "Cannot open video cam 2" << endl;
	return -1;
    }


    double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    namedWindow("cam1Direct", CV_WINDOW_AUTOSIZE);
    namedWindow("cam2Direct", CV_WINDOW_AUTOSIZE);
    namedWindow("rgb trackbars", CV_WINDOW_AUTOSIZE);

    int lowH = 32;
    int lowS = 0;
    int lowV = 252;
    int highH = 81;
    int highS = 46;
    int highV = 255;
    int lowH2 = 30;
    int lowS2 = 0;
    int lowV2 = 250;
    int highH2 = 81;
    int highS2 = 44;
    int highV2 = 255;
    int lowR = 0;
    int lowG = 10;
    int lowB = 10;
    int highR = 5;
    int highG = 255;
    int highB = 134;
    

//    namedWindow("Contours", CV_WINDOW_AUTOSIZE);
//    namedWindow("HSVThresholded", CV_WINDOW_AUTOSIZE);
 
    while(1){
	Mat frame;
	Mat frame2;
	bool bSuccess = cap.read(frame);
	bool b2Success = cap2.read(frame2);
	if(!bSuccess){
	    cout << "Cannot read frame from camera" << endl;
	    break;
	}
	if(!b2Success){
	    cout << "Cannot read frame from camera 2" << endl;
	    break;
	}
	
	Mat thresholded;
	Mat HSVThresholded;
	Mat thresholded2;
	Mat HSVThresholded2;
	Mat RGBThreshold;

	//HSV Threshold
	cvtColor(frame, thresholded,CV_BGR2HSV);
	inRange(thresholded, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), thresholded);
        
	
	//RGB Threshold
	inRange(frame2, Scalar(lowB, lowG, lowR), Scalar(highB, highG, highR), RGBThreshold);
	
	//HSV Threshold 2
	cvtColor(frame2, thresholded2, CV_BGR2HSV);
        inRange(thresholded2, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), thresholded2);
        imshow("T2 Part1", thresholded2);
	
	//clean up image
	//erode(thresholded, thresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//dilate(thresholded, thresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//dilate(thresholded, thresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//erode(thresholded, thresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	
	//erode(thresholded2, thresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        //dilate(thresholded2, thresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        //dilate(thresholded2, thresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        //erode(thresholded2, thresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//get contours and put number of contours to networktable
	findContours(thresholded, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	table->PutNumber("nContours Cam1: ", contours.size());

	findContours(thresholded2, contours2, RETR_LIST, CHAIN_APPROX_SIMPLE);
	table->PutNumber("nContours Cam2: ", contours2.size());  

	//Filter contours
	vector<vector<Point> > filterContoursOutput;
	std::vector<std::vector<cv::Point> > filterContoursContours = contours;
        double filterContoursMinArea = 250.0;  // default Double
        double filterContoursMinPerimeter = 0;  // default Double
        double filterContoursMinWidth = 0;  // default Double
        double filterContoursMaxWidth = 1000;  // default Double
        double filterContoursMinHeight = 0;  // default Double
        double filterContoursMaxHeight = 1000;  // default Double
        double filterContoursSolidity[] = {0, 100};
        double filterContoursMaxVertices = 150.0;  // default Double
        double filterContoursMinVertices = 25.0;  // default Double
        double filterContoursMinRatio = 2.0;  // default Double
        double filterContoursMaxRatio = 5.0;  // default Double
        filterContours(filterContoursContours, filterContoursMinArea, filterContoursMinPerimeter, filterContoursMinWidth, filterContoursMaxWidth, filterContoursMinHeight, filterContoursMaxHeight, filterContoursSolidity, filterContoursMaxVertices, filterContoursMinVertices, filterContoursMinRatio, filterContoursMaxRatio, filterContoursOutput); 
	
	vector<vector<Point> > filterContoursOutput2;
        std::vector<std::vector<cv::Point> > filterContoursContours2 = contours2;
	filterContours(filterContoursContours2, filterContoursMinArea, filterContoursMinPerimeter, filterContoursMinWidth, filterContoursMaxWidth, filterContoursMinHeight, filterContoursMaxHeight, filterContoursSolidity, filterContoursMaxVertices, filterContoursMinVertices, filterContoursMinRatio, filterContoursMaxRatio, filterContoursOutput2);
	
	table->PutNumber("fContours", filterContoursOutput2.size());
	
	//draw contours and put center of first contour to networktable
	Mat c(640, 480, CV_8UC3, Scalar(0));
	for(int i = 0; i < filterContoursOutput.size(); i++){
	    Rect r = boundingRect(filterContoursOutput[i]);
	    rectangle(c, r, Scalar(255,255,255));
	}
	if(filterContoursOutput.size() != 0){
	    Rect rect = boundingRect(filterContoursOutput[0]);
	    double centerX = rect.x + rect.width / 2;
	    table->PutNumber("centerX Cam1: ", centerX);
	}
	
	Mat c2(640, 480, CV_8UC3, Scalar(0));
        for(int i = 0; i < filterContoursOutput2.size(); i++){
            Rect r = boundingRect(filterContoursOutput2[i]);
            rectangle(c2, r, Scalar(255,255,255));
        }
        if(filterContoursOutput2.size() != 0){
            Rect rect = boundingRect(filterContoursOutput2[0]);
            double centerX2 = rect.x + rect.width / 2;
            table->PutNumber("centerX Cam2: ", centerX2);
        }


	//imshow("cam1Direct", frame);
	imshow("cam2Direct", frame2);
	//imshow("Contours Cam1", c);
	//imshow("HSVThresholded Cam1", thresholded);
	imshow("HSVThreshold cam2", thresholded2);
	imshow("Contours Cam2", c2);

	//imshow("RGB Threshold cam2", RGBThreshold);

	if(waitKey(30) == 27){
	    cout << "esc key pressed by user" << endl;
	    break;
	}
    }
    return 0;
}


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

    VideoCapture cap(1);

    if(!cap.isOpened()){
	cout << "Cannot open video cam" << endl;
	return -1;
    }

    double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    namedWindow("MyVideo", CV_WINDOW_AUTOSIZE);
    namedWindow("OG", CV_WINDOW_AUTOSIZE);
    

    int lowH = 32;
    int lowS = 0;
    int lowV = 252;
    int highH = 81;
    int highS = 46;
    int highV = 255;


    namedWindow("Contours", CV_WINDOW_AUTOSIZE);
    namedWindow("HSVThresholded", CV_WINDOW_AUTOSIZE);
 
    while(1){
	Mat frame;
	bool bSuccess = cap.read(frame);
	if(!bSuccess){
	    cout << "Cannot read frame from camera" << endl;
	    break;
	}
	Mat thresholded;
	Mat HSVThresholded;
	//HSV Threshold
	cvtColor(frame, thresholded,CV_BGR2HSV);
	inRange(thresholded, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), thresholded);
        cvtColor(frame, HSVThresholded,CV_BGR2HSV);
	inRange(HSVThresholded, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), HSVThresholded);
	//clean up image
	erode(thresholded, thresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(thresholded, thresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(thresholded, thresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(thresholded, thresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	
	//get contours nd put number of contours to networktable
	findContours(thresholded, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	table->PutNumber("nContours", contours.size());


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
	
	//draw contours and put center of first contour to networktable
	Mat c(640, 480, CV_8UC3, Scalar(0));
	for(int i = 0; i < filterContoursOutput.size(); i++){
	    Rect r = boundingRect(filterContoursOutput[i]);
	    rectangle(c, r, Scalar(255,255,255));
	}
	if(filterContoursOutput.size() != 0){
	    Rect rect = boundingRect(filterContoursOutput[0]);
	    double centerX = rect.x + rect.width / 2;
	    table->PutNumber("centerX", centerX);
	}
	
	imshow("MyVideo", thresholded);
	imshow("OG", frame);
	imshow("Contours", c);
	imshow("HSVThresholded", HSVThresholded);

	if(waitKey(30) == 27){
	    cout << "esc key pressed by user" << endl;
	    break;
	}
    }
    return 0;
}


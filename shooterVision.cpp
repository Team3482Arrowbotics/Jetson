#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ntcore.h>
#include "networktables/NetworkTable.h"
#include <iostream>

using namespace cv;
using namespace std;

vector<vector<Point> > contours;
vector<Rect> filteredContourRectangles;

int centerDelta = -25565;

int lowH = 32;
int lowS = 0;
int lowV = 252;
int highH = 81;
int highS = 46;
int highV = 255;
int lowR = 30;
int lowG = 0;
int lowB = 250;
int highR = 81;
int highG = 125;
int highB = 73;

int main(){
    //Instantiate camera
    VideoCapture cap("http://roboRIO-3482-frc.local:1181/?action=stream");
    if(!cap.isOpened()){
        cout << "Could not instantiate shooter camera" << endl;
	return 0;
    }
    
    //NetworkTables setup
    NetworkTable::SetClientMode();
    NetworkTable::SetIPAddress("roboRIO-3482-frc.local");
    auto table = NetworkTable::GetTable("Vision");
    
    while(1){
	Mat frame;
	Mat HSV;
	Mat HSVThresholded;
	bool hasFrame = cap.read(frame);
	if(!hasFrame){
	    cout << "Camera cannot read frame" << endl;
	} else{
	    //double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	    //double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	    cvtColor(frame, HSV, COLOR_BGR2HSV);
	    inRange(HSV, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), HSVThresholded);
	    findContours(HSVThresholded, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	    for(vector<Point> c : contours){
		Rect r =  boundingRect(c);		
		if(r.area() > 250 && r.width / r.height > 2){
		    filteredContourRectangles.push_back(r);
		}
	    }
	    if(filteredContourRectangles.size() > 0){
	        centerDelta = 320 - (filteredContourRectangles[0].x + filteredContourRectangles[0].width/2);
	    }
	    table->PutNumber("centerDelta", centerDelta);
	    table->PutBoolean("centered", abs(centerDelta) <= 20);
	    filteredContourRectangles.clear();
	    centerDelta = -25565;
	}
	if(waitKey(30) == 27){
            cout << "esc key pressed by user" << endl;
            break;
        }

    }
}

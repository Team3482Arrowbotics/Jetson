#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;


int fileNum = 0;
int main(int argc, char* argv[]){
    VideoCapture inImg("http://roboRIO-3482-frc.local:1181/?action=stream");
    cout << "VideoCapture opened: " << inImg.isOpened() << endl;
    //Variables for trackbars
    int lowH = 0;
    int lowS = 0;
    int lowV = 0;
    int highH = 180;
    int highS = 255;
    int highV = 255;
    int lowR = 0;
    int lowG = 0;
    int lowB = 0;
    int highR = 255;
    int highG = 255;
    int highB = 255;
    
    
    //Window for trackbars
    namedWindow("trackbars");
    createTrackbar("lowH", "trackbars", &lowH, 180);
    createTrackbar("highH", "trackbars", &highH, 255);
    createTrackbar("lowS", "trackbars", &lowS, 255);
    createTrackbar("highS", "trackbars", &highS, 255);
    createTrackbar("lowV", "trackbars", &lowV, 255);
    createTrackbar("highV", "trackbars", &highV, 255);
    createTrackbar("lowB", "trackbars", &lowB, 180);
    createTrackbar("highB", "trackbars", &highB, 255);
    createTrackbar("lowG", "trackbars", &lowG, 255);
    createTrackbar("highG", "trackbars", &highG, 255);
    createTrackbar("lowR", "trackbars", &lowR, 255);
    createTrackbar("highR", "trackbars", &highR, 255);
    
    while(1){
	Mat frame;
	bool hasFrame = inImg.read(frame);
	if(!hasFrame){
	    cout << "cannot read picture from camera" << endl;
	    return -1;
	}
	//show original image
	imshow("original image", frame);
	//Run BGR Threshold
	Mat BGRThresholded;
	inRange(frame, Scalar(lowB, lowG, lowR), Scalar(highB, highG, highR), BGRThresholded);
	//Show BGR Thresholded image
	imshow("BGR", BGRThresholded);

	//Convert image to HSV
	Mat HSV;
	cvtColor(frame, HSV, COLOR_BGR2HSV);
	//Run HSV Threshold
	inRange(HSV, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), HSV);
	//Show HSV Thresholded image
	imshow("HSV", HSV);
	//Exit if user presses escape
	if(waitKey(30) == 27){
	    cout << "user exit" << endl;
	    break;
	}
	//Write to output file if user presses space
	else if(waitKey(30) == 32){
	    string name = "/home/ubuntu/cameraTest/output";
	   
	    string imgName = name + "/outImg" + to_string(fileNum) + ".jpg";
	    string BGRName = name + "/outBGR" + to_string(fileNum) + ".jpg";
	    string HSVName = name + "/outHSV" + to_string(fileNum) + ".jpg";
	    imwrite(imgName, frame);
	    imwrite(BGRName, BGRThresholded);
	    imwrite(HSVName, HSV);
	    cout << "wrote to " << "./output" << endl;
	    fileNum++;
	}
	
    }
    waitKey(0);
    return 1;
}

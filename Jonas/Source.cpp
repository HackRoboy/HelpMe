#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>

#include <fstream>
#include <string.h>

using namespace cv;
using namespace std;

int main(int, char**)
{
	double score = 0;
	double timer = 0;
	

	vector<vector<Point> > h_contours, v_contours;
	vector<Vec4i> hierarchy;

	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;
	Mat edges, h_edges, frame, hsvframe, templ1, background, frame1, edges1, test1; //allocate memoryedges;
	namedWindow("edges", 1);
	namedWindow("Background", 1);

	templ1 = imread("Hand3.png");
	cvtColor(templ1, h_edges, CV_BGR2GRAY);
	cap >> frame1; // get a new frame from camera
	cvtColor(frame1, edges1, CV_BGR2GRAY);
	//GaussianBlur(edges1, edges1, Size(9, 9), 1.5, 1.5);
	//medianBlur(edges1, edges1, 5);
	Laplacian(edges1, edges1, 0, 3, 1, 0, BORDER_DEFAULT);
	
	//Canny(edges1, edges1, 0, 40, 3);
	background = edges1;
	for (;;)
	{
		//cout << "Output";
		cap >> frame; // get a new frame from camera
		cvtColor(frame, edges, CV_BGR2GRAY);
		//GaussianBlur(edges, edges, Size(9, 9), 1.5, 1.5);
		Laplacian(edges, edges, 0, 3, 1, 0, BORDER_DEFAULT);
		//Laplacian(InputArray src, OutputArray dst, int ddepth, int ksize = 1, double scale = 1, double delta = 0, int borderType = BORDER_DEFAULT)
		//medianBlur(edges, edges, 3);
		
		
		   
		if (timer == 100) {

			timer = 0;
			cout << "Output";

			
			background = (background+ background+background + background + background + edges) / 6;
			
			boxFilter(background, test1, -1, Size(3,3), Point(-1, -1), false, BORDER_DEFAULT);
			//waitKey();
			}
		edges =edges - test1;
		Canny(edges, edges, 100, 50, 3);
		//medianBlur(edges, edges, 3);
		
		
		

		/*
		findContours(h_edges, h_contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		findContours(edges, v_contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		
	
		
		for (int i = 0; i < v_contours.size(); i++) {
			for (int g = 0; g < h_contours.size(); g++) {
				
				score = matchShapes(h_contours[g], v_contours[i], CV_CONTOURS_MATCH_I3, 0);
				//std::cout << score << std::endl;
				if ((score  > 1)) {
					
					Scalar color(255, 255, 255);

					drawContours(edges, v_contours, i, color, 10 , 8, hierarchy);
					
				}
				/*else {
					Scalar color(255, 255, 255);

					drawContours(edges, v_contours, i, color, CV_FILLED, 8, hierarchy);
				}//


			}
		}
		*/
		//imshow("edges", edges);
		timer++;
		cout << to_string(timer)<< endl;

		imshow("edges", edges);
		imshow("Background", background);

		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}
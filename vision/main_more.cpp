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
	//double score = 0;
	//double timer = 0;


	vector<vector<Point> > h_contours, v_contours;
	vector<Vec4i> hierarchy;

	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;
	Mat edges, h_edges, frame, hsvframe, templ1, background, frame1, edges1, test1; //allocate memoryedges;
	namedWindow("edges", 1);
	namedWindow("Background", 1);

	templ1 = imread("Hand3.png");
	cvtColor(templ1, h_edges, COLOR_BGR2GRAY);
	cap >> frame1; // get a new frame from camera
	cvtColor(frame1, edges1, COLOR_BGR2GRAY);
	GaussianBlur(edges1, edges1, Size(7, 7), 1.5, 1.5);
	//Canny(edges1, edges1, 0, 40, 3);
	background = edges1;
	Rect rect_fr = Rect(3, 3, frame.cols-7, frame.rows-7), rect_arm;
	Mat reality = Mat::zeros(frame.cols-7, frame.rows-7, CV_8UC1),
		aim = Mat::zeros(frame.cols-7, frame.rows-7, CV_8UC1),
		maxim = Mat::zeros(frame.cols-7, frame.rows-7, CV_8UC1),
		diff;
	reality = frame1(rect_fr);
	int min = 0;
	for (;;)
	{
		cap >> frame; // get a new frame from camera
		cvtColor(frame, edges, COLOR_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		for (int g=0; g<3; g++)
			for (int q=0; q<3; q++)
			{
				rect_arm = Rect(g, q, frame.cols-7, frame.rows-7);
				aim = frame(rect_arm);
				diff = reality - aim;
				double s = cv::sum(diff)[0];
				if (g==0 && q==0)
				{
					min = s;
					maxim = aim;
				}
				else
					if ( s<min )
					{
						min = s;
						maxim = aim;
					}
			}
		aim/*not related to name, just buffer!*/ = edges(rect_fr);
		edges = edges - maxim;
		imshow("edges", edges);
		edges = edges - test1;
		//Canny(edges, edges, 100, 50,r++;

		imshow("edges", edges);
		imshow("Background", background);

		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}

/*#include <iostream>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

#if defined(WIN32) || defined(_WIN32)
#include <io.h>
#else
#include <dirent.h>
#endif

#ifdef HAVE_CVCONFIG_H
#include <cvconfig.h>
#endif

#ifdef HAVE_TBB
#include "tbb/task_scheduler_init.h"
#endif

using namespace std;
using namespace cv;

static void help()
{
    cout << "This program demonstrated the use of the latentSVM detector." << endl <<
            "It reads in a trained object models and then uses them to detect the objects in an images." << endl <<
             endl <<
            "Call:" << endl <<
            "./latentsvm_multidetect <imagesFolder> <modelsFolder> [<overlapThreshold>][<threadsNumber>]" << endl <<
            "<overlapThreshold> - threshold for the non-maximum suppression algorithm." << endl <<
            "Example of <modelsFolder> is opencv_extra/testdata/cv/latentsvmdetector/models_VOC2007" << endl <<
             endl <<
            "Keys:" << endl <<
            "'n' - to go to the next image;" << endl <<
            "'esc' - to quit." << endl <<
            endl;
}

static void detectAndDrawObjects( Mat& image, LatentSvmDetector& detector, const vector<Scalar>& colors, float overlapThreshold, int numThreads )
{
    vector<LatentSvmDetector::ObjectDetection> detections;

    TickMeter tm;
    tm.start();
    detector.detect( image, detections, overlapThreshold, numThreads);
    tm.stop();

    cout << "Detection time = " << tm.getTimeSec() << " sec" << endl;

    const vector<string> classNames = detector.getClassNames();
    CV_Assert( colors.size() == classNames.size() );

    for( size_t i = 0; i < detections.size(); i++ )
    {
        const LatentSvmDetector::ObjectDetection& od = detections[i];
        rectangle( image, od.rect, colors[od.classID], 3 );
    }
    // put text over the all rectangles
    for( size_t i = 0; i < detections.size(); i++ )
    {
        const LatentSvmDetector::ObjectDetection& od = detections[i];
        putText( image, classNames[od.classID], Point(od.rect.x+4,od.rect.y+13), FONT_HERSHEY_SIMPLEX, 0.55, colors[od.classID], 2 );
    }
}

static void readDirectory( const string& directoryName, vector<string>& filenames, bool addDirectoryName=true )
{
    filenames.clear();

#if defined(WIN32) | defined(_WIN32)
    struct _finddata_t s_file;
    string str = directoryName + "\\*.*";

    intptr_t h_file = _findfirst( str.c_str(), &s_file );
    if( h_file != static_cast<intptr_t>(-1.0) )
    {
        do
        {
            if( addDirectoryName )
                filenames.push_back(directoryName + "\\" + s_file.name);
            else
                filenames.push_back((string)s_file.name);
        }
        while( _findnext( h_file, &s_file ) == 0 );
    }
    _findclose( h_file );
#else
    DIR* dir = opendir( directoryName.c_str() );
    if( dir != NULL )
    {
        struct dirent* dent;
        while( (dent = readdir(dir)) != NULL )
        {
            if( addDirectoryName )
                filenames.push_back( directoryName + "/" + string(dent->d_name) );
            else
                filenames.push_back( string(dent->d_name) );
        }

        closedir( dir );
    }
#endif

    sort( filenames.begin(), filenames.end() );
}

int main(int argc, char* argv[])
{
    help();

    string images_folder, models_folder;
    float overlapThreshold = 0.2f;
    int numThreads = -1;
    if( argc > 2 )
    {
        images_folder = argv[1];
        models_folder = argv[2];
        if( argc > 3 ) overlapThreshold = (float)atof(argv[3]);
        if( overlapThreshold < 0 || overlapThreshold > 1)
        {
            cout << "overlapThreshold must be in interval (0,1)." << endl;
            exit(-1);
        }

        if( argc > 4 ) numThreads = atoi(argv[4]);
    }

    vector<string> images_filenames, models_filenames;
    readDirectory( images_folder, images_filenames );
    readDirectory( models_folder, models_filenames );

    LatentSvmDetector detector( models_filenames );
    if( detector.empty() )
    {
        cout << "Models cann't be loaded" << endl;
        exit(-1);
    }

    const vector<string>& classNames = detector.getClassNames();
    cout << "Loaded " << classNames.size() << " models:" << endl;
    for( size_t i = 0; i < classNames.size(); i++ )
    {
        cout << i << ") " << classNames[i] << "; ";
    }
    cout << endl;

    cout << "overlapThreshold = " << overlapThreshold << endl;

    vector<Scalar> colors;
    generateColors( colors, detector.getClassNames().size() );

    for( size_t i = 0; i < images_filenames.size(); i++ )
    {
        Mat image = imread( images_filenames[i] );
        if( image.empty() )  continue;

        cout << "Process image " << images_filenames[i] << endl;
        detectAndDrawObjects( image, detector, colors, overlapThreshold, numThreads );

        imshow( "result", image );

        for(;;)
        {
            int c = waitKey();
            if( (char)c == 'n')
                break;
            else if( (char)c == '\x1b' )
                exit(0);
        }
    }

    return 0;
}*/


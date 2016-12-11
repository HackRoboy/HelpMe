#include <iostream>
#include <opencv2/opencv.hpp>
#include <deque>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>

// tcp/ip connection
#include <zmq.h>
#include <zmq_utils.h>
#include <string>
#include <iostream>
#include <thread>
#include "MessageTransmitter.h"

using namespace std;
using namespace cv;


#define DO_SERVER_SEND

#define SHOW_DEBUG_IMAGE


bool detectedPersonFalling(Mat &img, Mat &foreGround, BackgroundSubtractorMOG2 &mog);


MessageTransmitter g_messageTransmitter;

std::deque<bool> g_emergencyDetectedBuff;
std::deque<bool> g_emergencySendBuff;

//KF global variables
const int KF_stateSize = 6;
const int KF_measSize = 4;
const int KF_contrSize = 0;
double KF_ticks = 0;

KalmanFilter g_KF(KF_stateSize, KF_measSize, KF_contrSize, CV_32F);

void fillHoles(cv::Mat& im_in, cv::Mat& im_out)
{
    // create binary image
    Mat im_th;
    threshold(im_in, im_th, 200, 255, THRESH_BINARY_INV);


    // apply opening to the image to fill holes created by
    // noise
    Mat element = getStructuringElement( MORPH_ELLIPSE,
                                         Size( 8, 8) );
    morphologyEx( im_th, im_out, MORPH_OPEN, element, Point(-1,-1), 1 );
}


void filterNoise(cv::Mat &src_gray)
{
    medianBlur(src_gray, src_gray, 5);
}
/*
void mergeOverlappingBB(std::vector<cv::RotatedRect> & inboxes,
                        cv::RotatedRect& maxBox,
                        std::vector<cv::RotatedRect> &outBoxes,
                        const int width, const int height)
{

    cv::Mat maskColoed = cv::Mat::zeros(height, width, CV_8UC3);
    cv::Mat mask = cv::Mat::zeros(height, width, CV_8UC1);

    cv::rectangle(maskColoed, maxBox.boundingRect(), cv::Scalar(255, 0, 0), CV_FILLED);

    cv::rectangle(mask, maxBox.boundingRect(), cv::Scalar(255), CV_FILLED);
    for(auto box: inboxes)
    {
        cv::Point2f vec = box.center - maxBox.center;
        //cv::line(mask, maxBox.center, maxBox.center + vec, cv::Scalar(255), 2);

        cv::line(maskColoed, maxBox.center, maxBox.center + vec, cv::Scalar(255, 255, 0), 2);

        // boxes that are just around below the max bounding box
        if(std::abs(vec.y) <  maxBox.boundingRect().height / 2)
            continue;

        if( std::abs(vec.x)  < (6 * maxBox.boundingRect().width / 8) )
        {
            auto rect = box.boundingRect();
            rect = rect + cv::Size( 0, vec.y - rect.height);
            cv::rectangle(mask, rect, cv::Scalar(255), CV_FILLED); // Draw filled bounding boxes on mask


            cv::rectangle(maskColoed, rect, cv::Scalar(255, 255, 0), CV_FILLED); // Draw filled bounding boxes on mask
        }
    }

    cv::Mat final = cv::Mat::zeros(height, width, CV_8UC3);

    std::vector<std::vector<cv::Point>> contours;
    // Find contours in mask
    // If bounding boxes overlap, they will be joined by this function call
    cv::findContours(mask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    std::cout << "final contours " << contours.size() << std::endl;
    for (int j = 0; j < contours.size(); j++)
    {


        Mat pointsf;
        Mat(contours[j]).convertTo(pointsf, CV_32F);

        if( pointsf.rows < 5)
            continue;

        std::cout << "DIM:" << pointsf.rows << " " << pointsf.cols<< std::endl;
        RotatedRect extendedRect = fitEllipse(pointsf);
        outBoxes.push_back(extendedRect);

//        Point2f vtx[4];
//        extendedRect.points(vtx);
//        for( int j = 0; j < 4; j++ )
//            line(maskColoed, vtx[j], vtx[(j+1)%4], Scalar(0, 0, 255), 2, CV_AA);

        cv::rectangle(final, extendedRect.boundingRect(), cv::Scalar(255, 255, 0), CV_FILLED); // Draw filled bounding boxes on mask

    }

    imshow("merged Boxes!", mask);
    imshow("merged Col!", maskColoed);

    imshow("final Col!", final);

}
*/

bool detectFallingPersonElliseInBlob(cv::Mat bimage, cv::Mat &debugImage)
{
    // padd image by 2 pixels so contours are closed properly on the
    // border
    Mat extended(bimage.size()+Size(4 ,4), bimage.type());
    extended.setTo(255);

    cv::Mat roi = extended(cv::Rect(2, 2, bimage.size().width, bimage.size().height));
    bimage.copyTo(roi);

    // extract contours from the extended image
    vector<vector<Point> > contours;
    findContours(extended, contours, RETR_LIST, CHAIN_APPROX_NONE);


    debugImage = Mat::zeros(extended.size(), CV_8UC3);

    // find the contour of the maximum bonding box
    int maxSize = -1;
    int indexMax = -1;

    std::vector<RotatedRect> boxes;
    RotatedRect maxBox;
    for(size_t i = 0; i < contours.size(); i++)
    {
        size_t count = contours[i].size();

        // ignore simple contours
        if( count < 6 )
            continue;

        // fit ellipse
        Mat pointsf;
        Mat(contours[i]).convertTo(pointsf, CV_32F);
        RotatedRect box = fitEllipse(pointsf);

        // filter out boxes that take almos the entire image (due to extension of 2
        // pixels at the border
        if(box.size.width * box.size.height > debugImage.rows * debugImage.cols * 0.7)
            continue;

        // filter by size
        if( MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height)*10 )
            continue;

        // store all boxes
        boxes.push_back(box);

        // find the largest box
        if(box.boundingRect().area() > maxSize)
        {
            indexMax = i;
            maxBox = box;
            maxSize = box.boundingRect().area();
        }

        drawContours(debugImage, contours, (int)i, Scalar::all(255), 1, 8);

        ellipse(debugImage, box, Scalar(0,0,255), 1, CV_AA);
        ellipse(debugImage, box.center, box.size*0.5f, box.angle, 0, 360, Scalar(255,255,255), 1, CV_AA);
        Point2f vtx[4];
        box.points(vtx);
        for( int j = 0; j < 4; j++ )
            line(debugImage, vtx[j], vtx[(j+1)%4], Scalar(0,255,0), 1, CV_AA);
    }

    if(indexMax < 0)
        return false;

    std::vector<RotatedRect> filteredBoxes;
    for(RotatedRect box : boxes)
    {
        cv::Point2f vec = box.center - maxBox.center;

        auto length = vec.dot(vec);

        if(length == 0)//same box
        {
            filteredBoxes.push_back(box);
            circle(debugImage, box.center, 5, Scalar(255, 0, 0), 5 );
        }

        // filter out bounding boxes contained
        if(maxBox.boundingRect().contains(box.center))
        {
            continue;
        }

        filteredBoxes.push_back(box);
        circle(debugImage, box.center, 5, Scalar(0, 255, 0), 5 );
    }

    // only allow "large" objects/person
    if( maxBox.boundingRect().area() < 5000 )
    {
        g_emergencyDetectedBuff.push_back(false);
        g_emergencyDetectedBuff.pop_front();
        return false;
    }

    // update the KF
    double precTick = KF_ticks;
    KF_ticks = (double) cv::getTickCount();
    double dT = (KF_ticks - precTick) / cv::getTickFrequency(); //seconds

    g_KF.transitionMatrix.at<float>(2) = dT;
    g_KF.transitionMatrix.at<float>(9) = dT;

    Mat state = g_KF.predict();

    cv::RotatedRect predRect;
    predRect.size = cv::Size(state.at<float>(4),state.at<float>(5));
    predRect.center.x = state.at<float>(0);
    predRect.center.y = state.at<float>(1);
    predRect.angle = maxBox.angle; // we don't filter the angle

    cv::Point center;
    center.x = state.at<float>(0);
    center.y = state.at<float>(1);

    cv::circle(debugImage, center, 2, CV_RGB(255,0,0), -1);

    Point2f orientedBB[4];
    predRect.points(orientedBB);
    for( int j = 0; j < 4; j++ )
        line(debugImage, orientedBB[j], orientedBB[(j+1)%4], Scalar(0,255,0), 1, CV_AA);

    // [z_x,z_y,z_w,z_h]
    cv::Mat measurement(KF_measSize, 1, CV_32F);
    measurement.at<float>(0) =  maxBox.center.x;
    measurement.at<float>(1) =  maxBox.center.y;
    measurement.at<float>(2) =  maxBox.size.width;
    measurement.at<float>(3) =  maxBox.size.height;

    g_KF.correct(measurement);

    // create the debug image
    for(auto box: filteredBoxes)
    {
        ellipse(debugImage, box, Scalar(0,0,255), 1, CV_AA);
        ellipse(debugImage, box.center, box.size*0.5f, box.angle, 0, 360, Scalar(0,255,255), 1, CV_AA);
        Point2f vtx[4];
        box.points(vtx);
        for( int j = 0; j < 4; j++ )
            line(debugImage, vtx[j], vtx[(j+1)%4], Scalar(0,255,0), 1, CV_AA);

    }

    // ration between bb with and height indicates "lying/falling" person
    auto ratio = static_cast<float>(maxBox.boundingRect().width) / static_cast<float>(maxBox.boundingRect().height);
    bool flipped = ratio > 1.4;

    g_emergencyDetectedBuff.push_back(flipped);
    g_emergencyDetectedBuff.pop_front();

    int counter = 0;
    for(int i = 0; i < g_emergencyDetectedBuff.size(); i++)
    {
        if(g_emergencyDetectedBuff[i])
            counter++;
    }

    // if we already detected emergency we notify
    if(counter >= 2)
    {
        return true;

    }else
    {
        return false;
    }
}


bool detectedPersonFalling(Mat &img, Mat &debugImage, BackgroundSubtractorMOG2 &mog)
{
    cv::Mat foreGround;
    // do foreground segmentation
    mog.operator()(img, foreGround);

    // just take the foreground, no shadows
    foreGround = foreGround == 255;

    // filter out scatter pixels
    filterNoise(foreGround);

    Mat filled;
    fillHoles(foreGround, filled);


    return detectFallingPersonElliseInBlob(filled, debugImage);
}

void handleFallDetected(bool isFallDetected, cv::Mat& debugImage)
{
    if(isFallDetected)
    {
        std::cout << "Emergency!!!" << std::endl;
        std::string text = "Emergency!!!";

        // did we already sent it?
        for(auto haveEmergencySend :g_emergencySendBuff)
        {
            if(haveEmergencySend)
            {
                putText(debugImage, text.c_str(), cvPoint(debugImage.cols/8,debugImage.rows/2),
                        FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(200,200,250), 1, CV_AA);
                return;
            }
        }

        g_emergencySendBuff.push_back(true);
        g_emergencySendBuff.pop_front();


#ifdef DO_SERVER_SEND
        g_messageTransmitter.sendEmergencyMessage();
#endif
        putText(debugImage, text.c_str(), cvPoint(debugImage.cols/8,debugImage.rows/2),
                FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(200,200,250), 1, CV_AA);
    }
    else {
        putText(debugImage, "", cvPoint(debugImage.cols / 8, debugImage.rows / 2),
                FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(200, 200, 250), 1, CV_AA);

        g_emergencySendBuff.push_back(false);
        g_emergencySendBuff.pop_front();
    }
}

void initKF()
{
    // measurement cov
    g_KF.measurementMatrix = cv::Mat::zeros(KF_measSize, KF_stateSize, CV_32F);
    g_KF.measurementMatrix.at<float>(0) = 1.0f;
    g_KF.measurementMatrix.at<float>(7) = 1.0f;
    g_KF.measurementMatrix.at<float>(16) = 1.0f;
    g_KF.measurementMatrix.at<float>(23) = 1.0f;

    // process cov
    g_KF.processNoiseCov.at<float>(0) = 1e-2;
    g_KF.processNoiseCov.at<float>(7) = 1e-2;
    g_KF.processNoiseCov.at<float>(14) = 2.0f;
    g_KF.processNoiseCov.at<float>(21) = 1.0f;
    g_KF.processNoiseCov.at<float>(28) = 1e-2;
    g_KF.processNoiseCov.at<float>(35) = 1e-2;

    //measurement cov
    cv::setIdentity(g_KF.measurementNoiseCov, cv::Scalar(1e-1));
}

int main (int argc, const char * argv[])
{
    //init with false
    for(int i = 0; i < 8; i++)
    {
        g_emergencyDetectedBuff.push_back(false);
        g_emergencySendBuff.push_back(false);
    }


//  Prepare our context and socket

#ifdef DO_SERVER_SEND
    g_messageTransmitter = MessageTransmitter("tcp://10.177.254.73:5556");
    g_messageTransmitter.setup();
#endif

    //VideoCapture cap(0);
    //VideoCapture cap("/home/gecko/HackRoboy/OpenCVHandGuesture/video.mp4");f
    VideoCapture cap("/home/gecko/HackRoboy/helpMe/FallDetection/output.avi");

    cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    if (!cap.isOpened())
        return -1;

    Mat img;
    Mat blobs;
    cv::Mat debugImage;

    cv::BackgroundSubtractorMOG2 mog;

    mog.set("nmixtures", 10);
    mog.set("detectShadows",1);
    mog.set("backgroundRatio", 0.7);
    //mog.set("fVarInit", 0.05);
    mog.set("fCT", 0.0002);


/*
    Size S = Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));

    int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    VideoWriter outputVideo;                                        // Open the output
    outputVideo.open("output.avi", CV_FOURCC('D','I','V','X'), 30, S, true);

    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for write: " << endl;
        return -1;
    }
*/
    initKF();

    Mat prediction = g_KF.predict();
    Point predictPt(prediction.at<float>(0),prediction.at<float>(1));

    namedWindow("video capture", CV_WINDOW_AUTOSIZE);

    bool detectedFall = false;
    while (true)
    {
        bool done = cap.read(img);


        if(!done)
            return 0;
        if (!img.data)
            continue;

        detectedFall = detectedPersonFalling(img, debugImage, mog);

        handleFallDetected(detectedFall, debugImage);

        imshow("video capture", img);

#ifdef SHOW_DEBUG_IMAGE
        imshow("debugImage", debugImage);
#endif
        if (waitKey(40) == 'q')
            break;

    }

#ifdef DO_SERVER_SEND
    g_messageTransmitter.cleanup();
#endif
    return 0;
}

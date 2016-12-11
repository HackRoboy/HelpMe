#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


void fillHoles(cv::Mat& im_in, cv::Mat& im_out)
{
    // Threshold.
    // Set values equal to or above 220 to 0.
    // Set values below 220 to 255.
    Mat im_th;
    threshold(im_in, im_th, 200, 255, THRESH_BINARY_INV);
    //cv::imshow("thres", im_th);
    cv::Mat imgFlooded;

    cv::Mat unflooded;
    im_in.copyTo(unflooded);

    int morph_size = 2;
    Mat element = getStructuringElement( MORPH_ELLIPSE,
                                         Size( 8, 8) );

    morphologyEx( im_th, imgFlooded, MORPH_OPEN, element, Point(-1,-1), 1 );

    im_out = imgFlooded;
    /*
    cv::floodFill(im_th, cv::Point(0,0), 255);
    cv::imshow("flooeded", im_th);
    //bitwise_not(im_th, im_th);
    imgFlooded = (im_th | unflooded);

    cv::imshow("flooededOR", imgFlooded);



    dilate(im_th, im_out, Mat(), Point(-1, -1), 3, 1, 1);

    cv::imshow("Dilated", im_out);
    im_th =  im_th == 0;

     */
    //floodFill(im_floodfill, cv::Point(0,0), Scalar(255));
    //cv::imshow("Flood", im_floodfill);
    // Invert floodfilled image
    //Mat im_floodfill_inv;

    //bitwise_not(im_floodfill, im_out);
}


cv::Mat findBiggestBlob(cv::Mat & matImage, cv::Mat& imageDraw){
    int largest_area=0;
    int largest_contour_index=0;

    vector< vector<Point> > contours; // Vector for storing contour
    vector<Vec4i> hierarchy;

    findContours( matImage, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); // Find the contours in the image

    for( int i = 0; i< contours.size(); i++ ) {// iterate through each contour.
        double a=contourArea( contours[i],false);  //  Find the area of contour
        if(a>largest_area){
            largest_area=a;
            largest_contour_index=i;                //Store the index of largest contour
            //bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
        }
    }

    drawContours( imageDraw, contours, largest_contour_index, Scalar(255), CV_FILLED, 8, hierarchy ); // Draw the largest contour using previously stored index.
    return matImage;
}

void clearDots(cv::Mat& src_gray)
{
    medianBlur(src_gray, src_gray, 5);
}

void fitElliseToBlob(cv::Mat bimage)
{
    cv::Mat boundaries = cv::Mat::zeros(bimage.size(), CV_8UC3);
    //bimage.copyTo(boundaries);

    Mat extended(bimage.size()+Size(4,4), bimage.type());
    extended.setTo(255);
    cv::Mat roi = extended(cv::Rect(2, 2, bimage.size().width, bimage.size().height));
    bimage.copyTo(roi);
    cv::imshow("extened", extended);
    //Mat markers = extended(Rect(1, 1, bimage.cols, bimage.rows));

// all your calculation part

    //std::vector<std::vector<Point> > contours;
    //findContours(boundaries, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    /*
    Mat regions(bimage.size(), CV_8U);
    drawContours(regions, contours, -1, Scalar(255), CV_FILLED, 8, Mat(), INT_MAX, Point(-1,-1))
     */
     /*
    bitwise_or(boundaries.row(0), boundaries.row(1), boundaries.row(1));
    bitwise_or(boundaries.col(0), boundaries.col(1), boundaries.col(1));
    bitwise_or(boundaries.row(bimage.rows-1), boundaries.row(bimage.rows-2), boundaries.row(bimage.rows-2));
    bitwise_or(boundaries.col(bimage.cols-1), boundaries.col(bimage.cols-2), boundaries.col(bimage.cols-2));
    */
    vector<vector<Point> > contours;
    //findContours(extended, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    findContours(extended, contours, RETR_LIST, CHAIN_APPROX_NONE);
    Mat cimage = Mat::zeros(extended.size(), CV_8UC3);
    for(size_t i = 0; i < contours.size(); i++)
    {
        size_t count = contours[i].size();
        if( count < 6 )
            continue;

        Mat pointsf;
        Mat(contours[i]).convertTo(pointsf, CV_32F);
        RotatedRect box = fitEllipse(pointsf);

        if( MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height)*10 )
            continue;

        drawContours(cimage, contours, (int)i, Scalar::all(255), 1, 8);
        ellipse(cimage, box, Scalar(0,0,255), 1, CV_AA);
        ellipse(cimage, box.center, box.size*0.5f, box.angle, 0, 360, Scalar(0,255,255), 1, CV_AA);
        Point2f vtx[4];
        box.points(vtx);
        for( int j = 0; j < 4; j++ )
            line(cimage, vtx[j], vtx[(j+1)%4], Scalar(0,255,0), 1, CV_AA);
    }
    imshow("result", cimage);
}


int main (int argc, const char * argv[])
{
    VideoCapture cap(0);
    //VideoCapture cap("/home/gecko/HackRoboy/OpenCVHandGuesture/video.mp4");
    //VideoCapture cap("/home/gecko/HackRoboy/OpenCVHandGuesture/outputfile.mp4");
    //VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    if (!cap.isOpened())
        return -1;

    Mat img;
    Mat blobs;
    cv::Mat foreGround;

    cv::BackgroundSubtractorMOG2 mog;

    mog.set("nmixtures", 10);
    mog.set("detectShadows",1);
    mog.set("backgroundRatio", 0.7);
    //mog.set("fVarInit", 0.05);
    mog.set("fCT", 0.0002);

    // Set up the detector with default parameters.
        cv::SimpleBlobDetector::Params params;
    //params.minDistBetweenBlobs = 50.0f;
    /*
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = false;
    params.filterByCircularity = true;
    params.filterByArea = false;
    params.minArea = 60.0f;

*/
    params.filterByColor = true;
    params.blobColor = 0;
/*
    params.minThreshold = 0;
    params.maxThreshold = 256;
*/
    params.filterByArea = true;
    params.minArea = 400;
    params.maxArea = 1000000000;

    params.filterByCircularity = false;
    params.minCircularity = 0.001;

    params.filterByConvexity = false;
    params.minConvexity = 0.5;
    params.maxConvexity = 1.0;

    params.filterByInertia =false;
    params.minInertiaRatio = 0.5;
    //params.maxArea = 900.0f;

    SimpleBlobDetector detector(params);

    namedWindow("video capture", CV_WINDOW_AUTOSIZE);
    while (true)
    {
        bool done = cap.read(img);
        if(!done)
            return 0;
        if (!img.data)
            continue;



        mog(img, foreGround);
        foreGround = foreGround == 255;
        imshow("Foreground", foreGround);

        clearDots(foreGround);
        imshow("clearDots", foreGround);
        //imshow("clearDotsColor", img);
        //cv::imwrite("image.png", foreGround);
        cv::Mat filled;
        fillHoles(foreGround, filled);
        //cv::bitwise_not(filled, filled);
        //filled = filled == 0;
        //foreGround = cv::imfill(foreGround,'holes');

        //cv::imshow("Foreground", foreGround);
        cv::imshow("filled", filled);

        fitElliseToBlob(filled);
/*
        cv::Mat draw(filled.size(), CV_8UC3);
        draw.setTo(cv::Scalar(0, 0, 0));
        findBiggestBlob(filled, draw);

        cv::imshow("blob", filled);
        */

        //Mat im_th;
        //threshold(filled, im_th, 200, 255, THRESH_BINARY_INV);

        //cv::imshow("im_th", im_th);

// Detect blobs.
        std::vector<KeyPoint> keypoints;
        std::vector<KeyPoint> keypointsLargest;
        detector.detect( filled, keypoints);

        int iLargest = -1;
        int maxSize = 0;
        for(int i = 0; i < keypoints.size(); i++)
        {
            if(keypoints[i].size > maxSize)
            {
                iLargest = i;
                maxSize = keypoints[i].size;
                std::cout << "max " << maxSize << std::endl;
            }
        }
        if(iLargest >=0 ){
            keypointsLargest.push_back(keypoints[iLargest]);
        }

        std::cout << "Keypoints: " << keypoints.size() << std::endl;
// Draw detected blobs as red circles.
// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
        Mat im_with_keypoints;
        drawKeypoints( img, keypointsLargest, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

// Show blobs
        imshow("keypoints", im_with_keypoints );


        imshow("video capture", img);
        if (waitKey(10) == 'q')
            break;
    }


    return 0;
}
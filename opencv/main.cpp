//#include <QtDebug> // qDebug()
//#include <QApplication>

#define T 3

#if T==1
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#elif T==2
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#elif T==3
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
//    QApplication app(argc, argv);
    
//    MyClass window;
//    window.show();

//    return app.exec();

#if T==1
    Mat image;
    image = imread("foto1.jpg", CV_LOAD_IMAGE_COLOR);
    if (!image.data) {
        cout <<  "Could not open or find the image" << endl;
        return -1;
    }

    namedWindow("Display window", CV_WINDOW_AUTOSIZE);
    imshow("Display window", image);
    waitKey(0);
    return 0;
#elif T==2
    VideoCapture cap(0); // open the default camera
    if (!cap.isOpened()) {
        cout <<  "Could not open default camera" << endl;
        return -1;
    }

    Mat edges;
    namedWindow("edges", 1);
    for(;;) {
        Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, CV_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
        imshow("edges", edges);
        if (waitKey(30) >= 0) break;
    }

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
#elif T==3
    cv::VideoCapture vcap;

    //const std::string videoStreamAddress = "rtsp://cam_address:554/live.sdp";
    const std::string videoStreamAddress = "rtp://@127.0.0.1:1234";
    /* it may be an address of an mjpeg stream,
        e.g. "http://user:pass@cam_address:8081/cgi/mjpg/mjpg.cgi?.mjpg" */

    //open the video stream and make sure it's opened
    if(!vcap.open(videoStreamAddress)) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    //Create output window for displaying frames.
    //It's important to create this window outside of the `for` loop
    //Otherwise this window will be created automatically each time you call
    //`imshow(...)`, which is very inefficient.
    cv::namedWindow("Output Window", 1);

    Mat edges;
    for (;;) {
        cv::Mat frame;
        if (!vcap.read(frame)) {
            std::cout << "No frame" << std::endl;
            cv::waitKey();
        }
        cvtColor(frame, edges, CV_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
        cv::imshow("Output Window", edges);
        if (cv::waitKey(30) >= 0) break;
    }
    return 0;
#endif
}

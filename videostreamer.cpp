#include "videostreamer.h"


using namespace cv;
using namespace std;


VideoStreamer::VideoStreamer()
{
    connect(&tUpdate,&QTimer::timeout,this,&VideoStreamer::streamVideo);

}

//VideoStreamer::~VideoStreamer()
//{
//    cap.release();
//    tUpdate.stop();
//    threadStreamer->requestInterruption();
//}

void VideoStreamer::streamVideo()
{

    if(frame.data)
    {
        QImage img = QImage(frame.data,frame.cols,frame.rows,QImage::Format_RGB888).rgbSwapped();
        emit newImage(img);
    }
}

void VideoStreamer::catchFrame(cv::Mat emittedFrame)
{
    frame = emittedFrame;
}

void VideoStreamer::openVideoCamera(QString path)
{
    if(path.length() == 1)
    cap.open(path.toInt());
    //cap.open("rtsp://admin:1234@10.6.0.119:554/");
    else
    cap.open(path.toStdString());

    VideoStreamer* worker = new VideoStreamer();
    worker->moveToThread(threadStreamer);
    QObject::connect(threadStreamer,SIGNAL(started()),worker,SLOT(streamerThreadSlot()));
    QObject::connect(worker,&VideoStreamer::emitThreadImage,this,&VideoStreamer::catchFrame);

    //QObject::connect(&threadStreamer,SIGNAL(started()),this,&VideoStreamer::catchFrame);
    threadStreamer->start();

    double fps = cap.get(cv::CAP_PROP_FPS);
    tUpdate.start(1000/fps);
}

void VideoStreamer::streamerThreadSlot()
{
    cv::Mat tempFrame;

    while (1) {
        cap>>tempFrame;

        if(tempFrame.data)
            emit emitThreadImage(tempFrame);


        int frame_width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH)); //get the width of frames of the video
        int frame_height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT)); //get the height of frames of the video

        Size frame_size(frame_width, frame_height);

        //Size frame_size(800,600);
        //int h = 1024
        //int w = 600



        //int frames_per_second = 10;
        int frames_per_second = 25;
        VideoWriter oVideoWriter("MyVideo.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), frames_per_second, frame_size, true);

        //Mat frame;

        bool isSuccess = cap.read(tempFrame);


        if (waitKey(0) == 115)
        {
            cout << "S-key is pressed by the user. Start saving the video" << endl;
            while(1)
            {
                //imshow(window_name, frame);
                oVideoWriter.write(tempFrame);

                //bool isSuccess = cap.read(frame);
                 if (waitKey(3) == 99)
                {
                    cout << "C-key is pressed by the user. Stopping the video" << endl;
                    break;
                }
            //continue;
            }
            //break;
        }


        /*
        if (waitKey(3) == 115)
        {
            //while(waitKey(5)==99)
            oVideoWriter.write(frame);
            cout << "Save key is pressed by the user. Start saving the video" << endl;
            continue;
            //break;
        }

        */

        //Wait for for 10 milliseconds until any key is pressed.
        //If the 'Esc' key is pressed, break the while loop.
        //If any other key is pressed, continue the loop
        //If any key is not pressed within 10 milliseconds, continue the loop
        if (waitKey(0) == 27)
        {
            cout << "Esc key is pressed by the user. Exit the video" << endl;
            break;
        }



        if(QThread::currentThread()->isInterruptionRequested())
        {
            cap.release();
            return;
        }

    }
}

//---------------------------------------------------------------------------------------------------------------------
//  DVSAL
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2019 - Marco Montes Grova (a.k.a. marrcogrova) 
//---------------------------------------------------------------------------------------------------------------------
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
//  and associated documentation files (the "Software"), to deal in the Software without restriction, 
//  including without limitation the rights to use, copy, modify, merge, publish, distribute, 
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial 
//  portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES 
//  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------------------------------------------------

#include "dvsal/streamers/DvStreamer.h" 
#include "thread"

bool run = true;
dvsal::DvStreamer *streamer = nullptr;

int main(int _argc, char **_argv){

    streamer = dvsal::DvStreamer::create(dvsal::DvStreamer::eModel::dataset);

    std::string datasetPath = "/home/grvc/Downloads/shapes_translation/events.txt";
    if (!streamer->init(datasetPath)){
        std::cout << "Error creating streamer" << std::endl;
        return 0;
    }

    std::thread t([](){
        while (run){
            // std::cout << "thread function to visualize image \n";
            
            cv::Mat img = cv::Mat::zeros(cv::Size(240,180), CV_8UC3);
            streamer->image(img);
            
            cv::imshow("events",img);
            cv::waitKey(1);

            std::this_thread::sleep_for( std::chrono::milliseconds(30) );
        }
    });
    
    while(run){
        run = streamer->step();
        
        // std::this_thread::sleep_for( std::chrono::milliseconds(1) );
    }
    t.join();

    std::cout << "finished program" << std::endl;

    return 0;
}
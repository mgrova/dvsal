//---------------------------------------------------------------------------------------------------------------------
//  DVSAL
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2020 - Marco Montes Grova (a.k.a. mgrova) marrcogrova@gmail.com 
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

#include <dv-sdk/utils.h>
#include <dvsal/streamers/Streamer.h>
#include <dvsal/streamers/AEDAT4Streamer.h>
#include <filesystem>

dvsal::Streamer *streamer = nullptr;

int main(int _argc, char **_argv){

    std::filesystem::path filePath{_argv[1]};
    std::cout << filePath <<std::endl;

    std::ifstream fileStream = dvsal::AEDAT4Streamer::openFile(filePath);
	dvsal::InputInformation fileInfo = dvsal::AEDAT4Streamer::parseHeader(fileStream);
    
    streamer = dvsal::Streamer::create<dvsal::AEDAT4Streamer>(&fileStream , &fileInfo);
    if(!streamer->init()){
        std::cout << "Error creating streamer" << std::endl;
        return 0;
    }
    bool run = true;
    
    while(run){
        streamer->step();
        dv::EventStore UnfiltEvents;
        // streamer->events(UnfiltEvents , 1000);

        // cv::Mat image = cv::Mat::zeros(cv::Size(128,128),CV_8UC3);
		// for (auto ev: UnfiltEvents){
        // 	if (ev.polarity())
		// 		image.at<cv::Vec3b>(ev.y(), ev.x()) = cv::Vec3b(0,0,255);
		// 	else
		// 		image.at<cv::Vec3b>(ev.y(), ev.x()) = cv::Vec3b(0,255,0);
        // }
		// cv::imshow("tt",image);
		// cv::waitKey(0);

    }

    return 0;
}
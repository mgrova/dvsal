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

#include <boost/filesystem.hpp>
#include "dvsal/streamers/DvDatasetStreamer.h"

namespace dvsal{

    bool DvDatasetStreamer::init(const std::string &_string){
        datasetFile_ = std::ifstream(_string);
        
        if(!boost::filesystem::exists(_string)){
            std::cout << "Dataset file not exist" << std::endl;
            return false;
        }

        return true;
    }


    bool DvDatasetStreamer::step(){
        
        std::string line;
        std::getline(datasetFile_, line);  

        std::istringstream iss(line);
        float timestamp;
        int x, y, pol;
        
        iss >> timestamp >> x >> y >> pol;
        if (iss.good()){
            datasetFile_.close();
            return false;
        }
        dv::Event event(static_cast<int64_t>(timestamp * 1000000) , static_cast<int16_t>(x) , static_cast<int16_t>(y) , static_cast<uint8_t>(pol));
        
        events_.add(event);

        return true;
    }


    bool DvDatasetStreamer::events(dv::EventStore &_events){
        _events = events_;

        return true;
    } 

    bool DvDatasetStreamer::cutUsingTime(int _microseconds){
        
        events_ = events_.sliceTime(_microseconds);
        return true;
    }


    bool DvDatasetStreamer::image(cv::Mat &_image){

        dv::EventStore eventsLast30000microseconds = events_.sliceTime(-10000); //666   

        for (const auto &event : eventsLast30000microseconds) {
            if (event.polarity())
                _image.at<cv::Vec3b>(event.y(), event.x()) = cv::Vec3b(0,0,255);
            else
                _image.at<cv::Vec3b>(event.y(), event.x()) = cv::Vec3b(0,255,0);
        }


        return true;
    }    
}


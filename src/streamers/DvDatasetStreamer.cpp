//---------------------------------------------------------------------------------------------------------------------
//  DV-MICO
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


#include "dvsal/streamers/DvDatasetStreamer.h"

namespace dvsal{

    bool DvDatasetStreamer::init(const std::string &_string){
        datasetFile_ = new std::ifstream(_string);

        datasetFile_->open(_string);
        
        if(!datasetFile_->is_open()){
            std::cout << "Error reading dataset file" << std::endl;
            return false;
        }
        return true;
    }


    bool DvDatasetStreamer::step(){
        
        std::string line;
        std::getline(*datasetFile_, line);
        std::istringstream iss(line);
        float timestamp;
        int x, y, pol;
        
        iss >> timestamp >> x >> y >> pol;
        if (!iss.eof())
            return false;
        
        events_.add(dv::Event(x,y,pol,timestamp));
        std::cout << events_.size() << std::endl ;
        return true;
    }


    bool DvDatasetStreamer::events(dv::EventStore &_events){


    } 

    bool DvDatasetStreamer::image(cv::Mat &_image){


    }    
}
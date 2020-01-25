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

#include "dvsal/blocks/BlockDvImageVisualizer.h"

namespace dvsal{

    BlockDvImageVisualizer::BlockDvImageVisualizer(){
        createPolicy({{"DVS Events","v-event"}});

        registerCallback({"DVS Events"}, 
                            [&](flow::DataFlow _data){
                                if(idle_){
                                    idle_ = false;  
                                        
                                    dv::EventStore events = _data.get<dv::EventStore>("DVS Events");
                                    cv::Mat fakeImage = convertEventsToCVMat(events);
 
                                    cv::imshow("events",fakeImage);
                                    cv::waitKey(1);

                                    idle_ = true;
                                }

                            }
                        );
    }

    cv::Mat BlockDvImageVisualizer::convertEventsToCVMat(dv::EventStore _events){
            cv::Mat image = cv::Mat::zeros(cv::Size(240,180), CV_8UC3);
            for (const auto &event : _events) {
                if (event.polarity())
                    image.at<cv::Vec3b>(event.y(), event.x()) = cv::Vec3b(0,0,255);
                else
                    image.at<cv::Vec3b>(event.y(), event.x()) = cv::Vec3b(0,255,0);
            }

            return image;
        };        
}
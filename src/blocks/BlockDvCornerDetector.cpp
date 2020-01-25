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

#include "dvsal/blocks/BlockDvCornerDetector.h"

namespace dvsal{

    BlockDvCornerDetector::BlockDvCornerDetector(){
        createPipe("Corners events", "v_events");
        
        createPolicy({{"Unfiltered events", "v_events"}});
        registerCallback({"Unfiltered events"}, 
                                [&](flow::DataFlow _data){
                                    if(idle_){
                                        idle_ = false;
                                        auto UnfiltEvents = _data.get<dv::EventStore>("Unfiltered events");
                                        detector_->eventCallback(UnfiltEvents);
                                        dv::EventStore corners = detector_->cornersDetected();
                                        
                                        if(corners.size() > 0){
                                            getPipe("Corners events")->flush(corners);
                                        }
                                        idle_ = true;
                                    }
                                }
        );


    }

    bool BlockDvCornerDetector::configure(std::unordered_map<std::string, std::string> _params){
        std::string detecMethod;
        for(auto &param: _params){
            if(param.second == "")
                return false;

            if(param.first == "Detection_method"){
                detecMethod = param.second;
            }
        }
        if (detecMethod == "harris"){
            detector_ = new HarrisDetector;
        }
        else if (detecMethod == "fast"){
            detector_ = new FastDetector;
        }else{
            return false;
        }

        return true;
    }
    
    std::vector<std::string> BlockDvCornerDetector::parameters(){
        return {"Detection_method"};
    }
}

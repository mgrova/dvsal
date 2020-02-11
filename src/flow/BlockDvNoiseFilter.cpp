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

#include "dvsal/flow/BlockDvNoiseFilter.h"

namespace dvsal{

    BlockDvNoiseFilter::BlockDvNoiseFilter(){

        dvsNoiseFilter_ = new libcaer::filters::DVSNoise(240,180); // 666

        createPipe("filtered events", "v_event");
        
        createPolicy({{"Unfiltered events", "v_event"}});
        registerCallback({"Unfiltered events"}, 
                                [&](flow::DataFlow _data){
                                    if(idle_){
                                        idle_ = false;
                                        auto UnfiltEvents = _data.get<dv::EventStore>("Unfiltered events");;

                                        dv::EventStore filtered;
                                        if(filterEvents(UnfiltEvents , filtered)){
                                            getPipe("Corners events")->flush(filtered);
                                        }
                                        idle_ = true;
                                    }
                                }
        );
    }

    bool BlockDvNoiseFilter::filterEvents(dv::EventStore _events, dv::EventStore &_filteredEvents){
        // Convert dv::EventStore to libcaer::events::PolarityEventPacket or caerPolarityEventPacket
        caerPolarityEventPacket caerEvents;


        dvsNoiseFilter_->apply(caerEvents);
        // Convert caer to dv::EventStore
        // _filteredEvents = ;

        if (_filteredEvents.size() < 5){
            return false;
        }

        return true;
    }

}
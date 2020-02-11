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

        dvsNoiseFilter_ = new libcaer::filters::DVSNoise(240,240); // 666

        createPipe("filtered events", "v_event");
        
        createPolicy({{"Unfiltered events", "v_event"}});
        registerCallback({"Unfiltered events"}, 
                                [&](flow::DataFlow _data){
                                    if(idle_){
                                        idle_ = false;
                                        auto UnfiltEvents = _data.get<dv::EventStore>("Unfiltered events");
                                        
                                        dv::EventStore filtered;
                                        if(filterEvents(UnfiltEvents , filtered)){
                                            getPipe("filtered events")->flush(filtered);
                                        }
                                        idle_ = true;
                                    }
                                }
        );
    }

    std::vector<std::string> BlockDvNoiseFilter::parameters(){
        return {
            "enable_two_levels" , "enable_check_pol" , "support_min" , "support_max" , "time_window"
        };
    }

    bool BlockDvNoiseFilter::configure(std::unordered_map<std::string, std::string> _params){
        if(isRunningLoop()) // Cant configure if already running.
            return false;   

        std::string level  = _params["enable_two_levels"];
        twoLevels_ = (level == "true") ? true : false;
        std::string checkPol = _params["enable_check_pol"];
        checkPol_ = (checkPol == "true") ? true : false;
        supportMin_ = std::stoi(_params["support_min"]);
        supportMax_ = std::stoi(_params["support_max"]);
        actTime_    = std::stoi(_params["time_window"]);


        dvsNoiseFilter_->configSet(CAER_FILTER_DVS_BACKGROUND_ACTIVITY_TWO_LEVELS, twoLevels_);
	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_BACKGROUND_ACTIVITY_CHECK_POLARITY, checkPol_);
	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_BACKGROUND_ACTIVITY_SUPPORT_MIN, supportMin_);
	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_BACKGROUND_ACTIVITY_SUPPORT_MAX, supportMax_);
	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_BACKGROUND_ACTIVITY_TIME, actTime_);
	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_BACKGROUND_ACTIVITY_ENABLE, true);

	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_REFRACTORY_PERIOD_TIME, 200);
	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_REFRACTORY_PERIOD_ENABLE, true);

	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_HOTPIXEL_ENABLE, true);
	    dvsNoiseFilter_->configSet(CAER_FILTER_DVS_HOTPIXEL_LEARN, true);

        return true;
    }

    

    bool BlockDvNoiseFilter::filterEvents(dv::EventStore _events, dv::EventStore &_filteredEvents){

        caerPolarityEventPacket caerEvents = eventStoreToCAER(_events);
        dvsNoiseFilter_->apply(caerEvents);
        _filteredEvents = CAERToEventStore(caerEvents,caerEvents->packetHeader.eventNumber); // 666
        if (_filteredEvents.size() == _events.size()){
            std::cout << "Error filtering noise \n";
            return false;
        }

        return true;
    }

    caerPolarityEventPacket BlockDvNoiseFilter::eventStoreToCAER(dv::EventStore _sortEvStore){
        caerPolarityEventPacket pPolPack;
        pPolPack =  caerPolarityEventPacketAllocate(_sortEvStore.size(),0,0);
        
        u_int32_t index = 0;
        for (auto event : _sortEvStore){
            caerPolarityEventSetPolarity(&pPolPack->events[index]  , event.polarity());
            caerPolarityEventSetTimestamp(&pPolPack->events[index] , event.timestamp());
            caerPolarityEventSetX(&pPolPack->events[index]         , event.x());
            caerPolarityEventSetY(&pPolPack->events[index]         , event.y());
            caerPolarityEventValidate(&pPolPack->events[index]     , pPolPack);

            index++;
        }

        return pPolPack;
    }

    dv::EventStore BlockDvNoiseFilter::CAERToEventStore(caerPolarityEventPacket _pPolPack,u_int32_t _packetSize){
        dv::EventStore dvEvents;
        for(u_int32_t i=0;i<_packetSize;i++){

            dv::Event tmpEv(caerPolarityEventGetTimestamp(&_pPolPack->events[i]) , 
                            caerPolarityEventGetX(&_pPolPack->events[i]) , caerPolarityEventGetY(&_pPolPack->events[i]) , 
                            caerPolarityEventGetPolarity(&_pPolPack->events[i]));
            dvEvents.add(tmpEv);
        }
        return dvEvents;
    }
        

}
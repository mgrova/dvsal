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


#include "dvsal/streamers/DvCameraDVS128Streamer.h"

namespace dvsal{

    bool DvCameraDVS128Streamer::init(const std::string &_string){
        // Open a DVS128, give it a device ID of 1, and don't care about USB bus or SN restrictions.
        dvs128Handle_ = new libcaer::devices::dvs128(1, 0, 0, "");

        // Let's take a look at the information we have on the device.
	    struct caer_dvs128_info dvs128_info = dvs128Handle_->infoGet();

	    printf("%s --- ID: %d, Master: %d, DVS X: %d, DVS Y: %d, Firmware: %d.\n", dvs128_info.deviceString,
	    	dvs128_info.deviceID, dvs128_info.deviceIsMaster, dvs128_info.dvsSizeX, dvs128_info.dvsSizeY,
	    	dvs128_info.firmwareVersion);
        
        // Send the default configuration before using the device.
	    // No configuration is sent automatically!
	    dvs128Handle_->sendDefaultConfig();

        // Tweak some biases, to increase bandwidth in this case.
	    dvs128Handle_->configSet(DVS128_CONFIG_BIAS, DVS128_CONFIG_BIAS_PR, 695);
	    dvs128Handle_->configSet(DVS128_CONFIG_BIAS, DVS128_CONFIG_BIAS_FOLL, 867);

	    // Let's verify they really changed!
	    uint32_t prBias   = dvs128Handle_->configGet(DVS128_CONFIG_BIAS, DVS128_CONFIG_BIAS_PR);
	    uint32_t follBias = dvs128Handle_->configGet(DVS128_CONFIG_BIAS, DVS128_CONFIG_BIAS_FOLL);

    	printf("New bias values --- PR: %d, FOLL: %d.\n", prBias, follBias);

        // Now let's get start getting some data from the device. We just loop in blocking mode,
	    // no notification needed regarding new events. The shutdown notification, for example if
	    // the device is disconnected, should be listened to.
	    dvs128Handle_->dataStart(nullptr, nullptr, nullptr, &usbShutdownHandler, nullptr);

	    // Let's turn on blocking data-get mode to avoid wasting resources.
	    dvs128Handle_->configSet(CAER_HOST_CONFIG_DATAEXCHANGE, CAER_HOST_CONFIG_DATAEXCHANGE_BLOCKING, true);

        return false;
    }

    bool DvCameraDVS128Streamer::step(){

        return false;
    }

    bool DvCameraDVS128Streamer::events(dv::EventStore &_events){

        return false;
    }    

    bool DvCameraDVS128Streamer::image(cv::Mat &_image){


        return false;
    }   

    bool DvCameraDVS128Streamer::cutUsingTime(int _microseconds){

        return false;
    }

    void DvCameraDVS128Streamer::usbShutdownHandler(void *_ptr){
        (void) (_ptr); // UNUSED.

	    // globalShutdown_.store(true);

        return;
    }

}
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

#include <dvsal/streamers/CameraDavisStreamer.h>

namespace dvsal{

    bool CameraDavisStreamer::init(const std::string &_string){
        // Open a DVS128, give it a device ID of 1, and don't care about USB bus or SN restrictions.
        davisHandle_ = new libcaer::devices::davis(1);

        // Let's take a look at the information we have on the device.
	    struct caer_davis_info davis_info = davisHandle_->infoGet();

	    printf("%s --- ID: %d, Master: %d, DVS X: %d, DVS Y: %d, Firmware: %d.\n", davis_info.deviceString,
	    	davis_info.deviceID, davis_info.deviceIsMaster, davis_info.dvsSizeX, davis_info.dvsSizeY,
	    	davis_info.firmwareVersion);
        
        // Send the default configuration before using the device.
	    // No configuration is sent automatically!
	    davisHandle_->sendDefaultConfig();

        // Tweak some biases, to increase bandwidth in this case.
		struct caer_bias_coarsefine coarseFineBias;

		coarseFineBias.coarseValue        = 2;
		coarseFineBias.fineValue          = 116;
		coarseFineBias.enabled            = true;
		coarseFineBias.sexN               = false;
		coarseFineBias.typeNormal         = true;
		coarseFineBias.currentLevelNormal = true;

		davisHandle_->configSet(DAVIS_CONFIG_BIAS, DAVIS240_CONFIG_BIAS_PRBP, caerBiasCoarseFineGenerate(coarseFineBias));

		coarseFineBias.coarseValue        = 1;
		coarseFineBias.fineValue          = 33;
		coarseFineBias.enabled            = true;
		coarseFineBias.sexN               = false;
		coarseFineBias.typeNormal         = true;
		coarseFineBias.currentLevelNormal = true;

		davisHandle_->configSet(DAVIS_CONFIG_BIAS, DAVIS240_CONFIG_BIAS_PRSFBP, caerBiasCoarseFineGenerate(coarseFineBias));

		// Let's verify they really changed!
		uint32_t prBias   = davisHandle_->configGet(DAVIS_CONFIG_BIAS, DAVIS240_CONFIG_BIAS_PRBP);
		uint32_t prsfBias = davisHandle_->configGet(DAVIS_CONFIG_BIAS, DAVIS240_CONFIG_BIAS_PRSFBP);

		printf("New bias values --- PR-coarse: %d, PR-fine: %d, PRSF-coarse: %d, PRSF-fine: %d.\n",
			caerBiasCoarseFineParse(prBias).coarseValue, caerBiasCoarseFineParse(prBias).fineValue,
			caerBiasCoarseFineParse(prsfBias).coarseValue, caerBiasCoarseFineParse(prsfBias).fineValue);


        // Now let's get start getting some data from the device. We just loop in blocking mode,
	    // no notification needed regarding new events. The shutdown notification, for example if
	    // the device is disconnected, should be listened to.
	    davisHandle_->dataStart(nullptr, nullptr, nullptr, &usbShutdownHandler, nullptr);

	    // Let's turn on blocking data-get mode to avoid wasting resources.
	    davisHandle_->configSet(CAER_HOST_CONFIG_DATAEXCHANGE, CAER_HOST_CONFIG_DATAEXCHANGE_BLOCKING, true);

        return false;
    }

    bool CameraDavisStreamer::step(){
        if (!globalShutdown_.load(std::memory_order_relaxed)) {
		    std::unique_ptr<libcaer::events::EventPacketContainer> packetContainer = davisHandle_->dataGet();
		    if (packetContainer == nullptr) {
		    	// continue; // Skip if nothing there.
		    }
            printf("\nGot event container with %d packets (allocated).\n", packetContainer->size());

		    for (auto &packet : *packetContainer) {
			    if (packet == nullptr) {
				    printf("Packet is empty (not present).\n");
				    continue; // Skip if nothing there.
			    }

			    printf("Packet of type %d -> %d events, %d capacity.\n", packet->getEventType(), packet->getEventNumber(),
				    packet->getEventCapacity());

			    if (packet->getEventType() == POLARITY_EVENT) {
				    std::shared_ptr<const libcaer::events::PolarityEventPacket> polarity
				    	= std::static_pointer_cast<libcaer::events::PolarityEventPacket>(packet);

				    // Get full timestamp and addresses of first event.
				    const libcaer::events::PolarityEvent &firstEvent = (*polarity)[0];

			    	int32_t ts = firstEvent.getTimestamp();
			    	uint16_t x = firstEvent.getX();
			    	uint16_t y = firstEvent.getY();
			    	bool pol   = firstEvent.getPolarity();

			    	printf("First polarity event - ts: %d, x: %d, y: %d, pol: %d.\n", ts, x, y, pol);

                    dv::Event event(static_cast<int64_t>(ts) , static_cast<int16_t>(x) , static_cast<int16_t>(y) , static_cast<uint8_t>(pol));
        
                    events_.add(event);
			    }

				if (packet->getEventType() == FRAME_EVENT) {
					std::shared_ptr<const libcaer::events::FrameEventPacket> frame
						= std::static_pointer_cast<libcaer::events::FrameEventPacket>(packet);

					// Get full timestamp, and sum all pixels of first frame event.
					const libcaer::events::FrameEvent &firstEvent = (*frame)[0];

					int32_t ts   = firstEvent.getTimestamp();
					uint64_t sum = 0;

					for (int32_t y = 0; y < firstEvent.getLengthY(); y++)
						for (int32_t x = 0; x < firstEvent.getLengthX(); x++)
							sum += firstEvent.getPixel(x, y);
							
				}
		    }
        }


    return true;
    }

    bool CameraDavisStreamer::events(dv::EventStore &_events){
        _events = events_;

        return true;
    }    

    bool CameraDavisStreamer::image(cv::Mat &_image){


        return false;
    }   

    bool CameraDavisStreamer::cutUsingTime(int _microseconds){

        events_ = events_.sliceTime(_microseconds);
        return true;
    }

    void CameraDavisStreamer::usbShutdownHandler(void *_ptr){
        (void) (_ptr); // UNUSED.

        // 666 IMPORTANT
	    // globalShutdown_.store(true);
        return;
    }

}
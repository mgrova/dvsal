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


#include "dv_mico/streamers/DvStreamer.h"
#include "dv_mico/streamers/DvDatasetStreamer.h"
#include "dv_mico/streamers/DvCameraStreamer.h"

namespace dv_mico{

    DvStreamer * DvStreamer::create(eModel _type) {
		if (_type == eModel::dataset) {
			return new DvDatasetStreamer();
		}else if (_type == eModel::dvs128) {
			return new DvCameraStreamer();
        }else {
            std::cerr << "[DV-STREAMER]  unknown model type" << std::endl;
			return nullptr;
        }
    }
    
    DvStreamer * DvStreamer::create(std::string _type) {
		if (_type == "dataset") {
			return new DvDatasetStreamer();
		}else if (_type == "dvs128") {
			return new DvCameraStreamer();
        }else {
            std::cerr << "[DV-STREAMER]  unknown model type" << std::endl;
			return nullptr;
        }
    }


}
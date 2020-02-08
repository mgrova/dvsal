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

#ifndef DVSAL_FLOW_H_
#define DVSAL_FLOW_H_

#include <flow/flow.h>

#include "dvsal/flow/BlockDvImageVisualizer.h"
#include "dvsal/flow/BlockDvCornerDetector.h"
#include "dvsal/flow/BlockDvDatasetStreamer.h"

namespace dvsal{

    extern "C" flow::PluginNodeCreator* factory(){
        flow::PluginNodeCreator *creator = new flow::PluginNodeCreator;

        creator->registerNodeCreator([](){ return std::make_unique<flow::FlowVisualBlock<BlockDvDatasetStreamer,true>>(); }, "DVS");
        creator->registerNodeCreator([](){ return std::make_unique<flow::FlowVisualBlock<BlockDvCornerDetector>>();       }, "DVS");
        creator->registerNodeCreator([](){ return std::make_unique<flow::FlowVisualBlock<BlockDvImageVisualizer>>();      }, "DVS");

        return creator;
    }

    
}

#endif
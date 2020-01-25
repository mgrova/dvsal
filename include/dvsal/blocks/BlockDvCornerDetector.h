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

#ifndef BLOCK_DV_CORNERDETECTOR_H_
#define BLOCK_DV_CORNERDETECTOR_H_

#include <flow/flow.h>

#include <dvsal/processors/corner_detectors/Detector.h>
#include <dvsal/processors/corner_detectors/HarrisDetector.h>
#include <dvsal/processors/corner_detectors/FastDetector.h>

namespace dvsal{

    class BlockDvCornerDetector: public flow::Block{
    public:
        std::string name() override {return "DV Corner Detector";};
        std::string description() const override {return "Flow wrapper of DVS Corners Detector";};


        BlockDvCornerDetector();
    
        virtual bool configure(std::unordered_map<std::string, std::string> _params) override;
        std::vector<std::string> parameters() override;
        
    private:
        

    private:
        Detector* detector_ = nullptr;
        bool idle_ = true;
    };
}

// extern "C" flow::PluginNodeCreator* factory(){
//     flow::PluginNodeCreator *creator = new flow::PluginNodeCreator([](){ return std::make_unique<flow::FlowVisualBlock<dvsal::BlockDvCornerDetector>>(); });

//     return creator;
// }

#endif
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

#ifndef BLOCK_DV_CAMERA_DVS128_STREAMER_H_
#define BLOCK_DV_CAMERA_DVS128_STREAMER_H_

#include <flow/flow.h>
#include "dvsal/streamers/DvStreamer.h"


namespace dvsal{

    class BlockDvCameraDVS128Streamer : public flow::Block{
    public:
        std::string name() const override {return "DV DVS128 Streamer";};
        std::string description() const override {return "Flow wrapper of Camera DVS128";};

        BlockDvCameraDVS128Streamer();

        virtual bool configure(std::unordered_map<std::string, std::string> _params) override;
        std::vector<std::string> parameters() override;

        virtual QWidget * customWidget() override;

    protected:
        virtual void loopCallback() override;


    private:
        DvStreamer *streamer_;

        int eventRate_ = 200; // events batch output size
        int64_t lastHighest_ = 0;
    };
}

#endif
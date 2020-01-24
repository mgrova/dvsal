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

#ifndef DV_STREAMER_H_
#define DV_STREAMER_H_

#include <string>
#include <iostream>

#include <dv-sdk/processing.hpp>
#include <dv-sdk/config.hpp>

#include <opencv2/opencv.hpp>

namespace dv_mico{

    class DvStreamer{
    public:
      enum class eModel { dataset , dvs128 };

		  static DvStreamer *create(eModel _type);
		  static DvStreamer *create(std::string _type);

    public:
		  virtual bool init(const std::string &_string = "") = 0;
		  virtual bool events(dv::Event _event) = 0;

    };

    
}

#endif
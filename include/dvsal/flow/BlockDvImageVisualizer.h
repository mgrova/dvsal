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

#ifndef BLOCK_DV_IMAGE_VISUALIZER_H_
#define BLOCK_DV_IMAGE_VISUALIZER_H_

#include <flow/flow.h>
#include <opencv2/opencv.hpp>

#include <dv-sdk/processing.hpp>
#include <dv-sdk/config.hpp>
#include <dv-sdk/utils.h>

#include <vtkJPEGReader.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h> // Note: this is a 2D mapper (cf. vtkImageActor which is 3D)
#include <vtkActor2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

namespace dvsal{

    class BlockDvImageVisualizer: public flow::Block{
    public:
        std::string name() const override {return "DV Image Visualizer";};
        std::string description() const override {return "Flow wrapper of DVS Image Visualizer";};

        BlockDvImageVisualizer();
        
    private:
        vtkSmartPointer<vtkImageData> convertCVMatToVtkImageData(const cv::Mat &sourceCVImage, bool flipOverXAxis);
        cv::Mat convertEventsToCVMat(dv::EventStore _events);      

    private:
        bool idle_ = true;

        vtkSmartPointer<vtkImageMapper> mapper_;
        vtkSmartPointer<vtkActor2D> image_;
        vtkSmartPointer<vtkRenderer> renderer_;
        vtkSmartPointer<vtkRenderWindow> window_;
    };

    
}

#endif
#ifndef __itkLabelSetDilateImageFilter_txx
#define __itkLabelSetDilateImageFilter_txx

#include "itkLabelSetDilateImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"


#include "itkImageLinearIteratorWithIndex.h"
#include "itkImageLinearConstIteratorWithIndex.h"

#include "itkLabelSetUtils.h"


namespace itk
{

template <typename TInputImage, typename TOutputImage>
void
LabelSetDilateImageFilter<TInputImage, TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId )
{
  // this is where the work happens. We use a distance image with
  // floating point pixel to perform the parabolic operations. The
  // input image specifies the size of the SE at each location. These
  // values need to be squared on the way in to the parabolic
  // operations, and the squaring step needs to be integrated with the
  // line copy.
  // Similarly, the thresholding on output needs to be integrated
  // with the last processing stage.

  // compute the number of rows first, so we can setup a progress reporter
  typename std::vector< unsigned int > NumberOfRows;
  InputSizeType   size   = outputRegionForThread.GetSize();

  for (unsigned int i = 0; i < InputImageDimension; i++)
    {
    NumberOfRows.push_back( 1 );
    for (unsigned int d = 0; d < InputImageDimension; d++)
      {
      if( d != i )
        {
        NumberOfRows[i] *= size[ d ];
        }
      }
    }
  float progressPerDimension = 1.0/ImageDimension;

  ProgressReporter * progress = new ProgressReporter(this, threadId, NumberOfRows[this->m_CurrentDimension], 30, this->m_CurrentDimension * progressPerDimension, progressPerDimension);


  typedef ImageLinearConstIteratorWithIndex< TInputImage  >  InputConstIteratorType;
  typedef ImageLinearIteratorWithIndex< TOutputImage >  OutputIteratorType;

  typedef ImageLinearConstIteratorWithIndex<DistanceImageType> InputDistIteratorType;
  typedef ImageLinearIteratorWithIndex<DistanceImageType> OutputDistIteratorType;

  // for stages after the first
  typedef ImageLinearConstIteratorWithIndex< TOutputImage  >  OutputConstIteratorType;


  typedef ImageRegion< TInputImage::ImageDimension > RegionType;

  typename TInputImage::ConstPointer   inputImage(    this->GetInput ()   );
  typename TOutputImage::Pointer       outputImage(   this->GetOutput()        );

  outputImage->SetBufferedRegion( outputImage->GetRequestedRegion() );
  outputImage->Allocate();
  RegionType region = outputRegionForThread;

  InputConstIteratorType  inputIterator(  inputImage,  region );
  InputConstIteratorType  inputIteratorStage2(  outputImage,  region );
  OutputIteratorType      outputIterator( outputImage, region );
  //OutputConstIteratorType inputIteratorStage2( outputImage, region );

  InputDistIteratorType inputDistIterator(this->m_DistanceImage, region);
  OutputDistIteratorType outputDistIterator(this->m_DistanceImage, region);

  // setup the progress reporting
  // deal with the first dimension - this should be copied to the
  // output if the scale is 0

  // flag to indicate whether the internal distance image has been
  // initialized using the special first pass erosion
  if (this->m_Scale[this->m_CurrentDimension] > 0)
    {
    // Perform as normal
    //RealType magnitude = 1.0/(2.0 * m_Scale[0]);
    unsigned long LineLength = region.GetSize()[this->m_CurrentDimension];
    RealType image_scale = this->GetInput()->GetSpacing()[this->m_CurrentDimension];
    //bool lastpass = (m_CurrentDimension == ImageDimension - 1);

    if (!this->m_FirstPassDone)
      {
      LabSet::doOneDimensionDilateFirstPass<InputConstIteratorType,OutputDistIteratorType, OutputIteratorType,
                                            RealType>(inputIterator, outputDistIterator, outputIterator,
                                                      *progress, LineLength,
                                                      this->m_CurrentDimension,
                                                      this->m_MagnitudeSign,
                                                      this->m_UseImageSpacing,
                                                      this->m_Extreme,
                                                      image_scale,
                                                      this->m_Scale[this->m_CurrentDimension]);
      }
    else
      {
      LabSet::doOneDimensionDilate<InputConstIteratorType,
                                   InputDistIteratorType,
                                   OutputIteratorType,
                                   OutputDistIteratorType,
                                   RealType>(inputIteratorStage2,
                                             inputDistIterator,
                                             outputDistIterator,
                                             outputIterator,
                                             *progress, LineLength,
                                             this->m_CurrentDimension,
                                             this->m_MagnitudeSign,
                                             this->m_UseImageSpacing,
                                             this->m_Extreme,
                                             image_scale,
                                             this->m_Scale[this->m_CurrentDimension]);
      }
    }
}


} // namespace itk
#endif

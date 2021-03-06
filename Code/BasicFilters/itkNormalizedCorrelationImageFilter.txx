/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkNormalizedCorrelationImageFilter.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkNormalizedCorrelationImageFilter_txx
#define __itkNormalizedCorrelationImageFilter_txx

#include "itkNormalizedCorrelationImageFilter.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkProgressReporter.h"

namespace itk
{
template< class TInputImage, class TMaskImage, class TOutputImage, class TOperatorValueType >
void
NormalizedCorrelationImageFilter< TInputImage, TMaskImage, TOutputImage, TOperatorValueType >
::SetMaskImage(const TMaskImage *mask)
{
  this->ProcessObject::SetNthInput( 1, const_cast< TMaskImage * >( mask ) );
}

template< class TInputImage, class TMaskImage, class TOutputImage, class TOperatorValueType >
const TMaskImage *
NormalizedCorrelationImageFilter< TInputImage, TMaskImage, TOutputImage, TOperatorValueType >
::GetMaskImage() const
{
  return static_cast< MaskImageType * >( const_cast< DataObject * >( this->ProcessObject::GetInput(1) ) );
}

template< class TInputImage, class TMaskImage, class TOutputImage, class TOperatorValueType >
void
NormalizedCorrelationImageFilter< TInputImage, TMaskImage, TOutputImage, TOperatorValueType >
::GenerateInputRequestedRegion()
throw ( InvalidRequestedRegionError )
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  // Superclass handled the input image, now we just need to handle
  // the mask image is any.
  InputImagePointer inputPtr =
    const_cast< TInputImage * >( this->GetInput() );
  MaskImagePointer maskPtr =
    const_cast< TMaskImage * >( this->GetMaskImage() );

  if ( !inputPtr || !maskPtr )
    {
    return;
    }

  // get a copy of the input requested region which was set up by the
  // superclass (NeighborhoodOperatorImageFilter)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();

  // set the mask requested region to match the input requested region
  if ( maskPtr->GetLargestPossibleRegion().IsInside(inputRequestedRegion) )
    {
    maskPtr->SetRequestedRegion(inputRequestedRegion);
    }
  else
    {
    // Couldn't make the masked region match the input region.
    // Throw an exception.

    // store what we tried to request
    maskPtr->SetRequestedRegion(inputRequestedRegion);

    // build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region of the mask image.");
    e.SetDataObject(maskPtr);
    throw e;
    }
}

template< class TInputImage, class TMaskImage, class TOutputImage, class TOperatorValueType >
void
NormalizedCorrelationImageFilter< TInputImage, TMaskImage, TOutputImage, TOperatorValueType >
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       int threadId)
{
  // This filter can only operate on data types that are signed.
  if ( !NumericTraits< OutputPixelType >::is_signed )
    {
    itkExceptionMacro(<< "This filter can only create images of signed data type.");
    }

  // Normalize the template in a local variable. This will simplify
  // the calculations later.
  //
  typedef Neighborhood< ITK_TYPENAME NumericTraits< OperatorValueType >::RealType,
                        ImageDimension >                      NormalizedTemplateType;
  typedef typename NumericTraits< OutputPixelType >::RealType OutputPixelRealType;

  NormalizedTemplateType normalizedTemplate;
  normalizedTemplate.SetRadius( this->GetOperator().GetRadius() );

  typename NormalizedTemplateType::Iterator ntIt;
  typename OutputNeighborhoodType::ConstIterator tIt;

  OutputPixelRealType sum = 0.0;
  OutputPixelRealType sumOfSquares = 0.0;
  for ( tIt = this->GetOperator().Begin(); tIt < this->GetOperator().End(); ++tIt )
    {
    sum += ( *tIt );
    sumOfSquares += ( ( *tIt ) * ( *tIt ) );
    }
  OutputPixelRealType num =
    static_cast< OutputPixelRealType >( this->GetOperator().Size() );
  OutputPixelRealType mean = sum / num;
  OutputPixelRealType var = ( sumOfSquares - ( sum * sum / num ) ) / ( num - 1.0 );
  OutputPixelRealType std = vcl_sqrt(var);

  // convert std to a scaling factor k such that
  //
  //        || (coeff - mean) / k || = 1.0
  //
  double k = std * vcl_sqrt(num - 1.0);

  // normalize the template
  for ( ntIt = normalizedTemplate.Begin(), tIt = this->GetOperator().Begin();
        ntIt < normalizedTemplate.End(); ++ntIt, ++tIt )
    {
    *ntIt = ( *tIt - mean ) / k;
    }

  // get output/inputs
  OutputImageType *     output = this->GetOutput();
  const InputImageType *input = this->GetInput();
  const MaskImageType * mask = this->GetMaskImage();

  // Break the input into a series of regions.  The first region is free
  // of boundary conditions, the rest with boundary conditions. Note,
  // we pass in the input image and the OUTPUT requested region. We are
  // only concerned with centering the neighborhood operator at the
  // pixels that correspond to output pixels.
  typedef NeighborhoodAlgorithm::ImageBoundaryFacesCalculator< InputImageType >
  BFC;

  typedef typename BFC::FaceListType FaceListType;
  BFC          faceCalculator;
  FaceListType faceList;

  faceList = faceCalculator( input, outputRegionForThread,
                             this->GetOperator().GetRadius() );

  // support progress methods/callbacks
  ProgressReporter progress( this, threadId,
                             outputRegionForThread.GetNumberOfPixels() );

  // Process non-boundary region and each of the boundary faces.
  // These are N-d regions which border the edge of the buffer.
  ConstNeighborhoodIterator< InputImageType > bit;
  typename FaceListType::iterator fit;
  ImageRegionIterator< OutputImageType >    it;
  ImageRegionConstIterator< MaskImageType > mit;
  unsigned int                              i;
  unsigned int                              templateSize = normalizedTemplate.Size();
  OutputPixelRealType                       realTemplateSize;
  OutputPixelRealType                       value;
  OutputPixelRealType                       numerator;
  OutputPixelRealType                       denominator;
  OutputPixelRealType                       zero = NumericTraits< OutputPixelType >::Zero;

  realTemplateSize = static_cast< OutputPixelRealType >( templateSize );
  for ( fit = faceList.begin(); fit != faceList.end(); ++fit )
    {
    bit =
      ConstNeighborhoodIterator< InputImageType >(normalizedTemplate.GetRadius(),
                                                  input, *fit);
    bit.OverrideBoundaryCondition( this->GetBoundaryCondition() );
    bit.GoToBegin();

    it = ImageRegionIterator< OutputImageType >(output, *fit);

    if ( !mask )
      {
      // No mask is defined
      while ( !bit.IsAtEnd() )
        {
        // Compute the normalized correlation at this pixel.  The
        // template has already been normalized to mean zero and norm 1.
        // This simplifies the calculation to being just the correlation
        // of the image neighborhod with the template, normalized by a
        // function of the image neighborhood.
        sum = 0.0;
        sumOfSquares = 0.0;
        numerator = 0.0;
        for ( i = 0; i < templateSize; ++i )
          {
          value = static_cast< OutputPixelRealType >( bit.GetPixel(i) );

          numerator += ( value * normalizedTemplate[i] );

          // tally values for normalizing the result
          sum += value;
          sumOfSquares += ( value * value );
          }
        denominator = vcl_sqrt( sumOfSquares - ( sum * sum / realTemplateSize ) );

        it.Value() = numerator / denominator;

        ++bit;
        ++it;
        progress.CompletedPixel();
        }
      }
    else
      {
      // Mask is defined, use the same calculation as above but only
      // perform it under the mask
      mit = ImageRegionConstIterator< MaskImageType >(mask, *fit);
      while ( !bit.IsAtEnd() )
        {
        if ( mit.Get() )
          {
          // Compute the normalized correlation at this pixel.  The
          // template has already been normalized to mean zero and norm 1.
          // This simplifies the calculation to being just the correlation
          // of the image neighborhod with the template, normalized by a
          // function of the image neighborhood.
          sum = 0.0;
          sumOfSquares = 0.0;
          numerator = 0.0;
          for ( i = 0; i < templateSize; ++i )
            {
            value = static_cast< OutputPixelRealType >( bit.GetPixel(i) );

            numerator += ( value * normalizedTemplate[i] );

            // tally values for normalizing the result
            sum += value;
            sumOfSquares += ( value * value );
            }
          denominator = vcl_sqrt( sumOfSquares - ( sum * sum / realTemplateSize ) );

          it.Value() = numerator / denominator;
          }
        else
          {
          // Not under the mask.  Set the normalized correlation to zero.
          it.Value() = zero;
          }
        ++bit;
        ++it;
        ++mit;
        progress.CompletedPixel();
        }
      }
    }
}
} // end namespace itk

#endif

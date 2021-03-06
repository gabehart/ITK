/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkAbsoluteValueDifferenceImageFilter.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkAbsoluteValueDifferenceImageFilter_h
#define __itkAbsoluteValueDifferenceImageFilter_h

#include "itkBinaryFunctorImageFilter.h"
#include "itkConceptChecking.h"

namespace itk
{
/** \class AbsoluteValueDifferenceImageFilter
 * \brief Implements pixel-wise the computation of absolute value difference.
 *
 * This filter is parametrized over the types of the two
 * input images and the type of the output image.
 *
 * Numeric conversions (castings) are done by the C++ defaults.
 *
 * The filter will walk over all the pixels in the two input images, and for
 * each one of them it will do the following:
 *
 * - cast the input 1 pixel value to \c double
 * - cast the input 2 pixel value to \c double
 * - compute the difference of the two pixel values
 * - compute the absolute value of the difference
 * - cast the \c double value resulting from \c the absolute value to the pixel type of the output image
 * - store the casted value into the output image.
 *
 * The filter expect all images to have the same dimension
 * (e.g. all 2D, or all 3D, or all ND)
 *
 * \ingroup IntensityImageFilters Multithreaded
 */
namespace Functor
{
template< class TInput1, class TInput2, class TOutput >
class AbsoluteValueDifference2
{
public:
  AbsoluteValueDifference2() {}
  ~AbsoluteValueDifference2() {}
  bool operator!=(const AbsoluteValueDifference2 &) const
  {
    return false;
  }

  bool operator==(const AbsoluteValueDifference2 & other) const
  {
    return !( *this != other );
  }

  inline TOutput operator()(const TInput1 & A,
                            const TInput2 & B) const
  {
    const double dA = static_cast< double >( A );
    const double dB = static_cast< double >( B );
    const double diff = dA - dB;
    const double absdiff = ( diff > 0.0 ) ? diff : -diff;

    return static_cast< TOutput >( absdiff );
  }
};
}

template< class TInputImage1, class TInputImage2, class TOutputImage >
class ITK_EXPORT AbsoluteValueDifferenceImageFilter:
  public
  BinaryFunctorImageFilter< TInputImage1, TInputImage2, TOutputImage,
                            Functor::AbsoluteValueDifference2<
                              typename TInputImage1::PixelType,
                              typename TInputImage2::PixelType,
                              typename TOutputImage::PixelType >   >
{
public:
  /** Standard class typedefs. */
  typedef AbsoluteValueDifferenceImageFilter Self;
  typedef BinaryFunctorImageFilter< TInputImage1, TInputImage2, TOutputImage,
                                    Functor::AbsoluteValueDifference2<
                                      typename TInputImage1::PixelType,
                                      typename TInputImage2::PixelType,
                                      typename TOutputImage::PixelType >
                                    >  Superclass;

  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(AbsoluteValueDifferenceImageFilter,
               BinaryFunctorImageFilter);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( Input1CovertibleToDoubleCheck,
                   ( Concept::Convertible< typename TInputImage1::PixelType, double > ) );
  itkConceptMacro( Input2ConvertibleToDoubleCheck,
                   ( Concept::Convertible< typename TInputImage2::PixelType, double > ) );
  itkConceptMacro( DoubleCovertibleToOutputCheck,
                   ( Concept::Convertible< double, typename TOutputImage::PixelType > ) );
  /** End concept checking */
#endif
protected:
  AbsoluteValueDifferenceImageFilter() {}
  virtual ~AbsoluteValueDifferenceImageFilter() {}
private:
  AbsoluteValueDifferenceImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);                     //purposely not implemented
};
} // end namespace itk

#endif

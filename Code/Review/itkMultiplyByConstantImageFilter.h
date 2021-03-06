/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkMultiplyByConstantImageFilter.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkMultiplyByConstantImageFilter_h
#define __itkMultiplyByConstantImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkNumericTraits.h"

namespace itk
{
/** \class MultiplyByConstantImageFilter
 *
 * \brief Multiply input pixels by a constant.
 *
 * This filter is templated over the input image type
 * and the output image type.
 *
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 *
 *
 * This implementation was taken from the Insight Journal paper:
 * http://hdl.handle.net/1926/510
 *
 * \ingroup IntensityImageFilters  Multithreaded
 * \sa UnaryFunctorImageFilter
 */
namespace Functor
{
template< class TInput, class TConstant, class TOutput >
class MultiplyByConstant
{
public:
  MultiplyByConstant():m_Constant(NumericTraits< TConstant >::One) {}
  ~MultiplyByConstant() {}
  bool operator!=(const MultiplyByConstant & other) const
  {
    return !( *this == other );
  }

  bool operator==(const MultiplyByConstant & other) const
  {
    return other.m_Constant == m_Constant;
  }

  inline TOutput operator()(const TInput & A) const
  {
    // Because the user has to specify the constant we don't
    // check if the cte is not 0;
    return static_cast< TOutput >( A * m_Constant );
  }

  void SetConstant(TConstant ct) { this->m_Constant = ct; }
  const TConstant & GetConstant() const { return m_Constant; }

  TConstant m_Constant;
};
}

template< class TInputImage, class TConstant, class TOutputImage >
class ITK_EXPORT MultiplyByConstantImageFilter:
  public
  UnaryFunctorImageFilter< TInputImage, TOutputImage,
                           Functor::MultiplyByConstant<
                             typename TInputImage::PixelType, TConstant,
                             typename TOutputImage::PixelType > >
{
public:
  /** Standard class typedefs. */
  typedef MultiplyByConstantImageFilter Self;
  typedef UnaryFunctorImageFilter<
    TInputImage, TOutputImage,
    Functor::MultiplyByConstant<
      typename TInputImage::PixelType, TConstant,
      typename TOutputImage::PixelType >   >             Superclass;

  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiplyByConstantImageFilter, UnaryFunctorImageFilter);

  /** Set the constant that will be used to multiply all the image pixels */
  void SetConstant(TConstant ct)
  {
    if ( ct != this->GetFunctor().GetConstant() )
      {
      this->GetFunctor().SetConstant(ct);
      this->Modified();
      }
  }

  const TConstant & GetConstant() const
  {
    return this->GetFunctor().GetConstant();
  }

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( InputConvertibleToOutputCheck,
                   ( Concept::Convertible< typename TInputImage::PixelType,
                                           typename TOutputImage::PixelType > ) );
  itkConceptMacro( Input1Input2OutputMultiplyOperatorCheck,
                   ( Concept::MultiplyOperator< typename TInputImage::PixelType,
                                                TConstant,
                                                typename TOutputImage::PixelType > ) );
  /** End concept checking */
#endif
protected:
  MultiplyByConstantImageFilter() {}
  virtual ~MultiplyByConstantImageFilter() {}

  void PrintSelf(std::ostream & os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
    os << indent << "Constant: "
       << static_cast< typename NumericTraits< TConstant >::PrintType >( this->GetConstant() )
       << std::endl;
  }

private:
  MultiplyByConstantImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);                //purposely not implemented
};
} // end namespace itk

#endif

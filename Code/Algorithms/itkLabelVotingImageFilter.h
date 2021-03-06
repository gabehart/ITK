/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkLabelVotingImageFilter.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkLabelVotingImageFilter_h
#define __itkLabelVotingImageFilter_h

#include "itkImage.h"
#include "itkImageToImageFilter.h"

namespace itk
{
/** \class LabelVotingImageFilter
 *
 * \brief This filter performs pixelwise voting among an arbitrary number
 * of input images, where each of them represents a segmentation of the same
 * scene (i.e., image).
 *
 * Label voting is a simple method of classifier combination applied to
 * image segmentation. Typically, the accuracy of the combined segmentation
 * exceeds the accuracy of any of the input segmentations. Voting is therefore
 * commonly used as a way of boosting segmentation performance.
 *
 * The use of label voting for combination of multiple segmentations is
 * described in
 *
 * T. Rohlfing and C. R. Maurer, Jr., "Multi-classifier framework for
 * atlas-based image segmentation," Pattern Recognition Letters, 2005.
 *
 * \par INPUTS
 * All input volumes to this filter must be segmentations of an image,
 * that is, they must have discrete pixel values where each value represents
 * a different segmented object.
 *
 * Input volumes must all contain the same size RequestedRegions. Not all
 * input images must contain all possible labels, but all label values must
 * have the same meaning in all images.
 *
 * \par OUTPUTS
 * The voting filter produces a single output volume. Each output pixel
 * contains the label that occured most often among the labels assigned to
 * this pixel in all the input volumes, that is, the label that received the
 * maximum number of "votes" from the input pixels.. If the maximum number of
 * votes is not unique, i.e., if more than one label have a maximum number of
 * votes, an "undecided" label is assigned to that output pixel.
 *
 * By default, the label used for undecided pixels is the maximum label value
 * used in the input images plus one. Since it is possible for an image with
 * 8 bit pixel values to use all 256 possible label values, it is permissible
 * to combine 8 bit (i.e., byte) images into a 16 bit (i.e., short) output
 * image.
 *
 * \par PARAMETERS
 * The label used for "undecided" labels can be set using
 * SetLabelForUndecidedPixels. This functionality can be unset by calling
 * UnsetLabelForUndecidedPixels.
 *
 * \author Torsten Rohlfing, SRI International, Neuroscience Program
 *
 */
template< typename TInputImage, typename TOutputImage = TInputImage >
class ITK_EXPORT LabelVotingImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef LabelVotingImageFilter                          Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods) */
  itkTypeMacro(LabelVotingImageFilter, ImageToImageFilter);

  /** Extract some information from the image types.  Dimensionality
   * of the two images is assumed to be the same. */
  typedef typename TOutputImage::PixelType OutputPixelType;
  typedef typename TInputImage::PixelType  InputPixelType;

  /** Extract some information from the image types.  Dimensionality
   * of the two images is assumed to be the same. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Image typedef support */
  typedef TInputImage                           InputImageType;
  typedef TOutputImage                          OutputImageType;
  typedef typename InputImageType::ConstPointer InputImagePointer;
  typedef typename OutputImageType::Pointer     OutputImagePointer;

  /** Superclass typedefs. */
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  /** Set label value for undecided pixels.
    */
  void SetLabelForUndecidedPixels(const OutputPixelType l)
  {
    this->m_LabelForUndecidedPixels = l;
    this->m_HasLabelForUndecidedPixels = true;
    this->Modified();
  }

  /** Get label value used for undecided pixels.
   * After updating the filter, this function returns the actual label value
   * used for undecided pixels in the current output. Note that this value
   * is overwritten when SetLabelForUndecidedPixels is called and the new
   * value only becomes effective upon the next filter update.
   */
  OutputPixelType GetLabelForUndecidedPixels() const
  {
    return this->m_LabelForUndecidedPixels;
  }

  /** Unset label value for undecided pixels and turn on automatic selection.
    */
  void UnsetLabelForUndecidedPixels()
  {
    if ( this->m_HasLabelForUndecidedPixels )
      {
      this->m_HasLabelForUndecidedPixels = false;
      this->Modified();
      }
  }

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( InputConvertibleToOutputCheck,
                   ( Concept::Convertible< InputPixelType, OutputPixelType > ) );
  itkConceptMacro( IntConvertibleToInputCheck,
                   ( Concept::Convertible< int, InputPixelType > ) );
  itkConceptMacro( SameDimensionCheck,
                   ( Concept::SameDimension< InputImageDimension, ImageDimension > ) );
  itkConceptMacro( InputConvertibleToUnsignedIntCheck,
                   ( Concept::Convertible< InputPixelType, unsigned int > ) );
  itkConceptMacro( IntConvertibleToOutputPixelType,
                   ( Concept::Convertible< int, OutputPixelType > ) );
  itkConceptMacro( InputPlusIntCheck,
                   ( Concept::AdditiveOperators< InputPixelType, int > ) );
  itkConceptMacro( InputIncrementDecrementOperatorsCheck,
                   ( Concept::IncrementDecrementOperators< InputPixelType > ) );
  itkConceptMacro( OutputOStreamWritableCheck,
                   ( Concept::OStreamWritable< OutputPixelType > ) );
  /** End concept checking */
#endif
protected:
  LabelVotingImageFilter() { this->m_HasLabelForUndecidedPixels = false; }
  virtual ~LabelVotingImageFilter() {}

  /** Determine maximum label value in all input images and initialize
   * global data. */
  void BeforeThreadedGenerateData();

  void ThreadedGenerateData
    (const OutputImageRegionType & outputRegionForThread, int threadId);

  void PrintSelf(std::ostream &, Indent) const;

  /** Determine maximum value among all input images' pixels */
  InputPixelType ComputeMaximumInputValue();

private:
  LabelVotingImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);         //purposely not implemented

  OutputPixelType m_LabelForUndecidedPixels;
  bool            m_HasLabelForUndecidedPixels;
  InputPixelType  m_TotalLabelCount;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLabelVotingImageFilter.txx"
#endif

#endif

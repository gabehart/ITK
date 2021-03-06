/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkFEMElement1DStress.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkFEMElement1DStress_h
#define __itkFEMElement1DStress_h

#include "itkFEMElementBase.h"
#include "itkFEMMaterialLinearElasticity.h"

namespace itk {
namespace fem {

/**
 * \class Element1DStress
 * \brief Class that is used to define linear elasticity problem in 1D space.
 *
 * This class only defines the physics of the problem. Use his class together
 * with element classes that specify the geometry to fully define the element.
 *
 * You can specify one template parameter:
 *
 *   TBaseClass - Class from which Element1DStress is derived. TBaseClass must
 *                be derived from the Element base class. This enables you
 *                to use this class at any level of element definition.
 *                If not specified, it defaults to the Element base class.
 */
template<class TBaseClass=Element>
class Element1DStress : public TBaseClass
{
FEM_ABSTRACT_CLASS(Element1DStress,TBaseClass)
public:

  // Repeat the required typedefs and enums from parent class
  typedef typename Superclass::Float      Float;
  typedef typename Superclass::MatrixType MatrixType;
  typedef typename Superclass::VectorType VectorType;

  /**
   * Read data for this class from input stream
   */
  virtual void Read( std::istream&, void* info );

  /**
   * Write this class to output stream
   */
  virtual void Write( std::ostream& f ) const;

  /**
   * Default constructor only clears the internal storage
   */
  Element1DStress();

//////////////////////////////////////////////////////////////////////////
  /*
   * Methods related to the physics of the problem.
   */

  /**
   * Compute the B matrix.
   */
  virtual void GetStrainDisplacementMatrix(MatrixType& B, const MatrixType& shapeDgl) const;

  /**
   * Compute the D matrix.
   */
  virtual void GetMaterialMatrix(MatrixType& D) const;

  /**
   * Element stiffness matrix is reimplemented here, because we want to
   * be able to use this class to implement 1D stress problem in any 
   * number of dimensions i.e. Bar1D, Bar2D, Bar3D.
   */
  virtual void GetStiffnessMatrix(MatrixType& Ke) const;

  /**
   * 1D stress elements have 2 DOFs per node. In reality there is
   * only one, but it usually makes sense to separate it into the
   * components that correspond to specific dimensions in space.
   * So the number of DOFs per node is equal to the number of
   * spatial dimensions.
   */
  virtual unsigned int GetNumberOfDegreesOfFreedomPerNode( void ) const
    { return 2; }

public:

  /**
   * Pointer to material properties of the element
   */
  MaterialLinearElasticity::ConstPointer m_mat;
  virtual Material::ConstPointer GetMaterial(void) const { return m_mat; }
  virtual void SetMaterial(Material::ConstPointer mat_ ) { m_mat=dynamic_cast<const MaterialLinearElasticity*>(&*mat_); }
  

}; // class Element1DStress

#ifdef _MSC_VER
// Declare a static dummy function to prevent a MSVC 6.0 SP5 from crashing.
// I have no idea why things don't work when this is not declared, but it
// looks like this declaration makes compiler forget about some of the
// troubles it has with templates.
static void Dummy( void );
#endif // #ifdef _MSC_VER

}} // end namespace itk::fem

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFEMElement1DStress.txx"
#endif

#endif  // #ifndef __itkFEMElement1DStress_h

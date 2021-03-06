/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkExhaustiveOptimizerTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>
#include <algorithm>

#include "itkCommand.h"
#include "itkExhaustiveOptimizer.h"

#include <vnl/vnl_math.h>

/** 
 *  The objectif function is the quadratic form:
 *
 *  1/2 x^T A x - b^T x
 *
 *  Where A is a matrix and b is a vector
 *  The system in this example is:
 *
 *     | 3  2 ||x|   | 2|   |0|
 *     | 2  6 ||y| + |-8| = |0|
 *
 *
 *   the solution is the vector | 2 -2 |
 *
 */ 
class RSGCostFunction : public itk::SingleValuedCostFunction 
{
public:

  typedef RSGCostFunction                     Self;
  typedef itk::SingleValuedCostFunction      Superclass;
  typedef itk::SmartPointer<Self>            Pointer;
  typedef itk::SmartPointer<const Self>      ConstPointer;
  itkNewMacro( Self );

  enum { SpaceDimension=2 };
  
  typedef Superclass::ParametersType      ParametersType;
  typedef Superclass::DerivativeType      DerivativeType;
  typedef Superclass::MeasureType         MeasureType ;

  RSGCostFunction() 
  {
  }


  MeasureType  GetValue( const ParametersType & parameters ) const
  { 
    
    double x = parameters[0];
    double y = parameters[1];

    std::cout << "GetValue( " ;
    std::cout << x << " ";
    std::cout << y << ") = ";

    MeasureType measure = 0.5*(3*x*x+4*x*y+6*y*y) - 2*x + 8*y;

    std::cout << measure << std::endl;    

    return measure;

  }

  void GetDerivative( const ParametersType & parameters,
                            DerivativeType  & derivative ) const
  {

    double x = parameters[0];
    double y = parameters[1];

    std::cout << "GetDerivative( " ;
    std::cout << x << " ";
    std::cout << y << ") = ";

    derivative = DerivativeType( SpaceDimension ); 
    derivative[0] = 3 * x + 2 * y -2;
    derivative[1] = 2 * x + 6 * y +8;

  }

 
  unsigned int GetNumberOfParameters(void) const
    {
    return SpaceDimension;
    }
};

class IndexObserver : public itk::Command
{
public:
  typedef IndexObserver              Self;
  typedef itk::Command               Superclass;
  typedef itk::SmartPointer < Self > Pointer;

  itkNewMacro ( IndexObserver );

  virtual void  Execute ( const itk::Object *caller, const itk::EventObject &)
  {
    typedef itk::ExhaustiveOptimizer OptimizerType;
    const OptimizerType *optimizer = dynamic_cast < const OptimizerType * > ( caller );

    if ( 0 != optimizer )
    {
      OptimizerType::ParametersType currentIndex = optimizer->GetCurrentIndex ();

      if ( currentIndex.GetSize () == 2 )
      {
        std::cout << " @ index = " << currentIndex << std::endl;
        // Casting is safe here since the indices are always integer values (but there are stored in doubles):
        unsigned long idx = static_cast < unsigned long > ( currentIndex [ 0 ] + 21 * currentIndex [ 1 ] );
        m_visitedIndices.push_back ( idx );
      }
    } 
  }

  virtual void  Execute (itk::Object *caller, const itk::EventObject &event)
  {
    Execute ( static_cast < const itk::Object * > ( caller ), event );
  }

  std::vector < unsigned long > m_visitedIndices;
};

int itkExhaustiveOptimizerTest(int, char* [] ) 
{
  std::cout << "ExhaustiveOptimizer Test ";
  std::cout << std::endl << std::endl;

  typedef  itk::ExhaustiveOptimizer  OptimizerType;

  typedef  OptimizerType::ScalesType            ScalesType;
  
  
  // Declaration of a itkOptimizer
  OptimizerType::Pointer  itkOptimizer = OptimizerType::New();


  // Index observer (enables us to check if all positions were indeed visisted):
  IndexObserver::Pointer idxObserver = IndexObserver::New ();
  itkOptimizer->AddObserver ( itk::IterationEvent (), idxObserver );

  // Declaration of the CostFunction 
  RSGCostFunction::Pointer costFunction = RSGCostFunction::New();
  itkOptimizer->SetCostFunction( costFunction.GetPointer() );

  
  typedef RSGCostFunction::ParametersType    ParametersType;


  const unsigned int spaceDimension = 
                      costFunction->GetNumberOfParameters();

  // We start not so far from  | 2 -2 |
  ParametersType  initialPosition( spaceDimension );
  initialPosition[0] =  0.0;
  initialPosition[1] = -4.0;
  
  itkOptimizer->SetInitialPosition( initialPosition );


  ScalesType    parametersScale( spaceDimension );
  parametersScale[0] = 1.0;
  parametersScale[1] = 1.0;

  itkOptimizer->SetScales( parametersScale );


  itkOptimizer->SetStepLength( 1.0 );


  typedef OptimizerType::StepsType  StepsType;
  StepsType steps( 2 );
  steps[0] = 10;
  steps[1] = 10;

  itkOptimizer->SetNumberOfSteps( steps );


  try 
    {
    itkOptimizer->StartOptimization();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cout << "Exception thrown ! " << std::endl;
    std::cout << "An error ocurred during Optimization" << std::endl;
    std::cout << "Location    = " << e.GetLocation()    << std::endl;
    std::cout << "Description = " << e.GetDescription() << std::endl;
    return EXIT_FAILURE;
    }


  bool minimumValuePass = vnl_math_abs ( itkOptimizer->GetMinimumMetricValue() - -10 ) < 1E-3;

  std::cout << "MinimumMetricValue = " << itkOptimizer->GetMinimumMetricValue() << std::endl;
  std::cout << "Minimum Position = " << itkOptimizer->GetMinimumMetricValuePosition() << std::endl;

  bool maximumValuePass = vnl_math_abs ( itkOptimizer->GetMaximumMetricValue() - 926 ) < 1E-3;
  std::cout << "MaximumMetricValue = " << itkOptimizer->GetMaximumMetricValue() << std::endl;
  std::cout << "Maximum Position = " << itkOptimizer->GetMaximumMetricValuePosition() << std::endl;

  ParametersType finalPosition = itkOptimizer->GetMinimumMetricValuePosition();
  std::cout << "Solution        = (";
  std::cout << finalPosition[0] << "," ;
  std::cout << finalPosition[1] << ")" << std::endl;  

  bool visitedIndicesPass = true;
  std::vector < unsigned long > visitedIndices = idxObserver->m_visitedIndices;

  size_t requiredNumberOfSteps = ( 2 * steps [ 0 ] + 1 ) * ( 2 * steps [ 1 ] + 1 );
  if ( visitedIndices.size () != requiredNumberOfSteps )
  {
    visitedIndicesPass = false;
  }

  std::sort ( visitedIndices.begin (), visitedIndices.end () );

  for ( size_t i = 0; i < visitedIndices.size (); ++i )
    {
    if ( visitedIndices [ i ] != i )
      {
      visitedIndicesPass = false;
      std::cout << "Mismatch in visited index " << visitedIndices [ i ] << " @ " << i << std::endl;
      break;
      }    
    }

  //
  // check results to see if it is within range
  //
  bool trueParamsPass = true;
  double trueParameters[2] = { 2, -2 };
  for( unsigned int j = 0; j < 2; j++ )
    {
    if( vnl_math_abs( finalPosition[j] - trueParameters[j] ) > 0.01 )
      {
      trueParamsPass = false;
      }
    }

  if( !minimumValuePass || !maximumValuePass || !trueParamsPass || !visitedIndicesPass )
    {
    std::cout << "minimumValuePass   = " << minimumValuePass << std::endl;
    std::cout << "maximumValuePass   = " << maximumValuePass << std::endl;
    std::cout << "trueParamsPass     = " << trueParamsPass << std::endl;
    std::cout << "visitedIndicesPass = " << visitedIndicesPass << std::endl;
    std::cout << "Test failed." << std::endl;
    return EXIT_FAILURE;
    }


  std::cout << "Testing PrintSelf " << std::endl;
  itkOptimizer->Print( std::cout );

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;


}

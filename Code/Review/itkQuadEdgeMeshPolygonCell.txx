/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkQuadEdgeMeshPolygonCell.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkQuadEdgeMeshPolygonCell_txx
#define __itkQuadEdgeMeshPolygonCell_txx

#include "itkCellInterfaceVisitor.h"

namespace itk
{
// ---------------------------------------------------------------------
template< class TCellInterface >
QuadEdgeMeshPolygonCell< TCellInterface >
::QuadEdgeMeshPolygonCell(PointIdentifier nPoints)
{
  this->m_Ident = 0;

  // Create entry point
  EdgeCellType *edge = new EdgeCellType;
  m_EdgeCellList.push_back(edge);
  m_EdgeRingEntry = edge->GetQEGeom();

  // Create the rest
  QuadEdgeType *last = m_EdgeRingEntry;
  for ( PointIdentifier i = 1; i < nPoints; i++ )
    {
    edge = new EdgeCellType();
    m_EdgeCellList.push_back(edge);
    QuadEdgeType *edgeGeom = edge->GetQEGeom();

    edgeGeom->Splice( last->GetSym() );
    last = edgeGeom;
    }

  // Last topological connection, i.e., close the face
  m_EdgeRingEntry->Splice( last->GetSym() );

  //MakePointIds( );
}

// ---------------------------------------------------------------------
template< class TCellInterface >
QuadEdgeMeshPolygonCell< TCellInterface >
::QuadEdgeMeshPolygonCell(QuadEdgeType *e)
{
  this->m_Ident = 0;
  this->m_EdgeRingEntry = e;

  //MakePointIds( );
}

// ---------------------------------------------------------------------
template< class TCellInterface >
QuadEdgeMeshPolygonCell< TCellInterface >
::~QuadEdgeMeshPolygonCell()
{
  // this disconnect this cell from the
  // QuadEdgeMesh container if there was any.
  // poping this cell from the container is supposed to
  // have been done beforehand.
  this->m_Ident = 0;

  // first case, the polygon was created directly
  // just delete the edges in the edge list
  // the edgecell destructor should  take care of the QuadEdges
  while ( !m_EdgeCellList.empty() )
    {
    EdgeCellType *edge = m_EdgeCellList.back();
    m_EdgeCellList.pop_back();
    delete edge;
    }

  // second case, the polygon cell was created by
  // QuadEdgeMesh::AddFace( ) and the QuadEdgeMesh
  // should take care of everything.
  // We could iterate around the face to check if all
  // QuadEdges have now IsLeftSet( ) = false.
  //
  // TO BE DONE.
}

// ---------------------------------------------------------------------
template< class TCellInterface >
typename QuadEdgeMeshPolygonCell< TCellInterface >::SelfAutoPointer
QuadEdgeMeshPolygonCell< TCellInterface >
::New()
{
  SelfAutoPointer ptr(new Self);

  ptr.TakeOwnership();
  return ( ptr );
}

// ---------------------------------------------------------------------
template< class TCellInterface >
void QuadEdgeMeshPolygonCell< TCellInterface >
::Accept(unsigned long cellId, MultiVisitor *mv)
{
  typedef CellInterfaceVisitor< PixelType, CellTraits > IntVis;
  typename IntVis::Pointer v = mv->GetVisitor( this->GetType() );
  if ( v )
    {
    v->VisitFromCell(cellId, this);
    }
}

// ---------------------------------------------------------------------
template< class TCellInterface >
unsigned int QuadEdgeMeshPolygonCell< TCellInterface >
::GetNumberOfPoints() const
{
  // The constructor creates one edge by default
  unsigned int                 n = 0;
  PointIdInternalConstIterator it = this->InternalPointIdsBegin();

  while ( it != this->InternalPointIdsEnd() )
    {
    it++;
    n++;
    }
  // it's impossible to get n < 3 except the empty case
  if ( n > 2 )
    {
    return n;
    }
  else
    {
    return 0;
    }
}

// ---------------------------------------------------------------------
template< class TCellInterface >
typename QuadEdgeMeshPolygonCell< TCellInterface >::CellFeatureCount
QuadEdgeMeshPolygonCell< TCellInterface >
::GetNumberOfBoundaryFeatures(int dimension) const
{
  switch ( dimension )
    {
    case 0:
      return ( this->GetNumberOfPoints() );
    case 1:
      return ( this->GetNumberOfPoints() );
    default:
      return ( 0 );
    }
}

// ---------------------------------------------------------------------
template< class TCellInterface >
bool QuadEdgeMeshPolygonCell< TCellInterface >::GetBoundaryFeature(int dimension, CellFeatureIdentifier cellId,
                                                                   CellAutoPointer & cell)
{
  /// \todo
  (void)dimension;
  (void)cellId;
  (void)cell;
  return ( false );
}

// ---------------------------------------------------------------------
template< class TCellInterface >
void
QuadEdgeMeshPolygonCell< TCellInterface >
::SetPointIds(PointIdConstIterator first)
{
  if ( this->GetNumberOfPoints() > 2 )
    {
    PointIdConstIterator    i2 = first;
    PointIdInternalIterator i1 = this->InternalPointIdsBegin();
    while ( i1 != this->InternalPointIdsEnd() )
      {
      i1.Value()->SetOrigin(*i2);
      i1++;
      i2++;
      }
    }
}

// ---------------------------------------------------------------------
template< class TCellInterface >
void
QuadEdgeMeshPolygonCell< TCellInterface >
::InternalSetPointIds(PointIdInternalConstIterator first)
{
  if ( this->GetNumberOfPoints() > 2 )
    {
    PointIdInternalConstIterator i2 = first;
    PointIdInternalIterator      i1 = this->InternalPointIdsBegin();
    while ( i1 != this->InternalPointIdsEnd() )
      {
      i1.Value()->SetOrigin(*i2);
      i1++;
      i2++;
      }
    }
}

// ---------------------------------------------------------------------
template< class TCellInterface >
void QuadEdgeMeshPolygonCell< TCellInterface >
::SetPointIds(PointIdConstIterator first,
              PointIdConstIterator last)
{
  PointIdInternalIterator i1 = this->InternalPointIdsBegin();
  PointIdConstIterator    i2 = first;

  while ( i1 != this->InternalPointIdsEnd() && i2 != last )
    {
    i1.Value()->SetOrigin(*i2);
    i1++;
    i2++;
    }
}

// ---------------------------------------------------------------------
template< class TCellInterface >
void QuadEdgeMeshPolygonCell< TCellInterface >
::InternalSetPointIds(PointIdInternalConstIterator first,
                      PointIdInternalConstIterator last)
{
  PointIdInternalIterator      i1 = this->InternalPointIdsBegin();
  PointIdInternalConstIterator i2 = first;

  while ( i1 != this->InternalPointIdsEnd() && i2 != last )
    {
    i1.Value()->SetOrigin(*i2);
    i1++;
    i2++;
    }
}

// ---------------------------------------------------------------------
template< class TCellInterface >
void QuadEdgeMeshPolygonCell< TCellInterface >
::SetPointId(int localId, PointIdentifier pId)
{
  int                     n = 0;
  PointIdInternalIterator it = this->InternalPointIdsBegin();

  while ( it != this->InternalPointIdsEnd() && n <= localId )
    {
    if ( n == localId )
      {
      it.Value()->SetOrigin(pId);
      it.Value()->GetOnext()->SetOrigin(pId);
      }
    it++;
    n++;
    }
}

// ---------------------------------------------------------------------
template< class TCellInterface >
typename QuadEdgeMeshPolygonCell< TCellInterface >::PointIdentifier
QuadEdgeMeshPolygonCell< TCellInterface >
::GetPointId(int localId) const
{
  int                          n = 0;
  PointIdInternalConstIterator it = this->InternalPointIdsBegin();

  while ( it != this->InternalPointIdsEnd() && n <= localId )
    {
    if ( n == localId )
      {
      return ( it.Value()->GetOrigin() );
      }
    it++;
    n++;
    }
  return ( PointIdentifier(-1) );
}

// ---------------------------------------------------------------------
template< class TCellInterface >
typename QuadEdgeMeshPolygonCell< TCellInterface >::PointIdInternalIterator
QuadEdgeMeshPolygonCell< TCellInterface >
::InternalPointIdsBegin()
{
  return m_EdgeRingEntry->BeginGeomLnext();
}

// ---------------------------------------------------------------------
template< class TCellInterface >
typename QuadEdgeMeshPolygonCell< TCellInterface >::PointIdInternalIterator
QuadEdgeMeshPolygonCell< TCellInterface >
::InternalPointIdsEnd()
{
  return m_EdgeRingEntry->EndGeomLnext();
}

// ---------------------------------------------------------------------
template< class TCellInterface >
typename QuadEdgeMeshPolygonCell< TCellInterface >::PointIdInternalConstIterator
QuadEdgeMeshPolygonCell< TCellInterface >
::InternalGetPointIds() const
{
  const QuadEdgeType *         edge = const_cast< QuadEdgeType * >( m_EdgeRingEntry );
  PointIdInternalConstIterator iterator( edge->BeginGeomLnext() );

  return iterator;
}

// ---------------------------------------------------------------------
template< class TCellInterface >
typename QuadEdgeMeshPolygonCell< TCellInterface >::PointIdInternalConstIterator
QuadEdgeMeshPolygonCell< TCellInterface >
::InternalPointIdsBegin() const
{
  const QuadEdgeType *         edge = const_cast< QuadEdgeType * >( m_EdgeRingEntry );
  PointIdInternalConstIterator iterator( edge->BeginGeomLnext() );

  return iterator;
}

// ---------------------------------------------------------------------
template< class TCellInterface >
typename QuadEdgeMeshPolygonCell< TCellInterface >::PointIdInternalConstIterator
QuadEdgeMeshPolygonCell< TCellInterface >
::InternalPointIdsEnd() const
{
  const QuadEdgeType *         edge = const_cast< const QuadEdgeType * >( m_EdgeRingEntry );
  PointIdInternalConstIterator iterator = edge->EndGeomLnext();

  return iterator;
}
} // end namespace itk

#endif

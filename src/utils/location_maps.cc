

// $Id: location_maps.cc,v 1.2 2008/05/22 14:13:24 gdiso Exp $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2007  Benjamin S. Kirk, John W. Peterson
  
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
  
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



// C++ Includes -----------------------------------
#include <limits>
#include <utility>

// Local Includes -----------------------------------
#include "elem.h"
#include "location_maps.h"
#include "mesh_base.h"
#include "node.h"
#include "parallel.h"



//--------------------------------------------------------------------------

template <typename T>
void LocationMap<T>::init(MeshBase& mesh)
{
  // This function must be run on all processors at once
  // for non-serial meshes
  if (!mesh.is_serial())
    parallel_only();

  START_LOG("init()", "LocationMap");

  // Clear the old map
  _map.clear();

  // Cache a bounding box
  _lower_bound.clear();
  _lower_bound.resize(3, std::numeric_limits<Real>::max());
  _upper_bound.clear();
  _upper_bound.resize(3, -std::numeric_limits<Real>::max());

  MeshBase::node_iterator       it  = mesh.nodes_begin();
  const MeshBase::node_iterator end = mesh.nodes_end();

  for (; it != end; ++it)
    {
      Node* node = *it;

      // Expand the bounding box if necessary
      _lower_bound[0] = std::min(_lower_bound[0],
                                 (*node)(0));
      _lower_bound[1] = std::min(_lower_bound[1],
                                 (*node)(1));
      _lower_bound[2] = std::min(_lower_bound[2],
                                 (*node)(2));
      _upper_bound[0] = std::max(_upper_bound[0],
                                 (*node)(0));
      _upper_bound[1] = std::max(_upper_bound[1],
                                 (*node)(1));
      _upper_bound[2] = std::max(_upper_bound[2],
                                 (*node)(2));
    }

  // On a parallel mesh we might not yet have a full bounding box
  if (!mesh.is_serial())
    {
      Parallel::min(_lower_bound);
      Parallel::max(_upper_bound);
    }

  this->fill(mesh);

  STOP_LOG("init()", "LocationMap");
}



template <typename T>
void LocationMap<T>::insert(T &t)
{
  this->_map.insert(std::make_pair(this->key(this->point_of(t)), &t));
}



template <>
Point LocationMap<Node>::point_of(const Node& node) const
{
  return node;
}



template <>
Point LocationMap<Elem>::point_of(const Elem& elem) const
{
  return elem.centroid();
}



template <typename T>
T* LocationMap<T>::find(const Point& p,
                     const Real tol)
{
  // Look for the key in the multimap
  std::pair<typename map_type::iterator,
            typename map_type::iterator>
    pos = _map.equal_range(this->key(p));

  while (pos.first != pos.second)
    if (p.absolute_fuzzy_equals
         (this->point_of(*(pos.first->second)), tol))
      return pos.first->second;
    else
      ++pos.first;

  return NULL;
}



template <typename T>
unsigned int LocationMap<T>::key(const Point& p)
{
  Real xscaled = (p(0) - _lower_bound[0])/
                 (_upper_bound[0] - _lower_bound[0]),
       yscaled = (p(1) - _lower_bound[1])/
                 (_upper_bound[1] - _lower_bound[1]),
       zscaled = (p(2) - _lower_bound[2])/
                 (_upper_bound[2] - _lower_bound[2]);

  // 10 bits per coordinate, to work with 32+ bit machines
  unsigned chunkmax = 1024;
  Real chunkfloat = 1024.0;

  unsigned int n0 = static_cast<unsigned int> (chunkfloat * xscaled),
               n1 = static_cast<unsigned int> (chunkfloat * yscaled),
               n2 = static_cast<unsigned int> (chunkfloat * zscaled);

  return chunkmax*chunkmax*n0 + chunkmax*n1 + n2;
}



template <>
void LocationMap<Node>::fill(MeshBase& mesh)
{
  // Populate the nodes map
  MeshBase::node_iterator  it = mesh.nodes_begin(),
                          end = mesh.nodes_end();
  for (; it != end; ++it)
    this->insert(**it);
}



template <>
void LocationMap<Elem>::fill(MeshBase& mesh)
{
  // Populate the elem map
  MeshBase::element_iterator       it  = mesh.active_elements_begin(),
                                   end = mesh.active_elements_end();
  for (; it != end; ++it)
    this->insert(**it);
}



template class LocationMap<Elem>;
template class LocationMap<Node>;

// $Id: tree.h,v 1.3 2008/05/22 14:14:19 gdiso Exp $

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



#ifndef __tree_h__
#define __tree_h__

// C++ includes

// Local includes
#include "tree_node.h"
#include "tree_base.h"

// Forward Declarations
class MeshBase;

/**
 * This class defines a tree that may be used for fast point
 * location in space.
 *
 * @author Benjamin S. Kirk, 2002
 */

// ------------------------------------------------------------
// Tree class definition
template <unsigned int N>
class Tree : public TreeBase
{
public:

  /**
   * Constructor.
   */
  Tree (const MeshBase& m, const unsigned int tbs, const unsigned int mlevel=100, Trees::BuildType bt=Trees::NODES);

  /**
   * Copy-constructor.
   */
  Tree (const Tree<N>& other_tree);

  /**
   * Destructor.
   */
  ~Tree();

  unsigned int n_leaf() const
  { return N; }

  /**
   * Prints the nodes.
   */
  void print_nodes() const
  { std::cout << "Printing nodes...\n"; root.print_nodes(); }

  /**
   * Prints the nodes.
   */
  void print_elements() const
  { std::cout << "Printing elements...\n"; root.print_elements(); }

  /**
   * @returns the number of active bins.
   */
  unsigned int n_active_bins() const { return root.n_active_bins(); }

  /**
   * @returns a pointer to the element containing point p.
   */
  const Elem* find_element(const Point& p) const;

  /**
   * @returns a pointer to the element containing point p.
   */
  const Elem* operator() (const Point& p) const;

  /**
   * @return the first element the ray(p,dir) hit
   */
  const Elem * hit_element(const Point & p, const Point & dir) const;

  /**
   * @return true if the ray hit the mesh bounding_box
   */
  bool hit_boundbox(const Point & p, const Point & dir) const;

private:


  /**
   * The tree root.
   */
  TreeNode<N> root;


  /**
   * How the tree is built.
   */
  const Trees::BuildType build_type;


  /**
   * for surface elem mode, record surface elem here
   */
  std::vector<const Elem *> surface_elems;

};



/**
 * For convenience we define QuadTrees and OctTrees
 * explicitly.
 */
namespace Trees
{
  /**
   * A QuadTree is a tree appropriate
   * for 2D meshes.
   */
  typedef Tree<4> QuadTree;

  /**
   * An OctTree is a tree appropriate
   * for 3D meshes.
   */
  typedef Tree<8> OctTree;
}



// ------------------------------------------------------------
// Tree class inline methods



// copy-constructor
template <unsigned int N>
inline
Tree<N>::Tree (const Tree<N>& other_tree) :
  TreeBase   (other_tree),
  root       (other_tree.root),
  build_type (other_tree.build_type)
{
  genius_error();
}



template <unsigned int N>
inline
const Elem* Tree<N>::operator() (const Point& p) const
{
  return this->find_element(p);
}



#endif

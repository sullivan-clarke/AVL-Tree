#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "../include/avltree.hpp"
using namespace std;
using CS202::AVLTree;
using CS202::AVLNode;

/* Sullivan Clarke
 * avltree_lab.cpp
 * November 24th, 2025
 *
 * This lab finishes up the implementation for a full AVL tree.
 * The lab implements the AO, insert, delete, and more methods in order
 * to create fully funcitoning AVL tree program.
 */

//Assignment overload, copies a tree over to another tree
AVLTree& AVLTree::operator= (const AVLTree &t)        
{
  Clear();
  AVLNode *root = recursive_postorder_copy(t.sentinel->right); //Takes root from tree to be copied and copies it
  size = t.size;
  sentinel->right = root;
  root->parent = sentinel;
  return *this;
}

//Checks if a node is imbalanced
bool imbalance(AVLNode *n)
{
  //Calculates difference in child heights
  int childDiff = (n->right->height - n->left->height);
  if (childDiff > 1 || childDiff < -1) return true;
  return false;
}

//Fixes the height of a node to ensure its right
void fix_height(AVLNode *n)
{
  int lChild, rChild, height; 
  if (n->height == 0) return; //Stops case of trying to find children on a node that does not exist, aka the sentinel
  lChild = n->left->height; 
  rChild = n->right->height;
  
  height = (lChild > rChild) ? lChild : rChild;
  n->height = height + 1;
}

//Rotates about a node and links all necessary nodes together
void rotate(AVLNode *n)
{
  AVLNode *parent, *grandparent, *middle;
  bool leftOfParent, leftOfGrandParent; 
  leftOfParent = false;
  leftOfGrandParent = true;
  parent = n->parent;
  grandparent = n->parent->parent;

  //Checks for attempted rotation about root node or sentinel 
  if (n->parent->height == 0 || n->height == 0) return;

  //Gets direction of grandparent and parent to make linking after rotation easier
  if (parent->left == n) leftOfParent = true; 

  //Takes care of sentinel case since root is always right of sentinel 
  if ((grandparent->height == 0) || (grandparent->right == parent)) leftOfGrandParent = false; 

  //Gets middle child, becomes opposite side as parentDirection 
  if (leftOfParent)
  {
    middle = n->right;
  } else {
    middle = n->left;
  }
  
  //Links nodes in correct manner depending on orignal tree composition 
  if (leftOfParent)
  {
    n->right = parent;
    parent->left = middle;
  } else {
    n->left = parent;
    parent->right = middle;
  }
  parent->parent = n;
  if (middle->height != 0) middle->parent = parent; 
    
  //Ensures the node is connected to the correct grandparent node 
  if (leftOfGrandParent)
  {
    grandparent->left = n; 
  } else {
    grandparent->right = n;
  }
  //Sets correct parents
  n->parent = grandparent;

  //Adjusts immediate heights of node rotated and children 
  fix_height(n->left);
  fix_height(n->right);
  fix_height(n);

  //Moves up the tree fixing heights
  while (n->height != 0)
  {
    fix_height(n);
    n = n->parent;
  }
}

//Determines if the imbalance is zig zig or zig zag and preforms correct rotation to fix it
void fix_imbalance(AVLNode *n)
{
  string direction = "";
  AVLNode *next = n; 

  //Finds path 
  for (int i = 0; i < 2; i++)
  {
    if (i == 0)
    {
      //Determines if left heavy or right heavy and adds that path to the string
      direction += (next->right->height > next->left->height) ? '1' : '0';  //1 signals a right path while 0 signals a left path
    } else {
      if (direction.at(0) == '0') 
      {
        direction += (next->left->height >= next->right->height) ? '0' : '1';
      } else {
        direction += (next->right->height >= next->left->height) ? '1' : '0'; 
      }

    }
      next = (direction.at(i) == '1') ? next->right : next->left;
  }
  //Determine imbalance type, zig zig or zig zag
  if (direction.at(0) == direction.at(1))
  {
    if (direction.at(0) == '0')
    {
      rotate(n->left);
    } else {
      rotate(n->right);

    }
  } else {
    if (direction.at(0) == '0')
    {
      next = n->left->right;
      rotate(next);
      rotate(next);
    } else {
      next = n->right->left;
      rotate(next);
      rotate(next);
    }
  }

}

//Inserts a node to the AVL tree and ensures the tree remains balanced
bool AVLTree::Insert(const string &key, void *val)
{
  AVLNode *parent;
  AVLNode *n;

  parent = sentinel;
  n = sentinel->right;

  /* Find where the key should go.  If you find the key, return false. */

  while (n != sentinel) {
    if (n->key == key) return false;
    parent = n;
    n = (key < n->key) ? n->left : n->right;
  }

  n = new AVLNode;
  n->key = key;
  n->val = val;
  n->parent = parent;
  n->height = 1;
  n->left = sentinel;
  n->right = sentinel;

  /* Use the correct pointer in the parent to point to the new node. */

  if (parent == sentinel) {
    sentinel->right = n;
  } else if (key < parent->key) {
    parent->left = n;
  } else {
    parent->right = n;
  }

  //From this point on, the new node is hooked in, moves up the tree, adjusting heights and imbalances
  AVLNode *next;

  while (n != sentinel)
  {
    next = n;
    fix_height(next);
    if (imbalance(next))
    {  
      fix_imbalance(next);
    }
    n = n->parent;
  }

  size++;
  return true;
}
    
//Deletes a node from the AVL tree
bool AVLTree::Delete(const string &key)
{
  AVLNode *n, *parent, *mlc, *start;
  string tmpkey;
  void *tmpval;

  /* Try to find the key -- if you can't return false. */

  n = sentinel->right;
  while (n != sentinel && key != n->key) {
    n = (key < n->key) ? n->left : n->right;
  }
  if (n == sentinel) return false;

  /* We go through the three cases for deletion, although it's a little
     different from the canonical explanation. */

  parent = n->parent;
  /* Case 1 - I have no left child.  Replace me with my right child.
     Note that this handles the case of having no children, too. */

  if (n->left == sentinel) {
    if (n == parent->left) {
      parent->left = n->right;
    } else {
      parent->right = n->right;
    }
    if (n->right != sentinel)
    {
      n->right->parent = parent;
    }
    delete n;
    size--;

  /* Case 2 - I have no right child.  Replace me with my left child. */

  } else if (n->right == sentinel) {
    if (n == parent->left) {
      parent->left = n->left;
    } else {
      parent->right = n->left;
    }
    n->left->parent = parent;
    delete n;
    size--;

  /* If I have two children, then find the node "before" me in the tree.
     That node will have no right child, so I can recursively delete it.
     When I'm done, I'll replace the key and val of n with the key and
     val of the deleted node.  You'll note that the recursive call 
     updates the size, so you don't have to do it here. */

  } else {
    //Finds right most node in left subtree
    for (mlc = n->left; mlc->right != sentinel; mlc = mlc->right) ;
    tmpkey = mlc->key;
    tmpval = mlc->val;
    Delete(tmpkey);
    n->key = tmpkey;  //Replaces n with the replacement, basically deletes it
    n->val = tmpval;
    return true;
  }
  
  //Move up tree again, checking for imbalances and rotating
  while (parent != sentinel)
  {
    start = parent;
    fix_height(start);
    if (imbalance(start))
    {
      fix_imbalance(start);
    }
    parent = parent->parent;
  }
  return true;
}
               
//Returns the keys in a vector, ordered
vector <string> AVLTree::Ordered_Keys() const
{
  vector <string> rv;
  make_key_vector(sentinel->right, rv);
  return rv;
}
    
//Turns the keys into a vector
void AVLTree::make_key_vector(const AVLNode *n, vector<string> &v) const
{
  //In order traversal of the tree
  if (n == sentinel) return;
  make_key_vector(n->left, v);
  v.push_back(n->key);
  make_key_vector(n->right, v);
}
     
//Returns the height of an AVL tree
size_t AVLTree::Height() const
{
  return sentinel->right->height;
}

//Creates postorder copy of the an AVL tree
AVLNode *AVLTree::recursive_postorder_copy(const AVLNode *n) const
{
  if (n->height == 0) return sentinel;

  AVLNode *leftChild = recursive_postorder_copy(n->left);
  AVLNode *rightChild = recursive_postorder_copy(n->right);
  AVLNode *parent = new AVLNode;
  parent->key = n->key;
  parent->val = n->val;
  parent->height = n->height;
  if (leftChild != sentinel) leftChild->parent = parent;
  if (rightChild != sentinel) rightChild->parent = parent;
  parent->left = leftChild;
  parent->right = rightChild;
  return parent;
}

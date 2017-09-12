//------------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    2.1
/// \date       August 29, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///
/// \file       item.cpp
/// \author     Qi WU
///
/// \brief Modified Item class for ray-packet project
///
//------------------------------------------------------------------------------

#include "item.h"
#include <string.h>

//------------------------------------------------------------------------------

namespace qw {
  ItemBase::ItemBase() : name(NULL) {}
  ItemBase::~ItemBase() { if ( name ) delete [] name; }
  const char* ItemBase::GetName() const { return name ? name : ""; }
  void ItemBase::SetName(const char *newName)
  {
    if (name) delete [] name;
    if (newName) {
      int n = strlen(newName);
      name = new char[n+1];
      for (int i=0; i<n; i++) name[i] = newName[i];
      name[n] = '\0';
    } else { name = NULL; }
  }
};

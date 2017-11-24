///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 11/24/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.             //
///                                                                          //
/// Redistribution and use in source and binary forms, with or without       //
/// modification, are permitted provided that the following conditions are   //
/// met:                                                                     //
///  - Redistributions of source code must retain the above copyright        //
///    notice, this list of conditions and the following disclaimer.         //
///  - Redistributions in binary form must reproduce the above copyright     //
///    notice, this list of conditions and the following disclaimer in the   //
///    documentation and/or other materials provided with the distribution.  //
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS  //
/// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    //
/// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A          //
/// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       //
/// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   //
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         //
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,    //
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    //
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      //
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE    //
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.     //
///                                                                          //
///--------------------------------------------------------------------------//

#ifndef QARAY_ITEMS_H
#define QARAY_ITEMS_H
#pragma once

#include "core/core.h"
#include "math/math.h"

namespace qaray {
//-----------------------------------------------------------------------------
class ItemBase {
 private:
  char *name;          // The name of the item
 public:
  ItemBase() : name(nullptr) {}
  virtual ~ItemBase() { if (name) delete[] name; }
  const char *GetName() const { return name ? name : ""; }
  void SetName(const char *newName);
};
//-----------------------------------------------------------------------------
template<class T>
class ItemList : public std::vector<T *> {
 public:
  virtual ~ItemList() { DeleteAll(); }
  void DeleteAll()
  {
    size_t n = (int) this->size();
    for (size_t i = 0; i < n; i++) if (this->at(i)) delete this->at(i);
  }
};
//-----------------------------------------------------------------------------
template<class T>
class ItemFileList {
 public:
  void Clear() { list.DeleteAll(); }
  void Append(T *item, const char *name)
  {
    list.push_back(new FileInfo(item, name));
  }
  T *Find(const char *name) const
  {
    size_t n = list.size();
    for (size_t i = 0; i < n; i++)
      if (list[i] && strcmp(name, list[i]->GetName()) == 0)
        return list[i]->GetObj();
    return nullptr;
  }
 private:
  class FileInfo : public ItemBase {
   private:
    T *item;
   public:
    FileInfo() : item(nullptr) {}
    FileInfo(T *_item, const char *name) : item(_item) { SetName(name); }
    ~FileInfo() { Delete(); }
    void Delete() { if (item) delete item; item = nullptr; }
    void SetObj(T *_item) { Delete(); item = _item; }
    T *GetObj() { return item; }
  };
  ItemList<FileInfo> list;
};
}

#endif //QARAY_ITEMS_H

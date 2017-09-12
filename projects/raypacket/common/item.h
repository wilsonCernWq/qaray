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
/// \file       item.h 
/// \author     Qi WU
///
/// \brief Modified Item classes for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include <vector>

//------------------------------------------------------------------------------

namespace qw {
  class ItemBase {
  private:
    char *name; // The name of the item
  public:
    ItemBase();
    virtual ~ItemBase();
    const char* GetName() const;
    void SetName(const char *newName);
  };
};

//------------------------------------------------------------------------------

namespace qw {
  template <class T> class ItemList : public std::vector<T*> {
  public:
    virtual ~ItemList() { DeleteAll(); }
    void DeleteAll() { 
      int n = (int)this->size(); 
      for (int i=0; i<n; i++) { if (this->at(i)) delete this->at(i); }
    }
  };
};

//------------------------------------------------------------------------------

namespace qw {
  template <class T> class ItemFileList{
  public:
    void Clear() { list.DeleteAll(); }
    void Append( T* item, const char *name ) { list.push_back( new FileInfo(item,name) ); }
    T* Find( const char *name ) const { 
      int n=list.size(); 
      for ( int i=0; i<n; i++ ) { 
	if ( list[i] && strcmp(name,list[i]->GetName())==0 ) {
	  return list[i]->GetObj(); 
	}
      }
      return NULL; 
    }
  private:
    class FileInfo : public ItemBase {
    private:
      T *item;
    public:
      FileInfo() : item(NULL) {}
      FileInfo(T *_item, const char *name) : item(_item) { SetName(name); }
      ~FileInfo() { Delete(); }
      void Delete() { if (item) delete item; item=NULL; }
      void SetObj(T *_item) { Delete(); item=_item; }
      T* GetObj() { return item; }
    };
    ItemList<FileInfo> list;
  };
};

//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
///
/// \file       xmlload.cpp 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    5.0
/// \date       September 24, 2015
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-----------------------------------------------------------------------------

#include "xmlload.h"

#include "scene.h"
#include "objects.h"
#include "materials.h"
#include "lights.h"
#include "globalvar.h"

#include <tinyxml/tinyxml.h>

//-----------------------------------------------------------------------------

#ifdef WIN32
#define COMPARE(a,b) (_stricmp(a,b)==0)
#else
#define COMPARE(a,b) (strcasecmp(a,b)==0)
#endif

//-----------------------------------------------------------------------------

void LoadScene(TiXmlElement *element);
void LoadNode(Node *node, TiXmlElement *element, int level=0);
void LoadTransform( Transformation *trans, TiXmlElement *element, int level );
void LoadMaterial(TiXmlElement *element);
void LoadLight(TiXmlElement *element);
void ReadVector(TiXmlElement *element, Point3 &v);
void ReadColor (TiXmlElement *element, Color  &c);
void ReadFloat (TiXmlElement *element, float  &f, const char *name="value");

//-----------------------------------------------------------------------------

bool silentmode = false;

void LoadSceneInSilentMode(bool flag) { silentmode = flag; }

//-----------------------------------------------------------------------------

struct NodeMtl
{
  Node *node;
  const char *mtlName;
};

std::vector<NodeMtl> nodeMtlList;

//-----------------------------------------------------------------------------

int LoadScene(const char *filename)
{
  TiXmlDocument doc(filename);
  if ( ! doc.LoadFile() ) {
    if (!silentmode) {
      printf("Failed to load the file \"%s\"\n", filename);
    }
    return 0;
  }

  TiXmlElement *xml = doc.FirstChildElement("xml");
  if ( ! xml ) {
    if (!silentmode) {
      printf("No \"xml\" tag found.\n");
    }
    return 0;
  }

  TiXmlElement *scene = xml->FirstChildElement("scene");
  if ( ! scene ) {
    if (!silentmode) {
      printf("No \"scene\" tag found.\n");
    }
    return 0;
  }

  TiXmlElement *cam = xml->FirstChildElement("camera");
  if ( ! cam ) {
    if (!silentmode) {
      printf("No \"camera\" tag found.\n");
    }
    return 0;
  }

  nodeMtlList.clear();
  rootNode.Init();
  materials.DeleteAll();
  lights.DeleteAll();
  objList.Clear();
  LoadScene( scene );

  rootNode.ComputeChildBoundBox();

  // Assign materials
  int numNodes = nodeMtlList.size();
  for ( int i=0; i<numNodes; i++ ) {
    Material *mtl = materials.Find( nodeMtlList[i].mtlName );
    if ( mtl ) nodeMtlList[i].node->SetMaterial(mtl);
  }
  nodeMtlList.clear();

  // Load Camera
  camera.Init();
  camera.dir += camera.pos;
  TiXmlElement *camChild = cam->FirstChildElement();
  while ( camChild ) {
    if      ( COMPARE( camChild->Value(), "position"  ) ) ReadVector(camChild,camera.pos);
    else if ( COMPARE( camChild->Value(), "target"    ) ) ReadVector(camChild,camera.dir);
    else if ( COMPARE( camChild->Value(), "up"        ) ) ReadVector(camChild,camera.up);
    else if ( COMPARE( camChild->Value(), "fov"       ) ) ReadFloat (camChild,camera.fov);
    else if ( COMPARE( camChild->Value(), "width"     ) ) camChild->QueryIntAttribute("value", &camera.imgWidth);
    else if ( COMPARE( camChild->Value(), "height"    ) ) camChild->QueryIntAttribute("value", &camera.imgHeight);
    camChild = camChild->NextSiblingElement();
  }
  camera.dir -= camera.pos;
  camera.dir.Normalize();
  Point3 x = camera.dir ^ camera.up;
  camera.up = (x ^ camera.dir).GetNormalized();

  // renderImage.Init( camera.imgWidth, camera.imgHeight );

  return 1;
}

//-----------------------------------------------------------------------------

void PrintIndent(int level) { 
  if (!silentmode) {
    for ( int i=0; i<level; i++) printf("   "); 
  }
}

//-----------------------------------------------------------------------------

void LoadScene(TiXmlElement *element)
{
  for ( TiXmlElement *child = element->FirstChildElement(); child!=NULL; child = child->NextSiblingElement() ) {

    if ( COMPARE( child->Value(), "object" ) ) {
      LoadNode( &rootNode, child );
    } else if ( COMPARE( child->Value(), "material" ) ) {
      LoadMaterial( child );
    } else if ( COMPARE( child->Value(), "light" ) ) {
      LoadLight( child );
    }
  }
}

//-----------------------------------------------------------------------------

void LoadNode(Node *parent, TiXmlElement *element, int level)
{
  Node *node = new Node;
  parent->AppendChild(node);

  // name
  const char* name = element->Attribute("name");
  node->SetName(name);
  PrintIndent(level);
  if (!silentmode) {
    printf("object [");
    if ( name ) printf("%s",name);
    printf("]");
  }
  // material
  const char* mtlName = element->Attribute("material");
  if ( mtlName ) {
    if (!silentmode) printf(" <%s>", mtlName);
    NodeMtl nm;
    nm.node = node;
    nm.mtlName = mtlName;
    nodeMtlList.push_back(nm);
  }

  // type
  const char* type = element->Attribute("type");
  if ( type ) {
    if ( COMPARE(type,"sphere") ) {
      node->SetNodeObj( &theSphere );
      if (!silentmode) printf(" - Sphere");
    } else if ( COMPARE(type,"plane") ) {
      node->SetNodeObj( &thePlane );
      if (!silentmode) printf(" - Plane");
    } else if ( COMPARE(type,"obj") ) {
      if (!silentmode) printf(" - OBJ");
      Object *obj = objList.Find(name);
      if ( obj == NULL ) {	// object is not on the list, so we should load it now
	TriObj *tobj = new TriObj;
	if ( ! tobj->Load( name ) ) {
	  if (!silentmode) printf(" -- ERROR: Cannot load file \"%s.\"", name);
	  delete tobj;
	} else {
	  objList.Append(tobj,name);	// add to the list
	  obj = tobj;
	}
      }
      node->SetNodeObj( obj );
    } else {
      if (!silentmode) printf(" - UNKNOWN TYPE");
    }
  }


  if (!silentmode) printf("\n");


  for ( TiXmlElement *child = element->FirstChildElement(); child!=NULL; child = child->NextSiblingElement() ) {
    if ( COMPARE( child->Value(), "object" ) ) {
      LoadNode(node,child,level+1);
    }
  }
  LoadTransform( node, element, level );

}

//-----------------------------------------------------------------------------

void LoadTransform( Transformation *trans, TiXmlElement *element, int level )
{
  for ( TiXmlElement *child = element->FirstChildElement(); child!=NULL; child = child->NextSiblingElement() ) {
    if ( COMPARE( child->Value(), "scale" ) ) {
      Point3 s(1,1,1);
      ReadVector( child, s );
      trans->Scale(s.x,s.y,s.z);
      PrintIndent(level);
      if (!silentmode) printf("   scale %f %f %f\n",s.x,s.y,s.z);
    } else if ( COMPARE( child->Value(), "rotate" ) ) {
      Point3 s(0,0,0);
      ReadVector( child, s );
      s.Normalize();
      float a;
      ReadFloat(child,a,"angle");
      trans->Rotate(s,a);
      PrintIndent(level);
      if (!silentmode) printf("   rotate %f degrees around %f %f %f\n", a, s.x, s.y, s.z);
    } else if ( COMPARE( child->Value(), "translate" ) ) {
      Point3 t(0,0,0);
      ReadVector(child,t);
      trans->Translate(t);
      PrintIndent(level);
      if (!silentmode) printf("   translate %f %f %f\n",t.x,t.y,t.z);
    }
  }
}

//-----------------------------------------------------------------------------

void LoadMaterial(TiXmlElement *element)
{
  Material *mtl = NULL;

  // name
  const char* name = element->Attribute("name");
  if (!silentmode) {
    printf("Material [");
    if ( name ) printf("%s",name);
    printf("]");
  }

  // type
  const char* type = element->Attribute("type");
  if ( type ) {
    if ( COMPARE(type,"blinn") ) {
      if (!silentmode) printf(" - Blinn\n");
      MtlBlinn *m = new MtlBlinn();
      mtl = m;
      for ( TiXmlElement *child = element->FirstChildElement(); child!=NULL; child = child->NextSiblingElement() ) {
	Color c(1,1,1);
	float f=1;
	if ( COMPARE( child->Value(), "diffuse" ) ) {
	  ReadColor( child, c );
	  m->SetDiffuse(c);
	  if (!silentmode) printf("   diffuse %f %f %f\n",c.r,c.g,c.b);
	} else if ( COMPARE( child->Value(), "specular" ) ) {
	  ReadColor( child, c );
	  m->SetSpecular(c);
	  if (!silentmode) printf("   specular %f %f %f\n",c.r,c.g,c.b);
	} else if ( COMPARE( child->Value(), "glossiness" ) ) {
	  ReadFloat( child, f );
	  m->SetGlossiness(f);
	  if (!silentmode) printf("   glossiness %f\n",f);
	} else if ( COMPARE( child->Value(), "reflection" ) ) {
	  ReadColor( child, c );
	  m->SetReflection(c);
	  if (!silentmode) printf("   reflection %f %f %f\n",c.r,c.g,c.b);
	} else if ( COMPARE( child->Value(), "refraction" ) ) {
	  ReadColor( child, c );
	  m->SetRefraction(c);
	  ReadFloat( child, f, "index" );
	  m->SetRefractionIndex(f);
	  if (!silentmode) printf("   refraction %f %f %f (index %f)\n",c.r,c.g,c.b,f);
	} else if ( COMPARE( child->Value(), "absorption" ) ) {
	  ReadColor( child, c );
	  m->SetAbsorption(c);
	  if (!silentmode) printf("   absorption %f %f %f\n",c.r,c.g,c.b);
	}
      }

    } 
    else if ( COMPARE(type,"phong") ) {
      if (!silentmode) printf(" - Phong\n");
      MtlPhong *m = new MtlPhong();
      mtl = m;
      for ( TiXmlElement *child = element->FirstChildElement(); child!=NULL; child = child->NextSiblingElement() ) {
	Color c(1,1,1);
	float f=1;
	if ( COMPARE( child->Value(), "diffuse" ) ) {
	  ReadColor( child, c );
	  m->SetDiffuse(c);
	  if (!silentmode) printf("   diffuse %f %f %f\n",c.r,c.g,c.b);
	} else if ( COMPARE( child->Value(), "specular" ) ) {
	  ReadColor( child, c );
	  m->SetSpecular(c);
	  if (!silentmode) printf("   specular %f %f %f\n",c.r,c.g,c.b);
	} else if ( COMPARE( child->Value(), "glossiness" ) ) {
	  ReadFloat( child, f );
	  m->SetGlossiness(f);
	  if (!silentmode) printf("   glossiness %f\n",f);
	} else if ( COMPARE( child->Value(), "reflection" ) ) {
	  ReadColor( child, c );
	  m->SetReflection(c);
	  if (!silentmode) printf("   reflection %f %f %f\n",c.r,c.g,c.b);
	} else if ( COMPARE( child->Value(), "refraction" ) ) {
	  ReadColor( child, c );
	  m->SetRefraction(c);
	  ReadFloat( child, f, "index" );
	  m->SetRefractionIndex(f);
	  if (!silentmode) printf("   refraction %f %f %f (index %f)\n",c.r,c.g,c.b,f);
	} else if ( COMPARE( child->Value(), "absorption" ) ) {
	  ReadColor( child, c );
	  m->SetAbsorption(c);
	  if (!silentmode) printf("   absorption %f %f %f\n",c.r,c.g,c.b);

	}
      }
    } 
    else {
      if (!silentmode) printf(" - UNKNOWN\n");
    }	
  }

  if ( mtl ) {
    mtl->SetName(name);
    materials.push_back(mtl);
  }
}

//-----------------------------------------------------------------------------

void LoadLight(TiXmlElement *element)
{
  Light *light = NULL;

  // name
  const char* name = element->Attribute("name");
  if (!silentmode) {
    printf("Light [");
    if ( name ) printf("%s",name);
    printf("]");
  }

  // type
  const char* type = element->Attribute("type");
  if ( type ) {
    if ( COMPARE(type,"ambient") ) {
      if (!silentmode) printf(" - Ambient\n");
      AmbientLight *l = new AmbientLight();
      light = l;
      for ( TiXmlElement *child = element->FirstChildElement(); child!=NULL; child = child->NextSiblingElement() ) {
	if ( COMPARE( child->Value(), "intensity" ) ) {
	  Color c(1,1,1);
	  ReadColor( child, c );
	  l->SetIntensity(c);
	  if (!silentmode) printf("   intensity %f %f %f\n",c.r,c.g,c.b);
	}
      }
    } else if ( COMPARE(type,"direct") ) {
      if (!silentmode) printf(" - Direct\n");
      DirectLight *l = new DirectLight();
      light = l;
      for ( TiXmlElement *child = element->FirstChildElement(); child!=NULL; child = child->NextSiblingElement() ) {
	if ( COMPARE( child->Value(), "intensity" ) ) {
	  Color c(1,1,1);
	  ReadColor( child, c );
	  l->SetIntensity(c);
	  if (!silentmode) printf("   intensity %f %f %f\n",c.r,c.g,c.b);
	} else if ( COMPARE( child->Value(), "direction" ) ) {
	  Point3 v(1,1,1);
	  ReadVector( child, v );
	  l->SetDirection(v);
	  if (!silentmode) printf("   direction %f %f %f\n",v.x,v.y,v.z);
	}
      }
    } else if ( COMPARE(type,"point") ) {
      if (!silentmode) printf(" - Point\n");
      PointLight *l = new PointLight();
      light = l;
      for ( TiXmlElement *child = element->FirstChildElement(); child!=NULL; child = child->NextSiblingElement() ) {
	if ( COMPARE( child->Value(), "intensity" ) ) {
	  Color c(1,1,1);
	  ReadColor( child, c );
	  l->SetIntensity(c);
	  if (!silentmode) printf("   intensity %f %f %f\n",c.r,c.g,c.b);
	} else if ( COMPARE( child->Value(), "position" ) ) {
	  Point3 v(0,0,0);
	  ReadVector( child, v );
	  l->SetPosition(v);
	  if (!silentmode) printf("   position %f %f %f\n",v.x,v.y,v.z);
	}
      }
    } else {
      if (!silentmode) printf(" - UNKNOWN\n");
    }
  }

  if ( light ) {
    light->SetName(name);
    lights.push_back(light);
  }

}

//-----------------------------------------------------------------------------

void ReadVector(TiXmlElement *element, Point3 &v)
{
  double x = (double) v.x;
  double y = (double) v.y;
  double z = (double) v.z;
  element->QueryDoubleAttribute( "x", &x );
  element->QueryDoubleAttribute( "y", &y );
  element->QueryDoubleAttribute( "z", &z );
  v.x = (float) x;
  v.y = (float) y;
  v.z = (float) z;

  float f=1;
  ReadFloat( element, f );
  v *= f;
}

//-----------------------------------------------------------------------------

void ReadColor(TiXmlElement *element, Color &c)
{
  double r = (double) c.r;
  double g = (double) c.g;
  double b = (double) c.b;
  element->QueryDoubleAttribute( "r", &r );
  element->QueryDoubleAttribute( "g", &g );
  element->QueryDoubleAttribute( "b", &b );
  c.r = (float) r;
  c.g = (float) g;
  c.b = (float) b;

  float f=1;
  ReadFloat( element, f );
  c *= f;
}

//-----------------------------------------------------------------------------

void ReadFloat (TiXmlElement *element, float &f, const char *name)
{
  double d = (double) f;
  element->QueryDoubleAttribute( name, &d );
  f = (float) d;
}

//-----------------------------------------------------------------------------

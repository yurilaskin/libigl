#ifndef IGL_READMESH_H
#define IGL_READMESH_H

#include <string>
#include <vector>

namespace igl
{
  // load a tetrahedral volume mesh from a .mesh file
  //
  // Templates:
  //   Scalar  type for positions and vectors (will be read as double and cast
  //     to Scalar)
  //   Index  type for indices (will be read as int and cast to Index)
  // Input:
  //   mesh_file_name  path of .mesh file
  // Outputs:
  //   V  double matrix of vertex positions  #V by 3
  //   T  #T list of tet indices into vertex positions
  //   F  #F list of face indices into vertex positions
  template <typename Scalar, typename Index>
  inline bool readMESH(
    const std::string mesh_file_name,
    std::vector<std::vector<Scalar > > & V,
    std::vector<std::vector<Index > > & T,
    std::vector<std::vector<Index > > & F);

  // Input:
  //   mesh_file_name  path of .mesh file
  // Outputs:
  //   V  eigen double matrix #V by 3
  //   T  eigen int matrix #T by 4
  //   F  eigen int matrix #F by 3
  inline bool readMESH(
    const std::string str,
    Eigen::MatrixXd& V,
    Eigen::MatrixXi& T,
    Eigen::MatrixXi& F);
}

// Implementation
#include <cstdio>
#include <verbose.h>;

template <typename Scalar, typename Index>
inline bool igl::readMESH(
  const std::string mesh_file_name,
  std::vector<std::vector<Scalar > > & V,
  std::vector<std::vector<Index > > & T,
  std::vector<std::vector<Index > > & F)
{
  using namespace std;
  using namespace igl;
  FILE * mesh_file = fopen(mesh_file_name.c_str(),"r");
  if(NULL==mesh_file)
  {
    fprintf(stderr,"IOError: %s could not be opened...",mesh_file_name.c_str());
    return false;
  }
#ifndef LINE_MAX
#  define LINE_MAX 2048
#endif
  char line[LINE_MAX];
  bool still_comments;
  V.clear();
  T.clear();
  F.clear();

  // eat comments at beginning of file
  still_comments= true;
  while(still_comments)
  {
    fgets(line,LINE_MAX,mesh_file);
    still_comments = (line[0] == '#' || line[0] == '\n');
  }

  char str[LINE_MAX];
  sscanf(line," %s",str);
  // check that first word is MeshVersionFormatted
  if(0!=strcmp(str,"MeshVersionFormatted"))
  {
    fprintf(stderr,
      "Error: first word should be MeshVersionFormatted not %s\n",str);
    fclose(mesh_file);
    return false;
  }
  int one = -1;
  if(2 != sscanf(line,"%s %d",str,&one))
  {
    // 1 appears on next line?
    fscanf(mesh_file," %d",&one);
  }
  if(one != 1)
  {
    fprintf(stderr,"Error: second word should be 1 not %d\n",one);
    fclose(mesh_file);
    return false;
  }

  // eat comments
  still_comments= true;
  while(still_comments)
  {
    fgets(line,LINE_MAX,mesh_file);
    still_comments = (line[0] == '#' || line[0] == '\n');
  }

  sscanf(line," %s",str);
  // check that third word is Dimension
  if(0!=strcmp(str,"Dimension"))
  {
    fprintf(stderr,"Error: third word should be Dimension not %s\n",str);
    fclose(mesh_file);
    return false;
  }
  int three = -1;
  if(2 != sscanf(line,"%s %d",str,&three))
  {
    // 1 appears on next line?
    fscanf(mesh_file," %d",&three);
  }
  if(three != 3)
  {
    fprintf(stderr,"Error: only Dimension 3 supported not %d\n",three);
    fclose(mesh_file);
    return false;
  }

  // eat comments
  still_comments= true;
  while(still_comments)
  {
    fgets(line,LINE_MAX,mesh_file);
    still_comments = (line[0] == '#' || line[0] == '\n');
  }

  sscanf(line," %s",str);
  // check that fifth word is Vertices
  if(0!=strcmp(str,"Vertices"))
  {
    fprintf(stderr,"Error: fifth word should be Vertices not %s\n",str);
    fclose(mesh_file);
    return false;
  }
  size_t number_of_vertices;
  if(1 != fscanf(mesh_file," %ld",&number_of_vertices))
  {
    fprintf(stderr,"Error: expecting number of vertices...\n");
    fclose(mesh_file);
    return false;
  }
  // allocate space for vertices
  V.resize(number_of_vertices,vector<Scalar>(3,0));
  size_t extra;
  for(size_t i = 0;i<number_of_vertices;i++)
  {
    double x,y,z;
    if(4 != fscanf(mesh_file," %lg %lg %lg %ld",&x,&y,&z,&extra))
    {
      fprintf(stderr,"Error: expecting vertex position...\n");
      fclose(mesh_file);
      return false;
    }
    V[i][0] = x;
    V[i][1] = y;
    V[i][2] = z;
  }

  // eat comments
  still_comments= true;
  while(still_comments)
  {
    fgets(line,LINE_MAX,mesh_file);
    still_comments = (line[0] == '#' || line[0] == '\n');
  }

  sscanf(line," %s",str);
  // check that sixth word is Triangles
  if(0!=strcmp(str,"Triangles"))
  {
    fprintf(stderr,"Error: sixth word should be Triangles not %s\n",str);
    fclose(mesh_file);
    return false;
  }
  size_t number_of_triangles;
  if(1 != fscanf(mesh_file," %ld",&number_of_triangles))
  {
    fprintf(stderr,"Error: expecting number of triangles...\n");
    fclose(mesh_file);
    return false;
  }
  // allocate space for triangles
  F.resize(number_of_triangles,vector<Index>(3));
  // triangle indices
  size_t tri[3];
  for(size_t i = 0;i<number_of_triangles;i++)
  {
    if(4 != fscanf(mesh_file," %ld %ld %ld %ld",&tri[0],&tri[1],&tri[2],&extra))
    {
      printf("Error: expecting triangle indices...\n");
      return false;
    }
    for(size_t j = 0;j<3;j++)
    {
      F[i][j] = tri[j]-1;
    }
  }

  // eat comments
  still_comments= true;
  while(still_comments)
  {
    fgets(line,LINE_MAX,mesh_file);
    still_comments = (line[0] == '#' || line[0] == '\n');
  }

  sscanf(line," %s",str);
  // check that sixth word is Triangles
  if(0!=strcmp(str,"Tetrahedra"))
  {
    fprintf(stderr,"Error: seventh word should be Tetrahedra not %s\n",str);
    fclose(mesh_file);
    return false;
  }
  size_t number_of_tetrahedra;
  if(1 != fscanf(mesh_file," %ld",&number_of_tetrahedra))
  {
    fprintf(stderr,"Error: expecting number of tetrahedra...\n");
    fclose(mesh_file);
    return false;
  }
  // allocate space for tetrahedra
  T.resize(number_of_tetrahedra,vector<Index>(4));
  // tet indices
  size_t a,b,c,d;
  for(size_t i = 0;i<number_of_tetrahedra;i++)
  {
    if(5 != fscanf(mesh_file," %ld %ld %ld %ld %ld",&a,&b,&c,&d,&extra))
    {
      fprintf(stderr,"Error: expecting tetrahedra indices...\n");
      fclose(mesh_file);
      return false;
    }
    T[i][0] = a-1;
    T[i][1] = b-1;
    T[i][2] = c-1;
    T[i][3] = d-1;
  }
  fclose(mesh_file);
  return true;
}

#include <Eigen/Core>
#include "list_to_matrix.h"

inline bool igl::readMESH(
  const std::string str,
  Eigen::MatrixXd& V,
  Eigen::MatrixXi& T,
  Eigen::MatrixXi& F)
{
  std::vector<std::vector<double> > vV,vT,vF;
  bool success = igl::readMESH(str,vV,vT,vF);
  if(!success)
  {
    // readOBJ(str,vV,vTC,vN,vF,vFTC,vFN) should have already printed an error
    // message to stderr
    return false;
  }
  bool V_rect = igl::list_to_matrix(vV,V);
  if(!V_rect)
  {
    // igl::list_to_matrix(vV,V) already printed error message to std err
    return false;
  }
  bool T_rect = igl::list_to_matrix(vT,T);
  if(!T_rect)
  {
    // igl::list_to_matrix(vT,T) already printed error message to std err
    return false;
  }
  bool F_rect = igl::list_to_matrix(vF,F);
  if(!F_rect)
  {
    // igl::list_to_matrix(vF,F) already printed error message to std err
    return false;
  }
  assert(V.cols() == 3);
  assert(T.cols() == 4);
  assert(F.cols() == 3);
  return true;
}

#endif

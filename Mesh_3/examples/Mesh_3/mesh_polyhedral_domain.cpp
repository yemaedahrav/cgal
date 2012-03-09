#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#ifdef CONCURRENT_MESH_3
  #include <CGAL/Triangulation_lazy_ds_cell_base_3.h>
#endif

#include <CGAL/Polyhedral_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/refine_mesh_3.h>

// IO
#include <CGAL/IO/Polyhedron_iostream.h>

// Domain
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K> Polyhedron;
typedef CGAL::Polyhedral_mesh_domain_3<Polyhedron, K> Mesh_domain;

// Triangulation
#ifdef CONCURRENT_MESH_3
  typedef CGAL::Kernel_traits<Mesh_domain>::Kernel                        PMDKernel;
  typedef CGAL::details::Mesh_geom_traits_generator<PMDKernel>::type      Geom_traits;
  typedef CGAL::Triangulation_lazy_ds_cell_base_3<>                       DS_cell_base;
  typedef CGAL::Triangulation_cell_base_with_circumcenter_3<
            Geom_traits, DS_cell_base>                                    Cell_base_with_cc;
  typedef CGAL::Regular_triangulation_cell_base_3<
            Geom_traits, Cell_base_with_cc>                               Regular_cell_base;
  typedef CGAL::Mesh_triangulation_3<
              Mesh_domain,
              K,
              Geom_traits,
              Regular_cell_base>::type                                    Tr;
#else
  typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
#endif

typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;

// Criteria
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

// To avoid verbose function and named parameters call
using namespace CGAL::parameters;

int main()
{
  // Create input polyhedron
  Polyhedron polyhedron;
  std::ifstream input("data/elephant.off");
  input >> polyhedron;
   
  // Create domain
  Mesh_domain domain(polyhedron);
  
  // Mesh criteria (no cell_size set)
  Mesh_criteria criteria(facet_angle=25, facet_size=0.15, facet_distance=0.008,
                         cell_radius_edge_ratio=3);
  
  // Mesh generation
  C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, no_perturb(), no_exude());

  // Output
  std::ofstream medit_file("out_1.mesh");
  c3t3.output_to_medit(medit_file);
  medit_file.close();

  // Set tetrahedron size (keep cell_radius_edge_ratio), ignore facets
  Mesh_criteria new_criteria(cell_radius_edge_ratio=3, cell_size=0.03);

  // Mesh refinement
  CGAL::refine_mesh_3(c3t3, domain, new_criteria);

  // Output
  medit_file.open("out_2.mesh");
  c3t3.output_to_medit(medit_file);

  return 0;
}

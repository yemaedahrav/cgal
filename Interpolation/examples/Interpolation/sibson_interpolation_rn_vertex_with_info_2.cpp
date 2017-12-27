#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Regular_triangulation_2.h>

#include <CGAL/natural_neighbor_coordinates_2.h>
#include <CGAL/Interpolation_gradient_fitting_traits_2.h>
#include <CGAL/sibson_gradient_fitting.h>
#include <CGAL/interpolation_functions.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Interpolation_gradient_fitting_traits_2<K> Traits;

typedef K::FT                                            Coord_type;
typedef K::Weighted_point_2                              Point;
typedef K::Vector_2                                      Vector;

template <typename V, typename G>
struct Value_and_gradient {
  Value_and_gradient()
    : value(), gradient(CGAL::NULL_VECTOR)
  {}
  
  V value;
  G gradient;
};

typedef CGAL::Triangulation_vertex_base_with_info_2<Value_and_gradient<Coord_type,Vector>, K, CGAL::Regular_triangulation_vertex_base_2<K> >    Vb;
typedef CGAL::Regular_triangulation_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>     Tds;
typedef CGAL::Regular_triangulation_2<K,Tds>            Regular_triangulation;
typedef Regular_triangulation::Vertex_handle            Vertex_handle;


template <typename V, typename T>
struct Function_value {
  typedef V argument_type;
  typedef std::pair<T, bool> result_type;
  
  result_type operator()(const argument_type& a)const
  {
    return result_type(a->info().value, true);
  }
  
};


template <typename V, typename G>
struct Function_gradient
  : public std::iterator<std::output_iterator_tag,void,void,void,void> {
  
  typedef V argument_type;
  typedef std::pair<G,bool> result_type;
  
  
  result_type
  operator()(const argument_type& a)const
  {
    return std::make_pair(a->info().gradient,a->info().gradient != CGAL::NULL_VECTOR) ;
  }


  const Function_gradient& operator=(const std::pair<V, G>& p) const
    {
      p.first->info().gradient = p.second;
      return *this;
    }

  const Function_gradient& operator++(int) const
  {
    return *this;
  }
  
  const Function_gradient& operator*() const
  {
    return *this;
  }
};

int main()
{
  Regular_triangulation rt;

  Function_value<Vertex_handle,Coord_type> function_value;
  Function_gradient<Vertex_handle,Vector> function_gradient;
  
  //parameters for spherical function:
  Coord_type a(0.25), bx(1.3), by(-0.7), c(0.2);
  for (int y=0 ; y<4 ; y++){
    for (int x=0 ; x<4 ; x++){
      Point p(x,y);
      Vertex_handle vh = rt.insert(p);
      Coord_type value = a + bx* x+ by*y + c*(x*x+y*y);
      vh->info().value = value;
    }
  }

  sibson_gradient_fitting_rn_2(rt,
                               function_gradient,
                               function_value,
                               CGAL::Identity<std::pair<Vertex_handle, Vector> >(),
                               Traits());
  //coordinate computation
  Point p(1.6,1.4);
  std::vector< std::pair< Vertex_handle, Coord_type > > coords;
  typedef CGAL::Identity<std::pair< Vertex_handle, Coord_type > > Identity;
  Coord_type norm = CGAL::regular_neighbor_coordinates_2(rt,
                                                         p,
                                                         std::back_inserter(coords),
                                                         Identity()).second;


  //Sibson interpolant: version without sqrt:
  std::pair<Coord_type, bool> res = CGAL::sibson_c1_interpolation_square(coords.begin(),
                                                                         coords.end(),
                                                                         norm,
                                                                         p,
                                                                         function_value,
                                                                         function_gradient,
                                                                         Traits());
  
  if(res.second)
    std::cout << "Tested interpolation on " << p
              << " interpolation: " << res.first << " exact: "
              << a + bx * p.x()+ by * p.y()+ c*(p.x()*p.x()+p.y()*p.y())
              << std::endl;
  else
    std::cout << "C^1 Interpolation not successful." << std::endl
              << " not all function_gradients are provided."  << std::endl
              << " You may resort to linear interpolation." << std::endl;

  std::cout << "done" << std::endl;
  return 0;
}

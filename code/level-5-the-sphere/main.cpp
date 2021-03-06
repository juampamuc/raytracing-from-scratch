/**
 * Author: Nikolaus Mayer, 2018 (mayern@cs.uni-freiburg.de)
 */

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>


/// Define a vector class with constructor and operator
struct Vector
{
  /// Constructor
  Vector(float a=0.f, float b=0.f, float c=0.f)
    : x(a), y(b), z(c)
  { }

  /// Copy constructor
  Vector(const Vector& rhs)
    : x(rhs.x), y(rhs.y), z(rhs.z)
  { }

  /// Vector has three float attributes
  /// 
  union{float x; float r;};
  union{float y; float g;};
  union{float z; float b;};

  /// Vector addition
  Vector operator+(const Vector& r) const
  {
    return Vector{x+r.x,y+r.y,z+r.z};
  }

  /// Subtraction
  Vector operator-(const Vector& r) const
  {
    return *this + r*-1;
  }

  /// Unary minus / negation ("-vector")
  Vector operator-() const
  {
    return *this * -1;
  }

  /// Linear scaling
  Vector operator*(float r) const
  {
    return Vector{x*r,y*r,z*r};
  }

  /// Dot product with another Vector
  float operator%(Vector r) const
  {
    return x*r.x+y*r.y+z*r.z;
  }

  /// Cross-product with another Vector
  Vector operator^(Vector r) const
  {
    return Vector{y*r.z-z*r.y,
                  z*r.x-x*r.z,
                  x*r.y-y*r.x};
  }

  /// Vector length
  float length() const
  {
    return std::sqrt(*this % *this);
  }

  /// Normalization to unit length
  Vector operator!() const
  {
    return (*this)*(1./length());
  }
};


/// Abstract "raytraceable object" superclass
struct Object
{
  Object()
    : reflectivity(1),
      color(255,255,255)
  { }

  /// Check if object is hit by ray, and if yes, compute next ray
  ///
  /// incoming_ray_origin:    Origin 3D point of the incoming ray
  /// incoming_ray_direction: Direction vector of the incoming ray
  /// outgoing_ray_origin:    Origin 3D point of the reflected ray; i.e. the
  ///                         ray/object intersection/hit point
  /// outgoing_ray_direction: Direction vector of the outgoing/reflected ray
  /// hit_distance:           Distance the incoming ray has traveled from its
  ///                         origin to the hit point
  /// hit_color:              Object surface color at hit point
  /// object_reflectivity:    Ratio of incoming ray's energy reflected into the
  ///                         outgoing ray; rest is "absorbed" by the object
  virtual bool is_hit_by_ray(const Vector& incoming_ray_origin,
                             const Vector& incoming_ray_direction,
                             Vector& outgoing_ray_origin,
                             Vector& outgoing_ray_direction,
                             float& hit_distance,
                             Vector& hit_color,
                             float& object_reflectivity) const = 0;

  void set_reflectivity(float v)
  {
    reflectivity = v;
  };

  void set_color(const Vector& v)
  {
    color = v;
  };

  float reflectivity;
  Vector color;
};


/// A traceable triangle (base polygon); defined by 3 3D points
struct Triangle : Object
{
  Triangle(const Vector& p0,
           const Vector& p1,
           const Vector& p2)
    : p0(p0), p1(p1), p2(p2)
  {
    /// "u" and "v" vectors define the triangle (together with p0)
    ///
    ///       p1 
    ///       O   ^
    ///      /|   |
    ///     //|   | u
    ///    ///|   |
    ///   ////|   |
    ///  O----O   x
    ///  p2   p0
    ///
    ///  <----x
    ///    v
    ///
    u = p1-p0;
    v = p2-p0;
    normal = (v^u);
  }

  Vector p0, p1, p2;
  Vector u, v, normal;

  bool is_hit_by_ray(const Vector& incoming_ray_origin,
                     const Vector& incoming_ray_direction,
                     Vector& outgoing_ray_origin,
                     Vector& outgoing_ray_direction,
                     float& hit_distance,
                     Vector& hit_color,
                     float& object_reflectivity) const
  {
    if (!normal%-!incoming_ray_direction < 0)
      return false;

    const float& pox{p0.x};
    const float& poy{p0.y};
    const float& poz{p0.z};
    const float& ux{u.x};
    const float& uy{u.y};
    const float& uz{u.z};
    const float& vx{v.x};
    const float& vy{v.y};
    const float& vz{v.z};
    const float& rx{incoming_ray_direction.x};
    const float& ry{incoming_ray_direction.y};
    const float& rz{incoming_ray_direction.z};
    const float& ox{incoming_ray_origin.x};
    const float& oy{incoming_ray_origin.y};
    const float& oz{incoming_ray_origin.z};
    const float u_factor = (-(ox - pox)*(ry*vz - rz*vy) + (oy - poy)*(rx*vz - rz*vx) - (oz - poz)*(rx*vy - ry*vx))/(rx*uy*vz - rx*uz*vy - ry*ux*vz + ry*uz*vx + rz*ux*vy - rz*uy*vx);
    const float v_factor = ((ox - pox)*(ry*uz - rz*uy) - (oy - poy)*(rx*uz - rz*ux) + (oz - poz)*(rx*uy - ry*ux))/(rx*uy*vz - rx*uz*vy - ry*ux*vz + ry*uz*vx + rz*ux*vy - rz*uy*vx);
    const float ray_factor = (-(ox - pox)*(uy*vz - uz*vy) + (oy - poy)*(ux*vz - uz*vx) - (oz - poz)*(ux*vy - uy*vx))/(rx*uy*vz - rx*uz*vy - ry*ux*vz + ry*uz*vx + rz*ux*vy - rz*uy*vx);

    if (u_factor < 0 or
        v_factor < 0 or
        u_factor+v_factor > 1 or
        ray_factor < 0)
      return false;
    //hit_distance = ((p0+v*u_factor+u*v_factor)-outgoing_ray_origin).length();
    hit_distance = (incoming_ray_direction*ray_factor).length();
    if (hit_distance <= 1e-6)
      return false;

    /// Temporary normal vector
    const Vector n = normal;

    outgoing_ray_origin = p0+u*u_factor+v*v_factor;
    outgoing_ray_direction = !(incoming_ray_direction - !n*(incoming_ray_direction%!n)*2);
    hit_color = color;
    object_reflectivity = reflectivity;
    return true;
  }
};


struct Sphere : Object
{
  Sphere(const Vector& center,
         float radius)
    : center(center), radius(radius)
  { }

  const Vector center;
  const float radius;

  bool is_hit_by_ray(const Vector& incoming_ray_origin,
                     const Vector& incoming_ray_direction,
                     Vector& outgoing_ray_origin,
                     Vector& outgoing_ray_direction,
                     float& hit_distance,
                     Vector& hit_color,
                     float& object_reflectivity) const
  {
    const Vector p = incoming_ray_origin-center;
    const float b = p%incoming_ray_direction;
    const float c = p%p-radius*radius;

    //Does the ray hit the sphere ?
    if (b*b-c > 0) {
      //It does, compute the distance camera-sphere
      float s=-b-sqrt(radius*radius-p%p+b*b);

      if (s < 1e-3)
        return false;

      hit_distance=s;
      outgoing_ray_origin = incoming_ray_origin + incoming_ray_direction*hit_distance;
      const Vector normal = !(p+incoming_ray_direction*hit_distance);

      outgoing_ray_direction = !(incoming_ray_direction + !normal*(-incoming_ray_direction%!normal)*2);
      hit_color = color;
      object_reflectivity = reflectivity;

      return true;
    } else {
      return false;
    }
  }
};



Vector get_ground_color(const Vector& ray_origin,
                        const Vector& ray_direction)
{
  float distance = std::abs(ray_origin.y/ray_direction.y);
  float floor_hit_x = ray_origin.x + ray_direction.x*distance;
  float floor_hit_z = ray_origin.z + ray_direction.z*distance;
  if ((int)std::abs(std::floor(floor_hit_x))%2 == 
      (int)std::abs(std::floor(floor_hit_z))%2) 
  {
    return {255,0,0};
  } else {
    return {255,255,255};
  }
}


Vector get_sky_color(const Vector& ray_direction)
{
  return Vector{.7,.6,1}*255*std::pow(1-ray_direction.y,2);
}


int main(){

  /// LEFT-HANDED COORDINATE SYSTEM!
  /// FORWARDS vector (viewing direction)
  Vector ahead{0,0,1};
  /// RIGHT vector
  Vector right{.002,0,0};
  /// UP vector
  Vector up{0,.002,0};

  std::ofstream outfile("img.ppm");


  std::vector<Object*> scene_objects;

  /// 2 spheres
  scene_objects.push_back(new Sphere{Vector{1,2,0}, .5});
  scene_objects.back()->set_reflectivity(0.95);
  scene_objects.back()->set_color({0,0,0});
  scene_objects.push_back(new Sphere{Vector{-1.25,.8,0}, .25});
  scene_objects.back()->set_reflectivity(0.05);
  scene_objects.back()->set_color({255,0,0});

  /// Octahedron (8 triangles)
  /// Bottom half
  scene_objects.push_back(new Triangle{Vector{ 0, 0, 0},
                                       Vector{-1, 1, 0},
                                       Vector{ 0, 1, 1}});
  scene_objects.push_back(new Triangle{Vector{ 0, 0, 0},
                                       Vector{ 0, 1,-1},
                                       Vector{-1, 1, 0}});
  scene_objects.push_back(new Triangle{Vector{ 0, 0, 0},
                                       Vector{ 1, 1, 0},
                                       Vector{ 0, 1,-1}});
  scene_objects.push_back(new Triangle{Vector{ 0, 0, 0},
                                       Vector{ 0, 1, 1},
                                       Vector{ 1, 1, 0}});
  /// Top half
  scene_objects.push_back(new Triangle{Vector{ 0, 2, 0},
                                       Vector{ 0, 1, 1},
                                       Vector{-1, 1, 0}});
  scene_objects.push_back(new Triangle{Vector{ 0, 2, 0},
                                       Vector{ 1, 1, 0},
                                       Vector{ 0, 1, 1}});
  scene_objects.push_back(new Triangle{Vector{ 0, 2, 0},
                                       Vector{ 0, 1,-1},
                                       Vector{ 1, 1, 0}});
  scene_objects.push_back(new Triangle{Vector{ 0, 2, 0},
                                       Vector{-1, 1, 0},
                                       Vector{ 0, 1,-1}});

  const int max_hit_bounces{100};


  /// PPM header 
  /// The magic number "P6" means "color PPM in binary format (= not ASCII)"
  outfile << "P6 512 512 255 ";

  for(int y = 256; y >= -255; --y) {   //For each row
    for(int x = -255; x <= 256; ++x) {   //For each pixel in a row

      Vector color{0,0,0};

      Vector ray_origin{0,1,-4};
      /// Random offset on ray direction for antialiasing
      Vector ray_direction = !Vector{right*(x-0.5) + up*(y-0.5) + ahead};

      Vector ray_hit_at, 
             ray_bounced_direction, 
             hit_color;
      float distance_to_hit, 
            reflectivity_at_hit, 
            ray_energy_left=1;

      for (int bounce = 0; bounce <= max_hit_bounces; ++bounce) {
        /// Compute object intersections
        bool an_object_was_hit{false};
        float min_hit_distance{std::numeric_limits<float>::max()};
        Object* closest_object_ptr{nullptr};
        for (const auto& object : scene_objects) {
          if (object->is_hit_by_ray(ray_origin, ray_direction,
                                    ray_hit_at, ray_bounced_direction,
                                    distance_to_hit,
                                    hit_color,
                                    reflectivity_at_hit))
          {
            an_object_was_hit = true;

            if (distance_to_hit < min_hit_distance) {
              min_hit_distance = distance_to_hit;
              closest_object_ptr = object;
            }
          }
        }
        /// Compute color of hit object/ground/sky
        if (an_object_was_hit) {
          closest_object_ptr->is_hit_by_ray(ray_origin, ray_direction,
                                            ray_hit_at, ray_bounced_direction,
                                            distance_to_hit,
                                            hit_color,
                                            reflectivity_at_hit);
          ray_origin = ray_hit_at;
          ray_direction = ray_bounced_direction;
        } else {
          if (ray_direction.y < 0) {
            hit_color = get_ground_color(ray_origin, ray_direction);
            reflectivity_at_hit = 0;
          } else {
            hit_color = get_sky_color(ray_direction);
            reflectivity_at_hit = 0;
          }
        }


        color = color + (hit_color*(ray_energy_left*(1-reflectivity_at_hit)));
        ray_energy_left *= reflectivity_at_hit;
        if (ray_energy_left <= 0)
          break;
      }

      /// Write this pixel's RGB color triple (each a single byte)
      outfile << (unsigned char)(std::max(0.f, std::min(color.r, 255.f)))
              << (unsigned char)(std::max(0.f, std::min(color.g, 255.f)))
              << (unsigned char)(std::max(0.f, std::min(color.b, 255.f)));

    }
  }

  /// Image done
  outfile.close();

  /// Bye!
  return EXIT_SUCCESS;
}



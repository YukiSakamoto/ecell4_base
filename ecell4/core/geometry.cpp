#include "geometry.hpp"
#include <boost/math/quaternion.hpp>
#include <cmath>

namespace ecell4
{

Real3 rotate(const Real angle, const Real3& axis, const Real3& target)
{
    typedef boost::math::quaternion<Real> Quaternion;
    const Real cos_t(std::cos(angle * 0.5));
    const Real sin_t(std::sin(angle * 0.5));
    const Real sin_n(sin_t / length(axis));

    const Quaternion Q(cos_t, axis[0] * sin_n, axis[1] * sin_n, axis[2] * sin_n);
    const Quaternion P(0e0, target[0], target[1], target[2]);
    const Quaternion S(Q * P * boost::math::conj(Q));

    return Real3(S.R_component_2(), S.R_component_3(), S.R_component_4());
}

Real angle(const Real3& lhs, const Real3& rhs)
{
    const Real lensq_l = length_sq(lhs);
    const Real lensq_r = length_sq(rhs);
    const Real inner   = dot_product(lhs, rhs);
    const Real cosine  = inner / std::sqrt(lensq_l * lensq_r);
         if(cosine < -1.) return M_PI;
    else if(cosine > 1.)  return 0;
    else                  return std::acos(cosine);
}
}//ecell4

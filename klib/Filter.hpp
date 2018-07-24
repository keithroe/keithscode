
/// \file Util.hpp
/// Utility functions and macros

#ifndef LEGION_COMMON_MATH_FILTER_HPP_
#define LEGION_COMMON_MATH_FILTER_HPP_

#include <Legion/Common/Math/Vector.hpp>

namespace legion 
{

/// Warp the uniformly chosen 2D sample to fit a Box filter kernel.
/// \param in_sample  The input  sample in [0,2]^2
/// \returns          The warped sample in [-0.5,0.5]^2
Vector2 warpSampleByBoxFilter        ( const Vector2& in_sample );

/// Warp the uniformly chosen 2D sample to fit a Tent filter kernel.
///   \param in_sample  The input  sample in [0,2]^2
///   \returns          The warped sample in [-1,1]^2
Vector2 warpSampleByTentFilter       ( const Vector2& in_sample );

/// Warp the uniformly chosen 2D sample to fit a Cubic Spline filter kernel.
///   \param in_sample  The input  sample in [0,2]^2
///   \returns          The warped sample in [-2,2]^2
Vector2 warpSampleByCubicSplineFilter( const Vector2& in_sample );

}

#endif // LEGION_COMMON_MATH_FILTER_HPP_

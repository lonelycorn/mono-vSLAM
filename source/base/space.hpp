#pragma once
#include <base/math.hpp>
#include <iostream>

namespace mvSLAM
{

template <typename MeanType, typename CovarType>
class StateEstimate 
{
public:
    const MeanType &mean() const
    {
        return _mean;
    }

    MeanType &mean()
    {
        return _mean;
    }

    const CovarType &covar() const
    {
        return _covar;
    }

    CovarType &covar()
    {
        return _covar;
    }

    const CovarType &info() const
    {
        return _covar.inverse();
    }
    /**
     * @brief Ctor.
     * @note we may want to use information matrix instead
     */
    StateEstimate(const MeanType &mean, const CovarType &covar):
        _mean(mean), _covar(covar)
    {
    }
    /// Default ctor.
    StateEstimate():
        _mean(), _covar(CovarType::Identity())
    {
    }
    ~StateEstimate() {}
private:
    MeanType _mean;
    CovarType _covar;
};

using Pose = SE3;
using PoseUncertainty = Matrix6Type;
using PoseEstimate = StateEstimate<Pose, PoseUncertainty>;

using Point3 = Vector3Type;
using Point3Uncertainty = Matrix3Type;
using Point3Estimate = StateEstimate<Point3, Point3Uncertainty>;

using Point2 = Vector2Type;
using Point2Uncertainty = Matrix2Type;
using Point2Estimate = StateEstimate<Point2, Point2Uncertainty>;

}

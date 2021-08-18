#include "CeresRegistrator.hpp"
#include <ceres/ceres.h>
//TODO remove glog
#include <glog/logging.h>
#include "../Misc/log.hpp"
#include "../Misc/Base.hpp"
#include "../Math/EigenTransform.hpp"

using namespace tr;
using namespace ceres;

class NumDistanceTensorWrapper {
    const DistanceTensor& distanceTensor;
public:
    NumDistanceTensorWrapper(const DistanceTensor& distanceTensor) : distanceTensor(distanceTensor) {}
    bool operator()(const double* parameters, double* value) const {
        *value = double(distanceTensor.evaluate(parameters));
        return true;
    }
};

class DistanceTensorWrapper : public SizedCostFunction<1, 3> {
    const DistanceTensor& distanceTensor;
public:
    DistanceTensorWrapper(const DistanceTensor& distanceTensor) : distanceTensor(distanceTensor) {}
    virtual bool Evaluate(double const* const* parameters,
        double* residuals,
        double** jacobians) const {
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        const double indices[3] = {parameters[0][0],   //u
                                   parameters[0][1],   //v
                                   parameters[0][2]};  //angle
        if (jacobians == nullptr)
             *residuals = distanceTensor.evaluate(indices);
        else *residuals = distanceTensor.evaluate(indices, jacobians[0]);
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        return true;
    }
};

struct RasterPointCost {
    std::unique_ptr<CostFunctionToFunctor<1, 3> > distanceTensorWrapper;
    const mat4d P;
    const vec3d point, offsetPoint;
    const vec2d uv;

    RasterPointCost(const mat4d& P,
                    const RasterPoint& rasterPoint,
                    const DistanceTensor& distanceTensor) :
                    P(P),
                    point(rasterPoint.pos.cast<double>()),
                    offsetPoint(rasterPoint.offsetPos.cast<double>()),
                    uv(rasterPoint.uv.cast<double>()) {
        distanceTensorWrapper.reset(new ceres::CostFunctionToFunctor<1, 3>(
            new ceres::NumericDiffCostFunction<NumDistanceTensorWrapper
            ,ceres::CENTRAL,1,3>(new NumDistanceTensorWrapper(distanceTensor))));
        //distanceTensorWrapper.reset(new ceres::CostFunctionToFunctor<1, 3>(new DistanceTensorWrapper(distanceTensor)));
    }

    template <typename T>
    bool operator()(const T pos[3], const T ori[3], T residuals[1]) const {
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        T indices[3]; //(x, y), angle
        T ouv[2];   //offset (x, y)
        const T one(1), zero(0);
        if (project(pos, ori, point, indices, P) &&
            project(pos, ori, offsetPoint, ouv, P)) {
            const T dir[2] = { indices[0] - ouv[0], indices[1] - ouv[1] };
            if (dir[0] != zero) {
                indices[2] = ceres::atan(dir[1] / dir[0]);
                if (indices[2] < zero)
                    indices[2] += T(EIGEN_PI);
            }
            else indices[2] = T(EIGEN_PI / 2);
            (*distanceTensorWrapper)(indices, residuals);
        }
        else residuals[0] = T(0);
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        return true;
    }
};

tr::CeresRegistrator::CeresRegistrator(const mat4d& P) : Registrator(P) {
    google::InitGoogleLogging("Tracker");
}

SixDOF CeresRegistrator::registrate(const DistanceTensor& distanceTensor,
                                    const Template* candidate) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    double params[6];
    for (uint i = 0; i < 6; i++)
        params[i] = double(candidate->sixDOF.data[i]);
    ceres::Problem problem;
    
    for (const RasterPoint& rasterPoint : candidate->rasterPoints) {
        auto rast = new RasterPointCost(P, rasterPoint, distanceTensor);
        CostFunction* cost_function = new AutoDiffCostFunction<RasterPointCost, 1, 3, 3>(rast);
        problem.AddResidualBlock(cost_function,
                                 new HuberLoss(1.0),
                                 params,
                                 &params[3]);
    }

    // Run the solver!
    ceres::Solver::Options options;
    options.minimizer_progress_to_stdout = true;
    options.initial_trust_region_radius = 300;
    options.max_num_iterations = 100;
    //options.check_gradients = true;
    options.num_threads = 1;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    
    SixDOF result;
    for (uint i = 0; i < 6; i++)
        result.data[i] = float(params[i]);

    Logger::log(summary.FullReport());
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return result;
}

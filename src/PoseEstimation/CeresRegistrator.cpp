#include "CeresRegistrator.hpp"
#include <ceres/ceres.h>
//TODO remove glog
#include <glog/logging.h>
#include "../Misc/log.hpp"

using namespace tr;
using namespace ceres;

class DistanceTensorWrapper : public SizedCostFunction<1, 3> {
    const DistanceTensor& distanceTensor;
public:
    DistanceTensorWrapper(const DistanceTensor& distanceTensor) : distanceTensor(distanceTensor) {}
    virtual bool Evaluate(double const* const* parameters,
        double* residuals,
        double** jacobians) const {
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        const real indices[3] = {(real)parameters[0][0],   //u
                                 (real)parameters[0][1],   //v
                                 (real)parameters[0][2]};  //angle
        if (!jacobians)
            *residuals = double(distanceTensor.Evaluate(parameters[0]));
        else {
            real wrappedJacobians[3];
            *residuals = double(distanceTensor.Evaluate(parameters[0], wrappedJacobians));
            for (int i = 0u; i < 3u; i++)
                jacobians[0][i] = double(wrappedJacobians[i]);
        }

        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        return true;
    }
};

struct RasterPointCost {
    std::unique_ptr<CostFunctionToFunctor<1, 3> > distanceTensorWrapper;
    const emat4 P;
    const evec3 point, offsetPoint;
    const evec2 uv;

    RasterPointCost(const emat4& P, const RasterPoint& rasterPoint, const DistanceTensor& distanceTensor) :
        P(P),
        point(GLM2E<real>(rasterPoint.pos)),
        offsetPoint(GLM2E<real>(rasterPoint.offsetPos)),
        uv(GLM2E<real>(rasterPoint.uv)) {
        distanceTensorWrapper.reset(new CostFunctionToFunctor<1, 3>(new DistanceTensorWrapper(distanceTensor)));
    }

    template<typename T>
    inline void project(const T pos[3], const T ori[3], const evec3 p, T uv[2]) const {
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        Eigen::Quaternion<T> q = RPYToQ(ori);
        Eigen::Map<const Eigen::Matrix<T, 3, 1>> t(pos);
        const auto cam = q * p.cast<T>() + t;
        Eigen::Matrix<T, 3, 1> proj = (P.cast<T>() * cam.homogeneous()).hnormalized();
        const T one(1);
        const T two(2);
        for (uint i = 0; i < 2; i++)
            uv[i] = (proj[i] + one) / two;
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
    }

    template <typename T>
    bool operator()(const T pos[3], const T ori[3], T residuals[1]) const {
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        T indices[3]; //(x, y), angle
        T ouv[2];   //offset (x, y)
        project(pos, ori, point, indices);
        project(pos, ori, offsetPoint, ouv);
        T dir[2] = { indices[0] - ouv[0], indices[1] - ouv[1] };
        indices[2] = ceres::atan(dir[1] / dir[0]);
        if (indices[2] < T(0))
            indices[2] += T(EIGEN_PI);
        (*distanceTensorWrapper)(indices, residuals);
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        return true;
    }
};

tr::CeresRegistrator::CeresRegistrator(const emat4& P) : Registrator(P) {
    google::InitGoogleLogging("Tracker");
}

SixDOF CeresRegistrator::registrate(const DistanceTensor& distanceTensor, const Template* candidate) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    double params[6];
    for (uint i = 0; i < 6; i++)
        params[i] = double(candidate->sixDOF.data[i]);
    ceres::Problem problem;
    
    for (const RasterPoint& rasterPoint : candidate->rasterPoints) {
        auto rast = new RasterPointCost(P, rasterPoint, distanceTensor);
        CostFunction* cost_function = new AutoDiffCostFunction<RasterPointCost, 1, 3, 3>(rast);
        problem.AddResidualBlock(cost_function, new HuberLoss(1.0), params, &params[3]);
    }

    // Run the solver!
    ceres::Solver::Options options;
    options.minimizer_progress_to_stdout = true;
    options.check_gradients = true;
    options.num_threads = 1;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    
    SixDOF result;
    for (uint i = 0; i < 6; i++)
        result.data[i] = float(params[i]);
    delete[] params;

    Logger::log(summary.FullReport());
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return result;
}

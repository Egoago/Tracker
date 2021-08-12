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
        //TODO double <-> float??
        const float indices[3] = {(float)parameters[0][0],   //u
                                  (float)parameters[0][1],   //v
                                  (float)parameters[0][2]};  //angle
        if (!jacobians)
            *residuals = distanceTensor.at(indices);
        else
            *residuals = distanceTensor.at(indices, jacobians[0]);

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
        point(GLM2E(rasterPoint.pos)),
        offsetPoint(GLM2E(rasterPoint.offsetPos)),
        uv(GLM2E(rasterPoint.uv)) {
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
    SixDOF result;
    //The variable to solve for with its initial value. It will be
    //mutated in place by the solver.
    double pos[3];
    for (uint i = 0; i < 3; i++)
        pos[i] = (double)candidate->sixDOF.posData[i];
    double ori[3];
    for (uint i = 0; i < 3; i++)
        ori[i] = (double)candidate->sixDOF.orData[i];

    // Build the problem.
    ceres::Problem problem;

    // Set up the only cost function (also known as residual). This uses
    // auto-differentiation to obtain the derivative (jacobian).
    //for(unsigned int i = 0; i < candidate->rasterCount(); i++)
    for (const RasterPoint& rasterPoint : candidate->rasterPoints) {
        auto rast = new RasterPointCost(P, rasterPoint, distanceTensor);
        CostFunction* cost_function = new AutoDiffCostFunction<RasterPointCost, 1, 3, 3>(rast);
        problem.AddResidualBlock(cost_function, new HuberLoss(1.0), pos, ori);
    }

    // Run the solver!
    ceres::Solver::Options options;
    options.minimizer_progress_to_stdout = true;
    options.check_gradients = true;
    options.num_threads = 1;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    Logger::log(summary.FullReport());

    for (uint i = 0; i < 3; i++)
        result.posData[i] = (float)pos[i];
    for (uint i = 0; i < 3; i++)
        result.orData[i] = (float)ori[i];
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return result;
}

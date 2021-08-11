#include "CeresRegistrator.hpp"
#include <ceres/ceres.h>
//#include <glog/logging.h>
#include "../Misc/log.hpp"

using namespace tr;
using namespace ceres;

class DistanceTensorWrapper : public SizedCostFunction<1, 1> {
    const DistanceTensor& distanceTensor;
public:
    DistanceTensorWrapper(const DistanceTensor& distanceTensor) : distanceTensor(distanceTensor) {}
    virtual bool Evaluate(double const* const* parameters,
        double* residuals,
        double** jacobians) const {
        //TODO double <-> float??
        const float indices[3] = {(float)parameters[0][0],   //u
                                  (float)parameters[0][1],   //v
                                  (float)parameters[0][2]};  //angle
        if (!jacobians)
            *residuals = distanceTensor.at(indices);
        else
            *residuals = distanceTensor.at(indices, jacobians[0]);

        return true;
    }
};

struct RasterPointCost {
    std::unique_ptr<CostFunctionToFunctor<1, 1> > distanceTensorWrapper;
    const emat4 P;
    const evec3 point, offsetPoint;

    RasterPointCost(const emat4& P, const RasterPoint& rasterPoint, const DistanceTensor& distanceTensor) :
        P(P),
        point(GLM2E(rasterPoint.pos)),
        offsetPoint(GLM2E(rasterPoint.offsetPos)) {
        distanceTensorWrapper.reset(new CostFunctionToFunctor<1, 1>(new DistanceTensorWrapper(distanceTensor)));
    }

    template<typename T>
    inline void project(const T transf[6], const evec3& p, T uv[2]) const {
        Eigen::Quaternion<T> q = RPYToQ(&transf[3]);
        Eigen::Map<const Eigen::Matrix<T, 3, 1>> t(transf);
        const auto cam = q * p + t;
        Eigen::Matrix<T, 3, 1> proj = (P.cast<T>() * cam.homogeneous()).hnormalized();
        constexpr const T one(1);
        constexpr const T two(2);
        for (uint i = 0; i < 2; i++)
            uv[i] = (proj[i] + one) / two;
    }

    template <typename T>
    bool operator()(const T* transf, T* residuals) const {
        T indices[3]; //(x, y), angle
        T ouv[2];   //offset (x, y)
        project(transf, point, indices);
        project(transf, offsetPoint, ouv);
        T dir[2] = { indices[0] - ouv[0], indices[1] - ouv[1] };
        indices[2] = atan(dir[1] / dir[0]);
        (*distanceTensorWrapper)(indices, residuals);
        return true;
    }
};

SixDOF CeresRegistrator::registrate(const DistanceTensor& distanceTensor, const Template* candidate) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    //google::InitGoogleLogging(argv[0]);
    //The variable to solve for with its initial value. It will be
    //mutated in place by the solver.
     
    
    //double params[6];
    //for (uint i = 0; i < 6; i++)
    //    params[i] = candidate->sixDOF.data[i];

    //// Build the problem.
    //ceres::Problem problem;

    //// Set up the only cost function (also known as residual). This uses
    //// auto-differentiation to obtain the derivative (jacobian).
    ////for(unsigned int i = 0; i < candidate->rasterCount(); i++)
    //for (const RasterPoint& rasterPoint : candidate->rasterPoints) {
    //    auto rast = new RasterPointCost(P, rasterPoint, distanceTensor);
    //    CostFunction* cost_function =
    //        new AutoDiffCostFunction<RasterPointCost, 1, 1>(rast);
    //    problem.AddResidualBlock(cost_function, new HuberLoss(1.0), params);
    //}

    //// Run the solver!
    //ceres::Solver::Options options;
    //options.minimizer_progress_to_stdout = true;
    //ceres::Solver::Summary summary;
    //ceres::Solve(options, &problem, &summary);

    //Logger::log(summary.FullReport());

    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return candidate->sixDOF;
}

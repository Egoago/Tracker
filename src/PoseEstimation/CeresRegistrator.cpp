#include "CeresRegistrator.hpp"
#include <ceres/ceres.h>
//#include <glog/logging.h>
#include "../Misc/log.hpp"

using namespace tr;

//class DistanceTensorWrapper : public ceres::SizedCostFunction<1, 1> {
//public:
//    virtual bool Evaluate(double const* const* parameters,
//        double* residuals,
//        double** jacobians) const {
//        if (!jacobians) {
//            //ComputeDistortionValueAndJacobian(parameters[0][0], residuals, nullptr);
//        }
//        else {
//            //ComputeDistortionValueAndJacobian(parameters[0][0], residuals, jacobians[0]);
//        }
//        return true;
//    }
//};
//
//struct CustomCostFunction {
//    std::unique_ptr<ceres::CostFunctionToFunctor<1, 1> > distanceTensor;
//    CustomCostFunction() {
//        distanceTensor.reset(new ceres::CostFunctionToFunctor<1, 1>(new DistanceTensorWrapper));
//    }
//
//    template <typename T>
//    bool operator()(const T* theta, T* residuals) const {
//        const T q_0 = cos(theta[0]) * x[0] - sin(theta[0]) * x[1] + t[0];
//        const T q_1 = sin(theta[0]) * x[0] + cos(theta[0]) * x[1] + t[1];
//        const T r2 = q_0 * q_0 + q_1 * q_1;
//        T f;
//        (*distanceTensor)(&r2, &f);
//        residuals[0] = y[0] - f * q_0;
//        residuals[1] = y[1] - f * q_1;
//        return true;
//    }
//};

SixDOF CeresRegistrator::registrate(const DistanceTensor& distanceTensor, const Template* candidate) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    //google::InitGoogleLogging(argv[0]);
    // The variable to solve for with its initial value. It will be
    // mutated in place by the solver.
    //double params[6];
    //for (uint i = 0; i < 6; i++)
    //    params[i] = candidate->sixDOF.data[i];

    //// Build the problem.
    //ceres::Problem problem;

    //// Set up the only cost function (also known as residual). This uses
    //// auto-differentiation to obtain the derivative (jacobian).
    ////for(unsigned int i = 0; i < candidate->rasterCount(); i++)
    //for (auto& i : candidate->rasterPoints) {
    //    ceres::CostFunction* cost_function =
    //        new ceres::AutoDiffCostFunction<CustomCostFunction, 1, 1>(new CustomCostFunction);
    //    problem.AddResidualBlock(cost_function, new ceres::HuberLoss(1.0), params);
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

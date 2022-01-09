#include "CeresRegistrator.hpp"
#include <ceres/ceres.h>
//TODO remove glog
#include "../../Misc/log.hpp"
#include "../../Misc/Base.hpp"
#include "../../Misc/Constants.hpp"
#include "../../Math/EigenTransform.hpp"

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
    const vec3d point, offsetPoint, or_pos, or_dir;
    const vec2d uv;

    RasterPointCost(const mat4d& P,
                    const SixDOF pose,
                    const RasterPoint& rasterPoint,
                    const DistanceTensor& distanceTensor) :
                    P(P),
                    or_pos(pose.position.cast<double>()),
                    or_dir(pose.orientation.cast<double>()),
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
        if (renderPoint(pos, ori, point, indices, P) &&
            renderPoint(pos, ori, offsetPoint, ouv, P)) {
            const T dir[2] = { indices[0] - ouv[0], indices[1] - ouv[1] };
            if (dir[0] != zero) {
                indices[2] = ceres::atan(dir[1] / dir[0]);
                if (indices[2] < zero)
                    indices[2] += T(EIGEN_PI);
                if (indices[2] > T(EIGEN_PI))
                    indices[2] -= T(EIGEN_PI);
            }
            else indices[2] = T(EIGEN_PI / 2);
            (*distanceTensorWrapper)(indices, residuals);
            //residuals[0] *= (-pos[2])/T(1500.0);
            const Eigen::Map<const Eigen::Matrix<T, 3, 1>> p(pos);
            const Eigen::Map<const Eigen::Matrix<T, 3, 1>> o(ori);
            residuals[0] += T((or_pos.matrix().cast<T>() - p).squaredNorm() * 0.005);
            residuals[0] += T((or_dir.matrix().cast<T>() - o).squaredNorm() * 0.01);
        }
        else residuals[0] = T(1e10);
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        return true;
    }
};

struct RasterPointCostRot {
    std::unique_ptr<CostFunctionToFunctor<1, 3> > distanceTensorWrapper;
    const mat4d P;
    const vec3d point, offsetPoint, pos;
    const vec2d uv;

    RasterPointCostRot(const mat4d& P,
        const RasterPoint& rasterPoint,
        const DistanceTensor& distanceTensor,
        const vec3f pos1) :
        P(P),
        pos(pos1.cast<double>()),
        point(rasterPoint.pos.cast<double>()),
        offsetPoint(rasterPoint.offsetPos.cast<double>()),
        uv(rasterPoint.uv.cast<double>()) {
        distanceTensorWrapper.reset(new ceres::CostFunctionToFunctor<1, 3>(
            new ceres::NumericDiffCostFunction<NumDistanceTensorWrapper
            , ceres::CENTRAL, 1, 3>(new NumDistanceTensorWrapper(distanceTensor))));
        //distanceTensorWrapper.reset(new ceres::CostFunctionToFunctor<1, 3>(new DistanceTensorWrapper(distanceTensor)));
    }

    template <typename T>
    bool operator()(const T ori[3], T residuals[1]) const {
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        T indices[3]; //(x, y), angle
        T ouv[2];   //offset (x, y)
        const T one(1), zero(0);
        T position[3] = {T(pos.x()), T(pos.y()), T(pos.z())};
        if (renderPoint(position, ori, point, indices, P) &&
            renderPoint(position, ori, offsetPoint, ouv, P)) {
            const T dir[2] = { indices[0] - ouv[0], indices[1] - ouv[1] };
            if (dir[0] != zero) {
                indices[2] = ceres::atan(dir[1] / dir[0]);
                if (indices[2] < zero)
                    indices[2] += T(EIGEN_PI);
                if (indices[2] > T(EIGEN_PI))
                    indices[2] -= T(EIGEN_PI);
            }
            else indices[2] = T(EIGEN_PI / 2);
            (*distanceTensorWrapper)(indices, residuals);
            //residuals[0] *= (-position[2]) / T(1500.0);
        }
        else residuals[0] = T(1e10);
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        return true;
    }
};

struct RasterPointCostPos {
    std::unique_ptr<CostFunctionToFunctor<1, 3> > distanceTensorWrapper;
    const mat4d P;
    const vec3d point, offsetPoint, rot, or_pos;
    const vec2d uv;

    RasterPointCostPos(const mat4d& P,
        const RasterPoint& rasterPoint,
        const DistanceTensor& distanceTensor,
        const vec3f rot,
        const vec3f or_pos) :
        P(P),
        or_pos(or_pos.cast<double>()),
        rot(rot.cast<double>()),
        point(rasterPoint.pos.cast<double>()),
        offsetPoint(rasterPoint.offsetPos.cast<double>()),
        uv(rasterPoint.uv.cast<double>()) {
        distanceTensorWrapper.reset(new ceres::CostFunctionToFunctor<1, 3>(
            new ceres::NumericDiffCostFunction<NumDistanceTensorWrapper
            , ceres::CENTRAL, 1, 3>(new NumDistanceTensorWrapper(distanceTensor))));
        //distanceTensorWrapper.reset(new ceres::CostFunctionToFunctor<1, 3>(new DistanceTensorWrapper(distanceTensor)));
    }

    template <typename T>
    bool operator()(const T pos[3], T residuals[1]) const {
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        T indices[3]; //(x, y), angle
        T ouv[2];   //offset (x, y)
        const T one(1), zero(0);
        T ori[3] = { T(rot.x()), T(rot.y()), T(rot.z()) };
        if (renderPoint(pos, ori, point, indices, P) &&
            renderPoint(pos, ori, offsetPoint, ouv, P)) {
            const T dir[2] = { indices[0] - ouv[0], indices[1] - ouv[1] };
            if (dir[0] != zero) {
                indices[2] = ceres::atan(dir[1] / dir[0]);
                if (indices[2] < zero)
                    indices[2] += T(EIGEN_PI);
                if (indices[2] > T(EIGEN_PI))
                    indices[2] -= T(EIGEN_PI);
            }
            else indices[2] = T(EIGEN_PI / 2);
            (*distanceTensorWrapper)(indices, residuals);
            //residuals[0] *= (-pos[2]) / T(2500.0);
            const Eigen::Map<const Eigen::Matrix<T, 3, 1>> p(pos);
            residuals[0] += T((or_pos.matrix().cast<T>() - p).squaredNorm()*0.05);
        }
        else residuals[0] = T(1e10);
        //Logger::logProcess(__FUNCTION__);   //TODO remove logging
        return true;
    }
};

tr::CeresRegistrator::CeresRegistrator(const mat4d& P) : Registrator(P) {
    //google::InitGoogleLogging("Tracker");
}

void findOrientation(const DistanceTensor& distanceTensor, SixDOF& loc,const Template* candidate, const tr::mat4d P) {
    double params[3];
    for (uint i = 0; i < 3; i++)
        params[i] = double(loc.orData[i]);
    ceres::Problem problem;

    for (const RasterPoint& rasterPoint : candidate->rasterPoints) {
        auto rast = new RasterPointCostRot(P, rasterPoint, distanceTensor, loc.position);
        CostFunction* cost_function = new AutoDiffCostFunction<RasterPointCostRot, 1, 3>(rast);
        problem.AddResidualBlock(cost_function,
            new HuberLoss(10000.0),//ConfigParser::instance().getEntry(CONFIG_SECTION_REGISTRATION, "huber cutoff", 1.0)),
            params);
    }
    ceres::Solver::Options options;
    options.max_num_iterations = 20;
    options.num_threads = 8;
    options.linear_solver_type = ceres::DENSE_QR;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    for (uint i = 0; i < 3; i++)
        loc.orData[i] = float(params[i]);
}

float findPosition(const DistanceTensor& distanceTensor, SixDOF& loc, const Template* candidate, const tr::mat4d P) {
    double params[3];
    for (uint i = 0; i < 3; i++)
        params[i] = double(loc.posData[i]);
    ceres::Problem problem;

    for (const RasterPoint& rasterPoint : candidate->rasterPoints) {
        auto rast = new RasterPointCostPos(P, rasterPoint, distanceTensor, loc.orientation, loc.position);
        CostFunction* cost_function = new AutoDiffCostFunction<RasterPointCostPos, 1, 3>(rast);
        problem.AddResidualBlock(cost_function,
            new HuberLoss(10000.0),//ConfigParser::instance().getEntry(CONFIG_SECTION_REGISTRATION, "huber cutoff", 1.0)),
            params);
    }
    ceres::Solver::Options options;
    options.max_num_iterations = 20;
    options.num_threads = 8;
    options.linear_solver_type = ceres::DENSE_QR;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    for (uint i = 0; i < 3; i++)
        loc.posData[i] = float(params[i]);
    return summary.final_cost;
}

const Registrator::Registration CeresRegistrator::registrate(const DistanceTensor& distanceTensor, const Template* candidate) const {
    //Logger::logProcess(__FUNCTION__);   //TODO remove logging
    double params[6];
    for (uint i = 0; i < 6; i++)
        params[i] = double(candidate->sixDOF.data[i]);
    ceres::Problem problem;
    for (const RasterPoint& rasterPoint : candidate->rasterPoints) {
        auto rast = new RasterPointCost(P, candidate->sixDOF, rasterPoint, distanceTensor);
        CostFunction* cost_function = new AutoDiffCostFunction<RasterPointCost, 1, 3, 3>(rast);
        problem.AddResidualBlock(cost_function,
                                 new HuberLoss(10000.0),//ConfigParser::instance().getEntry(CONFIG_SECTION_REGISTRATION, "huber cutoff", 1.0)),
                                 params,
                                 &params[3]);
    }

    // Run the solver!
    ceres::Solver::Options options;
    //options.minimizer_progress_to_stdout = true;
    //options.initial_trust_region_radius = 300;
    options.max_num_iterations = 100;
    //options.check_gradients = true;
    //options.gradient_check_relative_precision = 1e-5;
    options.num_threads = 8;
    options.linear_solver_type = ceres::DENSE_QR;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    
    Registration result;
    for (uint i = 0; i < 6; i++)
        result.pose.data[i] = float(params[i]);
    result.finalLoss = (float)summary.final_cost;
    //Logger::log(summary.BriefReport());
    //Logger::logProcess(__FUNCTION__);   //TODO remove logging
    /*Registration result;
    result.pose = candidate->sixDOF;
    findOrientation(distanceTensor, result.pose, candidate, P);
    findPosition(distanceTensor, result.pose, candidate, P);
    findOrientation(distanceTensor, result.pose, candidate, P);
    result.finalLoss = findPosition(distanceTensor, result.pose, candidate, P);*/
    return result;
}

Registrator* tr::CeresRegistrator::clone() const {
    return new CeresRegistrator(*this);
}

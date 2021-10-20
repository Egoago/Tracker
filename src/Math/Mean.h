#pragma once
#include "Loss.h"
#include <numeric>

namespace tr {
    template<typename T>
    class Mean : public Loss<T> {
    protected:
        virtual inline T accumulate(T sum, T error) { return sum + error; }
    public:
        inline T loss(const std::vector<T>& errors) {
            return std::accumulate(errors.begin(), errors.end(), T(0), [this](T sum, T error) {return accumulate(sum, error); }) / (T)errors.size();
        }
        virtual inline Loss* clone() { return new Mean<T>(); }
    };

    template<typename T>
    class MSE : public Mean<T> {
    protected:
        inline T accumulate(T sum, T error) { return sum + error*error; }
        virtual inline Loss* clone() { return new MSE<T>(); }
    };

    template<typename T>
    class MAE : public Mean<T> {
    protected:
        inline T accumulate(T sum, T error) { return sum + abs(error); }
        virtual inline Loss* clone() { return new MAE<T>(); }
    };

    template<typename T>
    class Huber : public Mean<T> {
    private:
        const T c;
    protected:
        inline T accumulate(T sum, T error) {
            const T absError = abs(error);
            if (absError > c)
                return sum + c*(2*abs(error)-c);
            else return sum + error * error;
        }
    public:
        Huber(T cutOff = T(1)): c(abs(cutOff)) {}
        virtual inline Loss* clone() { return new Huber<T>(c); }
    };
}



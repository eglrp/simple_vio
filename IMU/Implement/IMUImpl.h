#ifndef IMUIMPL_H
#define IMUIMPL_H

#include "IMU/IMU.h"

#ifndef IMUTYPE_DEF_
#define IMUTYPE_DEF_

typedef IMUMeasure::ImuMeasureDeque ImuMeasureDeque;
typedef IMUMeasure::Transformation  Transformation;
typedef IMUMeasure::SpeedAndBias    SpeedAndBias;
typedef IMUMeasure::covariance_t    covariance_t;
typedef IMUMeasure::jacobian_t      jacobian_t;
typedef IMUMeasure::Error_t         Error_t;
typedef Eigen::Vector3d             speed_t;
typedef Eigen::Matrix<double, 6, 1> bias_t;

#endif // IMUTYPE_DEF_


class IMUImpl {
public:
    IMUImpl();
    virtual ~IMUImpl();

    virtual int propagation(const ImuMeasureDeque & imuMeasurements,
                    const ImuParameters & imuParams,
                    Transformation& T_WS,
                    SpeedAndBias & speedAndBiases,
                    okvis::Time & t_start,
                    okvis::Time & t_end,
                    covariance_t* covariance,
                    jacobian_t* jacobian) = 0;

    virtual int error(const IMU::pViFrame& frame_i, const IMU::pViFrame& frame_j, Error_t &err/* out */, void *info) = 0;
    virtual int repropagation();
    virtual int Jacobian(const Error_t& err, const IMU::pViFrame& frame_i, jacobian_t& jacobian_i, const IMU::pViFrame& frame_j, jacobian_t& jacobian_j, void *info) = 0;
};

#endif // IMUIMPL_H

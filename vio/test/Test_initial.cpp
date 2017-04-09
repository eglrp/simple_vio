//
// Created by lancelot on 4/6/17.
//
#include <qt4/QtGui/QApplication>
#include <QGLViewer/qglviewer.h>

#include "../Initialize.h"
#include "opencv2/ts.hpp"
#include "IO/camera/CameraIO.h"
#include "IO/image/ImageIO.h"
#include "IO/imu/IMUIO.h"
#include "DataStructure/imu/imuFactor.h"
#include "DataStructure/cv/Point.h"
#include "cv/Tracker/Tracker.h"
#include "cv/FeatureDetector/Detector.h"
#include "cv/Triangulater/Triangulater.h"

class drPoint {
public:
	drPoint(std::shared_ptr<Point>& point_) : point(point_) {}
	void draw() {
		glColor3d(0.3, 0.6, 0.9);
		point->pos_mutex.lock_shared();
		glVertex3d(point->pos_[0] * 100, point->pos_[1] * 100, point->pos_[2] * 100);
		point->pos_mutex.unlock_shared();
	}

private:
	std::shared_ptr<Point> point;
};

class drPose {
public:
	drPose(Sophus::SE3d pose_) : pose(pose_) {}
	void draw() {
		Eigen::Matrix3d rot = pose.so3().matrix();
		Eigen::Vector3d p1 = pose.translation();
		Eigen::Vector3d p2 = p1 + rot.block<3, 1>(0, 1);
		Eigen::Vector3d p3 = p1 + rot.block<3, 1>(0, 2);
		p1 += rot.block<3, 1>(0, 0);
		glColor3d(0, 0, 1);
		glVertex3d(p1[0], p1[1], p1[2]);
		glColor3d(1, 0, 1);
		glVertex3d(p2[0], p2[1], p2[2]);
		glVertex3d(p3[0], p3[1], p3[2]);
	}

private:
	Sophus::SE3d pose;
};

class Viewer : public QGLViewer {
public:
	Viewer() {}
	~Viewer() {}
	virtual void draw() {}
	virtual void animate() {}
	void pushPoint(std::shared_ptr<Point> point) {
		drPoints.push_back(drPoint(point));
	}

	void pushPose(std::shared_ptr<viFrame> viframe) {
		drPoses.push_back(drPose(viframe->getPose()));
	}

private:
	std::vector<drPose> drPoses;
	std::vector<drPoint> drPoints;
};

void testInitial(int argc, char **argv) {
	QApplication app(argc, argv);
	std::string imuDatafile("../testData/mav0/imu0/data.csv");
	std::string imuParamfile("../testData/mav0/imu0/sensor.yaml");
	IMUIO imuIO(imuDatafile, imuParamfile);
	IMUIO::pImuParam imuParam = imuIO.getImuParam();
	std::string camDatafile = "../testData/mav0/cam1/data.csv";
	std::string camParamfile = "../testData/mav0/cam1/sensor.yaml";
	CameraIO camIO(camDatafile, camParamfile);
	std::shared_ptr<AbstractCamera> cam = camIO.getCamera();
	std::string imageFile = "../testData/mav0/cam0/data.csv";
	ImageIO imageIO(imageFile, "../testData/mav0/cam0/data/", cam);
	Viewer viewer;

	std::shared_ptr<feature_detection::Detector>detector
			= std::make_shared<feature_detection::Detector>(752, 480, 25, IMG_LEVEL);
	std::shared_ptr<direct_tracker::Tracker> tracker = std::make_shared<direct_tracker::Tracker>();
	std::shared_ptr<Triangulater> triangulater = std::make_shared<Triangulater>();
	std::shared_ptr<IMU> imu = std::make_shared<IMU>();

	Initialize initer(detector, tracker, triangulater, imu);
	auto tmImg = imageIO.popImageAndTimestamp();
	std::pair<okvis::Time, cv::Mat> tmImgNext;
	std::shared_ptr<cvFrame> firstFrame = std::make_shared<cvFrame>(tmImg.second, cam, tmImg.first);
	initer.setFirstFrame(firstFrame);
	for(int i = 0; i < 10; ++i) {
		tmImgNext = imageIO.popImageAndTimestamp();
		std::shared_ptr<cvFrame> frame = std::make_shared<cvFrame>(tmImgNext.second, cam, tmImgNext.first);
		auto imuMeasure = imuIO.pop(tmImg.first, tmImgNext.first);
		Sophus::SE3d T;
		IMUMeasure::SpeedAndBias spbs;
		IMUMeasure::covariance_t var;
		IMUMeasure::jacobian_t jac;
		imu->propagation(imuMeasure, *imuParam, T, spbs, tmImg.first, tmImgNext.first, &var, &jac);
		imuFactor imufact(T, , spbs.block<3, 1>(0, 0));
		initer.pushcvFrame(frame, );
	}

	viewer.show();
	app.exec();
}
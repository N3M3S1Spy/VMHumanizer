#pragma once
#include "common/json.hpp"
#include "common/types.h"
#include <utility>
#include <vector>

namespace vmh {

class MouseBehavior {
public:
    MouseBehavior(const json& config);

    void moveTo(int targetX, int targetY, int durationMs = 500);
    std::pair<int, int> getCurrentPosition();

private:
    Point2D bezierPointQuadratic(const Point2D& p0, const Point2D& p1, const Point2D& p2, double t);
    Point2D bezierPointCubic(const Point2D& p0, const Point2D& p1, const Point2D& p2, const Point2D& p3, double t);
    Point2D applyJitter(double x, double y, double distance);
    double getSpeedMultiplier(double progress);
    Point2D generateControlPoint(const Point2D& start, const Point2D& end);
    std::vector<Point2D> generatePath(const Point2D& start, const Point2D& end, int steps);

    MouseConfig m_cfg;
};

} // namespace vmh

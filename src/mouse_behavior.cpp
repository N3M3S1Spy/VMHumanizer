#include "mouse_behavior.h"
#include "common/utils.h"
#include "windows/winapi_wrapper.h"
#include <cmath>
#include <algorithm>

namespace vmh {

MouseBehavior::MouseBehavior(const json& config) {
    if (config.contains("mouse")) {
        const auto& m = config["mouse"];
        std::string ct = m.value<std::string>("curve_type", "quadratic_bezier");
        m_cfg.curveType = (ct == "cubic_bezier") ? CurveType::CUBIC_BEZIER : CurveType::QUADRATIC_BEZIER;
        m_cfg.smoothness = m.value<double>("smoothness", 0.85);
        m_cfg.pauseOnTarget = m.value<bool>("pause_on_target", true);

        if (m.contains("jitter_amplitude") && m["jitter_amplitude"].isArray()) {
            m_cfg.jitterMin = static_cast<int>(m["jitter_amplitude"][0].getInt(2));
            m_cfg.jitterMax = static_cast<int>(m["jitter_amplitude"][1].getInt(5));
        }
        if (m.contains("pause_duration_range") && m["pause_duration_range"].isArray()) {
            m_cfg.pauseDurationMin = static_cast<int>(m["pause_duration_range"][0].getInt(100));
            m_cfg.pauseDurationMax = static_cast<int>(m["pause_duration_range"][1].getInt(500));
        }

        std::string sp = m.value<std::string>("speed_profile", "fast_then_slow");
        if (sp == "slow_start") m_cfg.speedProfile = SpeedProfile::SLOW_START;
        else if (sp == "uniform") m_cfg.speedProfile = SpeedProfile::UNIFORM;
        else m_cfg.speedProfile = SpeedProfile::FAST_THEN_SLOW;
    }
    logInfo("MouseBehavior initialized (curve=%s, smoothness=%.2f)",
            m_cfg.curveType == CurveType::CUBIC_BEZIER ? "cubic" : "quadratic",
            m_cfg.smoothness);
}

std::pair<int, int> MouseBehavior::getCurrentPosition() {
    return WinAPI::getMousePosition();
}

Point2D MouseBehavior::bezierPointQuadratic(const Point2D& p0, const Point2D& p1, const Point2D& p2, double t) {
    double u = 1.0 - t;
    return {
        u * u * p0.x + 2.0 * u * t * p1.x + t * t * p2.x,
        u * u * p0.y + 2.0 * u * t * p1.y + t * t * p2.y
    };
}

Point2D MouseBehavior::bezierPointCubic(const Point2D& p0, const Point2D& p1, const Point2D& p2, const Point2D& p3, double t) {
    double u = 1.0 - t;
    double uu = u * u;
    double uuu = uu * u;
    double tt = t * t;
    double ttt = tt * t;
    return {
        uuu * p0.x + 3.0 * uu * t * p1.x + 3.0 * u * tt * p2.x + ttt * p3.x,
        uuu * p0.y + 3.0 * uu * t * p1.y + 3.0 * u * tt * p2.y + ttt * p3.y
    };
}

Point2D MouseBehavior::applyJitter(double x, double y, double distance) {
    double amplitude = m_cfg.jitterMin + (m_cfg.jitterMax - m_cfg.jitterMin) *
                       std::min(1.0, distance / 500.0);
    double jx = randomDouble(-amplitude, amplitude);
    double jy = randomDouble(-amplitude, amplitude);
    return { x + jx, y + jy };
}

double MouseBehavior::getSpeedMultiplier(double progress) {
    switch (m_cfg.speedProfile) {
    case SpeedProfile::SLOW_START:
        // Ease-in: slow start, fast end
        return 0.3 + 1.7 * progress * progress;
    case SpeedProfile::FAST_THEN_SLOW:
        // Ease-out: fast start, slow approach
        return 2.0 * (1.0 - progress * progress) + 0.2;
    case SpeedProfile::UNIFORM:
    default:
        return 1.0;
    }
}

Point2D MouseBehavior::generateControlPoint(const Point2D& start, const Point2D& end) {
    double midX = (start.x + end.x) / 2.0;
    double midY = (start.y + end.y) / 2.0;
    double dx = end.x - start.x;
    double dy = end.y - start.y;
    double dist = std::sqrt(dx * dx + dy * dy);

    double offset = dist * (1.0 - m_cfg.smoothness) * 0.5;
    double perpX = -dy / (dist + 0.001);
    double perpY = dx / (dist + 0.001);

    double randOffset = randomDouble(-offset, offset);
    double randAlongPath = randomDouble(-dist * 0.2, dist * 0.2);

    return {
        midX + perpX * randOffset + (dx / dist) * randAlongPath,
        midY + perpY * randOffset + (dy / dist) * randAlongPath
    };
}

std::vector<Point2D> MouseBehavior::generatePath(const Point2D& start, const Point2D& end, int steps) {
    std::vector<Point2D> path;
    path.reserve(steps);

    Point2D cp1 = generateControlPoint(start, end);

    if (m_cfg.curveType == CurveType::CUBIC_BEZIER) {
        Point2D cp2 = generateControlPoint(start, end);
        for (int i = 0; i <= steps; ++i) {
            double t = static_cast<double>(i) / steps;
            path.push_back(bezierPointCubic(start, cp1, cp2, end, t));
        }
    } else {
        for (int i = 0; i <= steps; ++i) {
            double t = static_cast<double>(i) / steps;
            path.push_back(bezierPointQuadratic(start, cp1, end, t));
        }
    }

    return path;
}

void MouseBehavior::moveTo(int targetX, int targetY, int durationMs) {
    auto [curX, curY] = getCurrentPosition();
    Point2D start = { static_cast<double>(curX), static_cast<double>(curY) };
    Point2D end = { static_cast<double>(targetX), static_cast<double>(targetY) };

    double dx = end.x - start.x;
    double dy = end.y - start.y;
    double totalDist = std::sqrt(dx * dx + dy * dy);

    if (totalDist < 1.0) return;

    int steps = std::max(10, static_cast<int>(totalDist / 5.0));
    steps = std::min(steps, 200);

    auto path = generatePath(start, end, steps);

    double totalTimeMs = static_cast<double>(durationMs);
    double baseDelay = totalTimeMs / steps;

    for (int i = 1; i < static_cast<int>(path.size()); ++i) {
        double progress = static_cast<double>(i) / (path.size() - 1);
        double speedMult = getSpeedMultiplier(progress);

        Point2D pt = applyJitter(path[i].x, path[i].y, totalDist);

        int px = static_cast<int>(std::round(pt.x));
        int py = static_cast<int>(std::round(pt.y));
        px = std::max(0, std::min(px, GetSystemMetrics(SM_CXSCREEN) - 1));
        py = std::max(0, std::min(py, GetSystemMetrics(SM_CYSCREEN) - 1));

        WinAPI::sendMouseMove(px, py);

        double delay = baseDelay / (speedMult + 0.01);
        delay += randomDouble(-delay * 0.15, delay * 0.15);
        delay = std::max(1.0, delay);

        WinAPI::sleepMs(static_cast<int>(delay));
    }

    // Final position exactly on target
    WinAPI::sendMouseMove(targetX, targetY);

    if (m_cfg.pauseOnTarget) {
        int pauseMs = randomInt(m_cfg.pauseDurationMin, m_cfg.pauseDurationMax);
        double distFactor = std::max(0.3, 1.0 - totalDist / 1000.0);
        pauseMs = static_cast<int>(pauseMs * distFactor);
        WinAPI::sleepMs(pauseMs);
    }

    logDebug("Mouse moved to (%d, %d), distance=%.0f, steps=%d, duration=%dms",
             targetX, targetY, totalDist, steps, durationMs);
}

} // namespace vmh

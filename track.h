#ifndef TRACK_H
#define TRACK_H

#include <vector>

struct Point
{
	int x, y;
};

inline bool operator==(const Point& rHS, const Point& lHS) {
	return rHS.x == lHS.x && rHS.y == lHS.y;
}
inline bool operator!=(const Point& rHS, const Point& lHS) {
	return !(rHS == lHS);
}

typedef std::vector<Point> Track;

#endif //TRACK_H

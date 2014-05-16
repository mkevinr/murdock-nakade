#ifndef VEC2_H_
#define VEC2_H_

class Vec2
{
public:
	double x;
	double y;

	Vec2():x(0),y(0) {}

	Vec2(double x, double y):x(x),y(y) {}

	Vec2(double coordinate[2]):x(coordinate[0]),y(coordinate[1]) {}

	Vec2(const Vec2& other)
	{
		x = other.x;
		y = other.y;
	}

	Vec2 operator+(const Vec2 other)
	{
		return Vec2(other.x + x, other.y + y);
	}

	Vec2 operator-(const Vec2 other)
	{
		return Vec2(x - other.x, y - other.y);
	}

	Vec2 operator*(const double scalar)
	{
		return Vec2(x * scalar, y * scalar);
	}

	double dot(const Vec2 other)
	{
		return x * other.x + y * other.y;
	}

	void print()
	{
		printf("%g, %g", x, y);
	}
};

#endif

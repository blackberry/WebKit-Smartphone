/*
 * Copyright (C) 2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformPrimitives_h
#define BlackBerryPlatformPrimitives_h

namespace BlackBerry {
namespace Platform {

class IntSize {
public:
    IntSize(int width, int height) :
            m_width(width), m_height(height) {};
    IntSize() : m_width(0), m_height(0) {};

    bool isEmpty() const { return m_width < 1 || m_height < 1; }

    int width() const { return m_width; }
    int height() const { return m_height; }
    void setWidth(int width) { m_width = width; }
    void setHeight(int height) { m_height = height; }

private:
    int m_width;
    int m_height;
};

class IntPoint {
public:
    IntPoint(int x, int y) :
            m_x(x), m_y(y) {};
    IntPoint() : m_x(0), m_y(0) {};

    void translate(int dx, int dy) { m_x = m_x + dx; m_y = m_y + dy; }

    int x() const { return m_x; }
    int y() const { return m_y; }
    void setX(int x) { m_x = x; }
    void setY(int y) { m_y = y; }

private:
    int m_x;
    int m_y;
};

class IntRect {
public:
    IntRect(int x, int y, int width, int height) :
            m_location(x, y), m_size(width, height) {};
    IntRect(IntPoint location, IntSize size) :
            m_location(location), m_size(size) {};
    IntRect() : m_location(0, 0), m_size(0, 0) {};

    bool isEmpty() const { return m_size.isEmpty(); }

    void translate(int dx, int dy) { m_location.translate(dx, dy); }

    IntPoint location() const { return m_location; }
    void setLocation(const IntPoint& location) { m_location = location; }
    IntSize size() const { return m_size; }
    void setSize(const IntSize& size) { m_size = size; }

    int x() const { return m_location.x(); }
    int y() const { return m_location.y(); }
    int width() const { return m_size.width(); }
    int height() const { return m_size.height(); }

    void setX(int x) { m_location.setX(x); }
    void setY(int y) { m_location.setY(y); }
    void setWidth(int width) { m_size.setWidth(width); }
    void setHeight(int height) { m_size.setHeight(height); }

    int right() const { return x() + width(); }
    int bottom() const { return y() + height(); }

    void setRight(int right) { setWidth(right - x()); }
    void setBottom(int bottom) { setHeight(bottom - y()); }

    bool intersects(const IntRect&) const;
    bool contains(const IntRect&) const;

    // NOTE: The result is rounded to integer values, and thus may be not the exact center point.
    IntPoint center() const { return IntPoint(x() + width() / 2, y() + height() / 2); }

    bool contains(const IntPoint& point) const;
    void inflate(int dx, int dy);

private:
    IntPoint m_location;
    IntSize m_size;
};

inline bool operator==(const IntPoint& a, const IntPoint& b)
{
    return a.x() == b.x() && a.y() == b.y();
}

inline IntPoint operator+(const IntPoint& a, const IntPoint& b)
{
    return IntPoint(a.x() + b.x(), a.y() + b.y());
}

inline IntPoint operator-(const IntPoint& a, const IntPoint& b)
{
    return IntPoint(a.x() - b.x(), a.y() - b.y());
}

inline bool operator==(const IntSize& a, const IntSize& b)
{
    return a.width() == b.width() && a.height() == b.height();
}

inline bool operator==(const IntRect& a, const IntRect& b)
{
    return a.location() == b.location() && a.size() == b.size();
}

inline bool operator!=(const IntPoint& a, const IntPoint& b)
{
    return a.x() != b.x() || a.y() != b.y();
}

inline bool operator!=(const IntSize& a, const IntSize& b)
{
    return a.width() != b.width() || a.height() != b.height();
}

inline bool operator!=(const IntRect& a, const IntRect& b)
{
    return a.location() != b.location() || a.size() != b.size();
}

IntRect unionOfRects(const IntRect& a, const IntRect& b);


class FloatSize {
public:
    FloatSize(float width, float height) :
            m_width(width), m_height(height) {};
    FloatSize() : m_width(0), m_height(0) {};

    bool isEmpty() const { return m_width < 1 || m_height < 1; }

    float width() const { return m_width; }
    float height() const { return m_height; }
    void setWidth(float width) { m_width = width; }
    void setHeight(float height) { m_height = height; }

private:
    float m_width;
    float m_height;
};

class FloatPoint {
public:
    FloatPoint(float x, float y) :
            m_x(x), m_y(y) {};
    FloatPoint() : m_x(0), m_y(0) {};

    void translate(float dx, float dy) { m_x = m_x + dx; m_y = m_y + dy; }

    float x() const { return m_x; }
    float y() const { return m_y; }
    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }

private:
    float m_x;
    float m_y;
};

class FloatRect {
public:
    FloatRect(float x, float y, float width, float height) :
            m_location(x, y), m_size(width, height) {};
    FloatRect(FloatPoint location, FloatSize size) :
            m_location(location), m_size(size) {};
    FloatRect() : m_location(0, 0), m_size(0, 0) {};

    bool isEmpty() const { return m_size.isEmpty(); }

    void translate(float dx, float dy) { m_location.translate(dx, dy); }

    FloatPoint location() const { return m_location; }
    void setLocation(const FloatPoint& location) { m_location = location; }
    FloatSize size() const { return m_size; }
    void setSize(const FloatSize& size) { m_size = size; }

    float x() const { return m_location.x(); }
    float y() const { return m_location.y(); }
    float width() const { return m_size.width(); }
    float height() const { return m_size.height(); }

    void setX(float x) { m_location.setX(x); }
    void setY(float y) { m_location.setY(y); }
    void setWidth(float width) { m_size.setWidth(width); }
    void setHeight(float height) { m_size.setHeight(height); }

    // NOTE: The result is rounded to integer values, and thus may be not the exact center point.
    FloatPoint center() const { return FloatPoint(x() + width() / 2, y() + height() / 2); }

    bool contains(const FloatPoint& point) const;
    void inflate(float dx, float dy);

private:
    FloatPoint m_location;
    FloatSize m_size;
};

inline bool operator==(const FloatPoint& a, const FloatPoint& b)
{
    return a.x() == b.x() && a.y() == b.y();
}

inline FloatPoint operator+(const FloatPoint& a, const FloatPoint& b)
{
    return FloatPoint(a.x() + b.x(), a.y() + b.y());
}

inline FloatPoint operator-(const FloatPoint& a, const FloatPoint& b)
{
    return FloatPoint(a.x() - b.x(), a.y() - b.y());
}

inline bool operator==(const FloatSize& a, const FloatSize& b)
{
    return a.width() == b.width() && a.height() == b.height();
}

inline bool operator==(const FloatRect& a, const FloatRect& b)
{
    return a.location() == b.location() && a.size() == b.size();
}

inline bool operator!=(const FloatPoint& a, const FloatPoint& b)
{
    return a.x() != b.x() || a.y() != b.y();
}

inline bool operator!=(const FloatSize& a, const FloatSize& b)
{
    return a.width() != b.width() || a.height() != b.height();
}

inline bool operator!=(const FloatRect& a, const FloatRect& b)
{
    return a.location() != b.location() || a.size() != b.size();
}

FloatRect unionOfRects(const FloatRect& a, const FloatRect& b);

} // Platform
} // Olympia

#endif // BlackBerryPlatformPrimitives_h

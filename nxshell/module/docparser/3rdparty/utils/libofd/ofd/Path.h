#ifndef __OFD_PATH_H__
#define __OFD_PATH_H__

#include <memory>
#include <vector>
#include "ofd/Common.h"

namespace ofd{

    // **************** class Subpath ****************
    class Subpath{
        public:
            static SubpathPtr Instance(const Point_t &startPoint){
                return std::make_shared<Subpath>(startPoint);
            };

        public:
            Subpath(const Point_t &startPoint);
            Subpath(const Subpath* other);
            std::string to_string() const;

            // ================ Public Methods ================
        public:
            SubpathPtr Clone() const;

            void ClosePath();
            bool IsClosed() const{return m_bClosed;};

            size_t GetNumPoints() const{return m_points.size();};
            const Point_t& GetFirstPoint() const;
            const Point_t& GetLastPoint() const; 
            const Point_t& GetPoint(size_t idx) const;
            void SetPoint(size_t idx, const Point_t& point);

            void Offset(double dx, double dy);
            void LineTo(const Point_t& point);
            void CurveTo(const Point_t& p0, const Point_t& p1, const Point_t& p2);
            char GetFlag(size_t idx) const;
            Boundary CalculateBoundary() const;

        private:
            std::vector<Point_t> m_points;
            std::vector<char> m_flags;
            bool m_bClosed;

    }; // class Subpath

    // **************** class Path ****************
    class Path{
        public:
            static PathPtr Instance(){
                return std::make_shared<Path>();
            };

        public:
            Path();
            ~Path();
            std::string to_string() const;

            // ================ Public Methods ================
        public:
            void MoveTo(const Point_t& startPoint);
            void LineTo(const Point_t& point);
            void CurveTo(const Point_t& p0, const Point_t& p1, const Point_t& p2);
            void ClosePath();
            void Offset(double dx, double dy);
            void Append(const PathPtr otherPath);


            static PathPtr FromPathData(const std::string &pathData);
            std::string ToPathData() const;

            size_t GetNumSubpaths() const {return m_subpaths.size();};
            SubpathPtr GetSubpath(size_t idx) const {return m_subpaths[idx];};
            SubpathPtr GetLastSubpath() const;

            const Point_t& GetStartPoint() const {return m_startPoint;};
            Boundary CalculateBoundary() const;

        private:
            bool m_bJustMoved;
            Point_t m_startPoint;
            std::vector<SubpathPtr> m_subpaths;

    }; // class Path

}; // namespace ofd

#endif // __OFD_PATH_H__

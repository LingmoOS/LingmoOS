#include <assert.h>
#include <sstream>
#include <algorithm>

#include "ofd/Path.h"

#include "utils/utils.h"

using namespace ofd;

// **************** class Subpath ****************

Subpath::Subpath(const Point_t &startPoint) :
    m_bClosed(false){
    m_points.push_back(startPoint);
    m_flags.push_back('S');
}

Subpath::Subpath(const Subpath *other) :
    m_bClosed(other->m_bClosed){
    m_points.resize(other->m_points.size());
    std::copy(other->m_points.begin(), other->m_points.end(), m_points.begin());
    m_flags.resize(other->m_flags.size());
    std::copy(other->m_flags.begin(), other->m_flags.end(), m_flags.begin());
}

std::string Subpath::to_string() const{
    std::stringstream ss;
    size_t numPoints = GetNumPoints();
    ss << " numPoints:" << numPoints << " ";
    for ( size_t i = 0 ; i < numPoints ; i++){
        ss << "(" << m_points[i].X << "," << m_points[i].Y << ") ";
    }
    return ss.str();
}

Boundary Subpath::CalculateBoundary() const{
    Boundary boundary;
    size_t numPoints = m_points.size();
    if ( numPoints > 0 ){
        boundary.XMin = m_points[0].X;
        boundary.XMax = m_points[0].X;
        boundary.YMin = m_points[0].Y;
        boundary.YMax = m_points[0].Y;
        for ( size_t i = 1 ; i < numPoints ; i++ ){
            boundary.XMin = std::min(boundary.XMin, m_points[i].X);
            boundary.XMax = std::max(boundary.XMax, m_points[i].X);
            boundary.YMin = std::min(boundary.YMin, m_points[i].Y);
            boundary.YMax = std::max(boundary.YMax, m_points[i].Y);
        }
    }
    return boundary;
}

SubpathPtr Subpath::Clone() const{
    return std::make_shared<Subpath>(this);
}

const Point_t& Subpath::GetFirstPoint() const{
    assert(m_points.size() > 0);
    return m_points[0];
}

const Point_t& Subpath::GetLastPoint() const{
    assert(m_points.size() > 0);
    return m_points[m_points.size() - 1];
}

const Point_t& Subpath::GetPoint(size_t idx) const{
    assert(idx < m_points.size());
    return m_points[idx];
}

void Subpath::SetPoint(size_t idx, const Point_t& point){
    assert(idx < m_points.size());
    m_points[idx] = point;
}

void Subpath::ClosePath(){
    size_t numPoints = GetNumPoints();
    if ( numPoints < 1 ) return;
    //Point_t p0 = m_points[0];
    //Point_t p1 = m_points[numPoints - 1];
    //if ( p0 != p1 ){
    //if ( m_points[0] != m_points[numPoints - 1]){
        LineTo(m_points[0]);
    //}
    m_bClosed = true;
}

void Subpath::Offset(double dx, double dy){
    std::for_each(m_points.begin(), m_points.end(), 
            [=](Point_t& point){
                point.Offset(dx, dy);
            });
}

void Subpath::LineTo(const Point_t& point){
    m_points.push_back(point);
    m_flags.push_back('L');
}

void Subpath::CurveTo(const Point_t& p0, const Point_t& p1, const Point_t& p2){
    m_points.push_back(p0);
    m_points.push_back(p1);
    m_points.push_back(p2);
    m_flags.push_back('B');
    m_flags.push_back(' ');
    m_flags.push_back(' ');
}

char Subpath::GetFlag(size_t idx) const{
    return m_flags[idx];
}

// **************** class Path ****************

Path::Path() :
    m_bJustMoved(false){
}

Path::~Path(){
}

std::string Path::to_string() const {
    std::stringstream ss;
    size_t numSubpaths = GetNumSubpaths();
    ss << " Path: " << "numSubpaths:" << numSubpaths << " \n";
    for ( size_t idx = 0 ; idx < numSubpaths ; idx++){
        ss << "  subpath-" << idx << ": ";
        SubpathPtr subpath = GetSubpath(idx);
        if ( subpath != nullptr ){
            ss << subpath->to_string();
        }
        ss << "\n";
    }
    return ss.str();
}

Boundary Path::CalculateBoundary() const{
    Boundary boundary;

    size_t numSubpaths = m_subpaths.size();
    for ( size_t i = 0 ; i < numSubpaths ; i++ ){
        SubpathPtr subpath = GetSubpath(i);
        Boundary box = subpath->CalculateBoundary();
        if ( !box.IsEmpty() ){
            boundary.Union(box);
        }
    }

    return boundary;
}

// ======== Path::GetLastSubpath() ========
SubpathPtr Path::GetLastSubpath() const{
    size_t numSubpaths = GetNumSubpaths();
    if ( numSubpaths > 0 ){
        return m_subpaths[numSubpaths - 1];
    } else {
        return nullptr;
    }
}

// ======== Path::MoveTo() ========
void Path::MoveTo(const Point_t& startPoint){
    m_bJustMoved = true;
    m_startPoint = startPoint;
}

// ======== Path::LineTo() ========
void Path::LineTo(const Point_t& point){

    if ( m_bJustMoved ){
        SubpathPtr subPath = std::make_shared<Subpath>(m_startPoint);
        m_subpaths.push_back(subPath);
    } else {
        SubpathPtr lastSubpath = GetLastSubpath();
        assert(lastSubpath != nullptr );

        if (lastSubpath->IsClosed()){
            const Point_t& lastPoint = lastSubpath->GetLastPoint();
            SubpathPtr subPath = std::make_shared<Subpath>(lastPoint);
            m_subpaths.push_back(subPath);
        }
    }

    SubpathPtr lastSubpath = GetLastSubpath();
    lastSubpath->LineTo(point);
    m_bJustMoved = false;
}

// ======== Path::CurveTo() ========
void Path::CurveTo(const Point_t& p0, const Point_t& p1, const Point_t& p2){
    if ( m_bJustMoved ){
        SubpathPtr subPath = std::make_shared<Subpath>(m_startPoint);
        m_subpaths.push_back(subPath);
    } else {
        SubpathPtr lastSubpath = GetLastSubpath();
        assert(lastSubpath != nullptr );

        if (lastSubpath->IsClosed()){
            const Point_t& lastPoint = lastSubpath->GetLastPoint();
            SubpathPtr subPath = std::make_shared<Subpath>(lastPoint);
            m_subpaths.push_back(subPath);
        }
    }

    SubpathPtr lastSubpath = GetLastSubpath();
    lastSubpath->CurveTo(p0, p1, p2);
    m_bJustMoved = false;
}

// ======== Path::ClosePath() ========
void Path::ClosePath(){
    SubpathPtr lastSubpath = GetLastSubpath();
    if (lastSubpath == nullptr ) return;

    if ( m_bJustMoved ){
        SubpathPtr subPath = std::make_shared<Subpath>(m_startPoint);
        m_subpaths.push_back(subPath);
        m_bJustMoved = false;
    }

    lastSubpath = GetLastSubpath();
    lastSubpath->ClosePath();
}

// ======== Path::Offset() ========
void Path::Offset(double dx, double dy){
    std::for_each(m_subpaths.begin(), m_subpaths.end(),
            [=](SubpathPtr subPath){
                subPath->Offset(dx, dy);
            });
}

// ======== Path::Append() ========
void Path::Append(const PathPtr otherPath){
    std::for_each(otherPath->m_subpaths.begin(), otherPath->m_subpaths.end(),
            [=](SubpathPtr subPath){
                m_subpaths.push_back(subPath->Clone());
            });
    m_bJustMoved = false;
    m_startPoint.Clear();
}

// ======== Path::ToPathData() ========
std::string Path::ToPathData() const{
    std::stringstream ss;
    size_t numSubpaths = GetNumSubpaths();
    //LOG(DEBUG) << "AbbreviateData: numSubpaths:" << numSubpaths;
    if ( numSubpaths > 0 ){
        for ( size_t idx = 0 ; idx < numSubpaths ; idx++){
            SubpathPtr subpath = GetSubpath(idx);
            if ( subpath == nullptr ) continue;
            size_t numPoints = subpath->GetNumPoints();
            //LOG(DEBUG) << "AbbreviateData: numPoints:" << numPoints;
            if ( numPoints < 2 ) continue;

            const Point_t &startPoint = subpath->GetFirstPoint();
            if ( idx == 0 ){
                ss << "S " << startPoint.X << " " << startPoint.Y << " ";
            } else {
                ss << "M " << startPoint.X << " " << startPoint.Y << " ";
            }
            for ( size_t n = 1 ; n < numPoints ; n++ ){
                char flag = subpath->GetFlag(n);
                if ( flag == 'L' ){
                    const Point_t &p = subpath->GetPoint(n);
                    ss << "L " << p.X << " " << p.Y << " ";
                } else if ( flag == 'Q' ){
                    const Point_t &p1 = subpath->GetPoint(n);
                    const Point_t &p2 = subpath->GetPoint(n+1);
                    ss << "Q " << p1.X << " " << p1.Y << " " << p2.X << " " << p2.Y << " ";
                    n += 1;
                } else if ( flag == 'B' ) {
                    const Point_t &p1 = subpath->GetPoint(n);
                    const Point_t &p2 = subpath->GetPoint(n+1);
                    const Point_t &p3 = subpath->GetPoint(n+2);
                    ss << "B " << p1.X << " " << p1.Y << " " 
                               << p2.X << " " << p2.Y << " "
                               << p3.X << " " << p3.Y << " ";
                    n += 2;
                }
            }
            if ( subpath->IsClosed() ){
                ss << "C ";
            }
        }
    }

    return ss.str();
}

Point_t toPoint(const std::string& xString, const std::string& yString){
    double x = std::atof(xString.c_str());
    double y = std::atof(yString.c_str());
    return Point_t(x, y);
}

// ======== Path::FromPathData() ========
PathPtr Path::FromPathData(const std::string &pathData){
    bool ok = true;
    PathPtr path = Path::Instance();

    std::vector<std::string> tokens = utils::SplitString(pathData);
    size_t idx = 0;
    size_t numTokens = tokens.size();
    while ( numTokens > 0 ){
        std::string op = tokens[idx]; 
        numTokens--;
        idx++;
        if ( op == "L" ){
            // 线段连接
            if ( numTokens >= 2 ){
                Point_t point = toPoint(tokens[idx], tokens[idx+1]);
                path->LineTo(point);
                idx += 2;
                numTokens -= 2;
            } else {
//                LOG(WARNING) << "Not enough parameters in pathData tokens[" << idx << "]" << " pathData:" << pathData;
                ok = false;
                break;
            }
        } else if ( op == "M" ){
            // 移动到指定点
            if ( numTokens >= 2 ){
                Point_t point = toPoint(tokens[idx], tokens[idx+1]);
                path->MoveTo(point);
                idx += 2;
                numTokens -= 2;
            } else {
//                LOG(WARNING) << "Not enough parameters in pathData tokens[" << idx << "]" << " pathData:" << pathData;
                ok = false;
                break;
            }
        } else if ( op == "Q" ){
            // 二次贝塞尔曲线
            if ( numTokens >= 4 ){
                __attribute__((unused)) double x1 = std::atof(tokens[idx].c_str());
                __attribute__((unused)) double y1 = std::atof(tokens[idx+1].c_str());
                __attribute__((unused)) double x2 = std::atof(tokens[idx+2].c_str());
                __attribute__((unused)) double y2 = std::atof(tokens[idx+3].c_str());
                idx += 4;
                numTokens -= 4;
            } else {
//                LOG(WARNING) << "Not enough parameters in pathData tokens[" << idx << "]" << " pathData:" << pathData;
                ok = false;
                break;
            }
        } else if ( op == "B" ){
            // 三次贝塞尔曲线
            if ( numTokens >= 6 ){
                __attribute__((unused)) double x1 = std::atof(tokens[idx].c_str());
                __attribute__((unused)) double y1 = std::atof(tokens[idx+1].c_str());

                __attribute__((unused)) double x2 = std::atof(tokens[idx+2].c_str());
                __attribute__((unused)) double y2 = std::atof(tokens[idx+3].c_str());
                __attribute__((unused)) double x3 = std::atof(tokens[idx+4].c_str());
                __attribute__((unused)) double y3 = std::atof(tokens[idx+5].c_str());
                path->CurveTo(Point_t(x1,y1), Point_t(x2,y2), Point_t(x3,y3));
                idx += 6;
                numTokens -= 6;
            } else {
//                LOG(WARNING) << "Not enough parameters in pathData tokens[" << idx << "]" << " pathData:" << pathData;
                ok = false;
                break;
            }
        } else if ( op == "A" ){
            // 圆弧
            if ( numTokens >= 7 ){
                __attribute__((unused)) double rx = std::atof(tokens[idx].c_str());
                __attribute__((unused)) double ry = std::atof(tokens[idx+1].c_str());
                __attribute__((unused)) double angle = std::atof(tokens[idx+2].c_str());
                __attribute__((unused)) bool large = tokens[idx+3] == "0" ? false : true;
                __attribute__((unused)) bool sweep = tokens[idx+4] == "0" ? false : true;
                __attribute__((unused)) double x = std::atof(tokens[idx+5].c_str());
                __attribute__((unused)) double y = std::atof(tokens[idx+6].c_str());
                idx += 7;
                numTokens -= 7;
            } else {
//                LOG(WARNING) << "Not enough parameters in pathData tokens[" << idx << "]" << " pathData:" << pathData;
                ok = false;
                break;
            }
        } else if ( op == "S" ){
            if ( numTokens >= 2 ){
                Point_t point = toPoint(tokens[idx], tokens[idx+1]);
                path->MoveTo(point);
                idx += 2;
                numTokens -= 2;
            } else {
//                LOG(WARNING) << "Not enough parameters in pathData tokens[" << idx << "]" << " pathData:" << pathData;
                ok = false;
                break;
            }
        } else if ( op == "C" ){
            // 自动闭合
            path->ClosePath();
        }
    }

    if ( !ok ) {
        path = nullptr;
    }

    return path;
}


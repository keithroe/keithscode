
/*****************************************************************************\
 *                                                                           *
 *  filename : BBox.h                                                        *
 *  author   : R. Keith Morley                                               *
 *  last mod : 10/12/03                                                      *
 *                                                                           *
\*****************************************************************************/


#ifndef _BBOX_H_
#define _BBOX_H_ 1

#include <math/Vector3.h>

class BBox
{
public:
   BBox() {}

   BBox(const Vector3& a, const Vector3& b) : _min(a), _max(b) {}
   BBox(const BBox& b) : _min(b._min), _max(b._max) {}

   Vector3 min() const { return _min; }
   Vector3 max() const { return _max; }
   Vector3 mid() const { return (_max + _min) * 0.5; }

   bool overlapsBox(const BBox& box) const;

   bool containsPoint(const Vector3& point) const;
   
   void surround( const Vector3& point );
   void surround( const BBox& b );

   static BBox surround( const BBox& b, const Vector3& p );
   static BBox surround(const BBox& b1, const BBox& b2);

   Vector3 _min;
   Vector3 _max;
};


inline bool BBox::overlapsBox(const BBox& box) const
{
    return ( _min.x() <= box.max().x() && _max.x() >= box.min().x() &&
             _min.y() <= box.max().y() && _max.y() >= box.min().y() &&
             _min.z() <= box.max().z() && _max.z() >= box.min().z() );

}


inline bool BBox::containsPoint(const Vector3& p) const
{
    return ( p.x() >= _min.x() && p.x() <= _max.x() &&
             p.y() >= _min.y() && p.y() <= _max.y() && 
             p.z() >= _min.z() && p.z() <= _max.z() ); 

}


inline void BBox::surround(const Vector3& point) 
{
    _min[0] = point[0] < _min[0] ? point[0] : _min[0];
    _min[1] = point[1] < _min[1] ? point[1] : _min[1];
    _min[2] = point[2] < _min[2] ? point[2] : _min[2];
    _max[0] = point[0] > _max[0] ? point[0] : _max[0];
    _max[1] = point[1] > _max[1] ? point[1] : _max[1];
    _max[2] = point[2] > _max[2] ? point[2] : _max[2];
}


inline BBox BBox::surround( const BBox& b, const Vector3& p )
{
    BBox temp(b);
    temp.surround(p);
    return temp;
}


inline void BBox::surround( const BBox& b )
{
    _min = Vector3( _min.x() < b.min().x() ? _min.x() : b.min().x(),
                    _min.y() < b.min().y() ? _min.y() : b.min().y(),
                    _min.z() < b.min().z() ? _min.z() : b.min().z() );

    _max = Vector3( _max.x() > b.max().x() ? _max.x() : b.max().x(),
                    _max.y() > b.max().y() ? _max.y() : b.max().y(),
                    _max.z() > b.max().z() ? _max.z() : b.max().z() );
}


inline BBox BBox::surround(const BBox& b1, const BBox& b2)
{
    return BBox(
         Vector3( b1.min().x() < b2.min().x() ? b1.min().x() : b2.min().x(),
                  b1.min().y() < b2.min().y() ? b1.min().y() : b2.min().y(),
                  b1.min().z() < b2.min().z() ? b1.min().z() : b2.min().z() ),
         Vector3( b1.max().x() > b2.max().x() ? b1.max().x() : b2.max().x(),
                  b1.max().y() > b2.max().y() ? b1.max().y() : b2.max().y(),
                  b1.max().z() > b2.max().z() ? b1.max().z() : b2.max().z() ));
}

inline std::ostream& operator<<(std::ostream& out, const BBox& box) 
{
    return out << "[ " << box.min() << "  " << box.max() << " ]";
}



#endif // _BBOX_H_

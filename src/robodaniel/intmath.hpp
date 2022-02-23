#pragma once
#include <cstdlib>
#include <algorithm>

struct Vector2Int
{
	int x;
	int y;
};

struct Vector3Int
{
	int x;
	int y;
	int z;
};

inline Vector2Int Vector2IntZero()
{
	Vector2Int result = { 0, 0 };
	return result;
}

inline Vector2Int Vector2IntOne()
{
	Vector2Int result = { 1, 1 };
	return result;
}

inline Vector2Int Vector2IntAdd( const Vector2Int& v1, const Vector2Int& v2 )
{
	Vector2Int result = { v1.x + v2.x, v1.y + v2.y };
	return result;
}

inline Vector2Int Vector2IntAddValue( const Vector2Int& v, const int add )
{
	Vector2Int result = { v.x + add, v.y + add };
	return result;
}

inline Vector2Int Vector2IntSubtract( const Vector2Int& v1, const Vector2Int& v2 )
{
	Vector2Int result = { v1.x - v2.x, v1.y - v2.y };
	return result;
}

inline Vector2Int Vector2IntSubtractValue( const Vector2Int& v, const int sub )
{
	Vector2Int result = { v.x - sub, v.y - sub };
	return result;
}

inline int Vector2IntLengthSqr( const Vector2Int& v )
{
	int result = ( v.x * v.x ) + ( v.y * v.y );
	return result;
}

inline int Vector2IntDotProduct( const Vector2Int& v1, const Vector2Int& v2 )
{
	int result = ( v1.x * v2.x + v1.y * v2.y );
	return result;
}

inline Vector2Int Vector2IntScale( const Vector2Int& v, const int scale )
{
	Vector2Int result = { v.x * scale, v.y * scale };
	return result;
}

inline Vector2Int Vector2IntMultiply( const Vector2Int& v1, const Vector2Int& v2 )
{
	Vector2Int result = { v1.x * v2.x, v1.y * v2.y };
	return result;
}

inline Vector2Int Vector2IntNegate( const Vector2Int& v )
{
	Vector2Int result = { -v.x, -v.y };
	return result;
}

inline bool Vector2IntEqual( const Vector2Int& v1, const Vector2Int& v2 )
{
	return v1.x == v2.x && v1.y == v2.y;
}

inline Vector2 Vector2IntToFloat( const Vector2Int& v )
{
	return Vector2{ float( v.x ), float( v.y ) };
}

inline Vector3Int Vector3IntZero()
{
	Vector3Int result = { 0, 0, 0 };
	return result;
}

inline Vector3Int Vector3IntOne()
{
	Vector3Int result = { 1, 1, 1 };
	return result;
}

inline Vector3Int Vector3IntAdd( const Vector3Int& v1, const Vector3Int& v2 )
{
	Vector3Int result = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	return result;
}

inline Vector3Int Vector3IntAddValue( const Vector3Int& v, const int add )
{
	Vector3Int result = { v.x + add, v.y + add, v.z + add };
	return result;
}

inline Vector3Int Vector3IntSubtract( const Vector3Int& v1, const Vector3Int& v2 )
{
	Vector3Int result = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	return result;
}

inline Vector3Int Vector3IntSubtractValue( const Vector3Int& v, const int sub )
{
	Vector3Int result = { v.x - sub, v.y - sub, v.z - sub };
	return result;
}

inline Vector3Int Vector3IntScale( const Vector3Int& v, const int scalar )
{
	Vector3Int result = { v.x * scalar, v.y * scalar, v.z * scalar };
	return result;
}

inline Vector3Int Vector3IntMultiply( const Vector3Int& v1, const Vector3Int& v2 )
{
	Vector3Int result = { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
	return result;
}

inline Vector3Int Vector3IntCrossProduct( const Vector3Int& v1, const Vector3Int& v2 )
{
	Vector3Int result = { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
	return result;
}

inline Vector3Int Vector3IntPerpendicular( const Vector3Int& v )
{
	Vector3Int result = { 0 };

	int min = abs( v.x );
	Vector3Int cardinalAxis = { 1, 0, 0 };

	if ( abs( v.y ) < min )
	{
		min = abs( v.y );
		Vector3Int tmp = { 0, 1, 0 };
		cardinalAxis = tmp;
	}

	if ( abs( v.z ) < min )
	{
		Vector3Int tmp = { 0, 0, 1 };
		cardinalAxis = tmp;
	}

	result = Vector3IntCrossProduct( v, cardinalAxis );

	return result;
}

inline int Vector3IntLengthSqr( const Vector3Int& v )
{
	int result = v.x * v.x + v.y * v.y + v.z * v.z;
	return result;
}

inline int Vector3IntDotProduct( const Vector3Int& v1, const Vector3Int& v2 )
{
	int result = ( v1.x * v2.x + v1.y * v2.y + v1.z * v2.z );
	return result;
}

inline Vector3Int Vector3IntNegate( const Vector3Int& v )
{
	Vector3Int result = { -v.x, -v.y, -v.z };
	return result;
}

inline Vector3Int Vector3IntDivide( const Vector3Int& v1, const Vector3Int& v2 )
{
	Vector3Int result = { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
	return result;
}

inline Vector3Int Vector3IntMin( const Vector3Int& v1, const Vector3Int& v2 )
{
	Vector3Int result = { 0 };

	result.x = std::min<int>( v1.x, v2.x );
	result.y = std::min<int>( v1.y, v2.y );
	result.z = std::min<int>( v1.z, v2.z );

	return result;
}

inline Vector3Int Vector3IntMax( const Vector3Int& v1, const Vector3Int& v2 )
{
	Vector3Int result = { 0 };

	result.x = std::max<int>( v1.x, v2.x );
	result.y = std::max<int>( v1.y, v2.y );
	result.z = std::max<int>( v1.z, v2.z );

	return result;
}

inline bool Vector3IntEqual( const Vector3Int& v1, const Vector3Int& v2 )
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

inline Vector3 Vector3IntToFloat( const Vector3Int& v )
{
	return Vector3{ float( v.x ), float( v.y ), float( v.z ) };
}
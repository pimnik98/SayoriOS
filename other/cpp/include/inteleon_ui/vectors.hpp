#pragma once

namespace Inteleon {
	namespace Vectors {
		// 2D vector
		template<typename A, typename B>
		struct Vec2D {
			Vec2D(A _x, B _y) : x(_x), y(_y) {}
		
			A x;
			B y;
		};
		
		// 3D vector
		template<typename A, typename B, typename C>
		struct Vec3D {
			Vec3D(A _x, B _y, C _z) : x(_x), y(_y), z(_z) {}
		
			A x;
			B y;
			C z;
		};
		
		// 4D vector
		template<typename A, typename B, typename C, typename D>
		struct Vec4D {
			Vec4D(A _x, B _y, C _z, D _a)
			: x(_x), y(_y), z(_z), a(_a) {}
		
			A x;
			B y;
			C z;
			D a;
		};
	}
}

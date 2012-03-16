/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit
Please see our website at <http://pixhawk.ethz.ch>

(c) 2009 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

    PIXHAWK is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIXHAWK is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Definition of the Pixhawk matrix classes, they inherit from CvMat and should be 100% compatible with OpenCV functions.
 *
 *   @author Petri Tanskanen <mavteam@student.ethz.ch>
 *
 */

/** @addtogroup base */
/*@{*/


#ifndef _PX_MATRIX_H_
#define _PX_MATRIX_H_

#include <opencv/cv.h>
#include <PxVector3.h>

static inline void setSize( cv::Mat& m, int _dims, const int* _sz,
                            const size_t* _steps, bool autoSteps=false )
{
    size_t esz = CV_ELEM_SIZE(m.flags), total = esz;
    int i;
    for( i = _dims-1; i >= 0; i-- )
    {
        int s = _sz[i];
        CV_Assert( s >= 0 );
        m.size.p[i] = s;

        if( _steps )
            m.step.p[i] = i < _dims-1 ? _steps[i] : esz;
        else if( autoSteps )
        {
            m.step.p[i] = total;
            int64 total1 = (int64)total*s;
            total = (size_t)total1;
        }
    }
}

static inline void finalizeHdr(cv::Mat& m)
{
    int d = m.dims;
    if( m.data )
    {
        m.datalimit = m.datastart + m.size[0]*m.step[0];
        if( m.size[0] > 0 )
        {
            m.dataend = m.data + m.size[d-1]*m.step[d-1];
            for( int i = 0; i < d-1; i++ )
                m.dataend += (m.size[i] - 1)*m.step[i];
        }
        else
            m.dataend = m.datalimit;
    }
    else
        m.dataend = m.datalimit = 0;
}

/**
 * @brief OpenCV matrix wrapper for 3x3 matrices, manages its own data.
 *
 */
class PxMatrix3x3
{
public:
	/** @brief standard constructor */
	PxMatrix3x3(void) { }
	/** @brief copy constructor */
	PxMatrix3x3(const PxMatrix3x3 &mat) { for(int i=0; i<9; i++) local_data[i] = mat.local_data[i]; }
	/** @brief copy constructor from cv::Mat */
	PxMatrix3x3(const cv::Mat &mat) { for(int i=0; i<9; i++) local_data[i] = mat.at<float>(i); }
	/** @brief constructor given a 9-field C-array */
	PxMatrix3x3(const float *mat) { for(int i=0; i<9; i++) local_data[i] = mat[i]; }

	~PxMatrix3x3() { }

public:
	/** @name Access */
	/*@{*/
	/** @brief matrix-like access */
	float &operator() (const int row, const int col) { return local_data[col + row*3]; }
	/** @brief const Matrix-like access */
	float operator() (const int row, const int col) const { return local_data[col + row*3]; }
	/** @brief array-like access */
	float &operator[] (const int index) { return local_data[index]; }
	/** @brief const array-like access */
	float operator[] (const int index) const { return local_data[index]; }
	/*@}*/

public:
	/** @name Matrix-Matrix operators */
	/*@{*/
	/** @brief matrix-matrix multiplication */
	friend PxMatrix3x3 operator *(const PxMatrix3x3 &l, const PxMatrix3x3 &r)
	{ PxMatrix3x3 ret;
		ret.local_data[0] = l.local_data[0]*r.local_data[0]+l.local_data[1]*r.local_data[3]+l.local_data[2]*r.local_data[6];
		ret.local_data[1] = l.local_data[0]*r.local_data[1]+l.local_data[1]*r.local_data[4]+l.local_data[2]*r.local_data[7];
		ret.local_data[2] = l.local_data[0]*r.local_data[2]+l.local_data[1]*r.local_data[5]+l.local_data[2]*r.local_data[8];

		ret.local_data[3] = l.local_data[3]*r.local_data[0]+l.local_data[4]*r.local_data[3]+l.local_data[5]*r.local_data[6];
		ret.local_data[4] = l.local_data[3]*r.local_data[1]+l.local_data[4]*r.local_data[4]+l.local_data[5]*r.local_data[7];
		ret.local_data[5] = l.local_data[3]*r.local_data[2]+l.local_data[4]*r.local_data[5]+l.local_data[5]*r.local_data[8];

		ret.local_data[6] = l.local_data[6]*r.local_data[0]+l.local_data[7]*r.local_data[3]+l.local_data[8]*r.local_data[6];
		ret.local_data[7] = l.local_data[6]*r.local_data[1]+l.local_data[7]*r.local_data[4]+l.local_data[8]*r.local_data[7];
		ret.local_data[8] = l.local_data[6]*r.local_data[2]+l.local_data[7]*r.local_data[5]+l.local_data[8]*r.local_data[8];
		return ret; }
	/** @brief matrix-matrix subtraction */
	friend PxMatrix3x3 operator -(const PxMatrix3x3 &l, const PxMatrix3x3 &r) { PxMatrix3x3 ret; for(int i=0; i<9; i++) ret.local_data[i] = l.local_data[i] - r.local_data[i]; return ret; }
	/** @brief matrix-matrix multiplication with the possibility to transpose one or both matrices for the operation */
	friend PxMatrix3x3 multiplyTransposed(const PxMatrix3x3 &l, const PxMatrix3x3 &r)
	{ PxMatrix3x3 ret;
		ret.local_data[0] = l.local_data[0]*r.local_data[0]+l.local_data[1]*r.local_data[1]+l.local_data[2]*r.local_data[2];
		ret.local_data[1] = l.local_data[0]*r.local_data[3]+l.local_data[1]*r.local_data[4]+l.local_data[2]*r.local_data[5];
		ret.local_data[2] = l.local_data[0]*r.local_data[6]+l.local_data[1]*r.local_data[7]+l.local_data[2]*r.local_data[8];

		ret.local_data[3] = l.local_data[3]*r.local_data[0]+l.local_data[4]*r.local_data[1]+l.local_data[5]*r.local_data[2];
		ret.local_data[4] = l.local_data[3]*r.local_data[3]+l.local_data[4]*r.local_data[4]+l.local_data[5]*r.local_data[5];
		ret.local_data[5] = l.local_data[3]*r.local_data[6]+l.local_data[4]*r.local_data[7]+l.local_data[5]*r.local_data[8];

		ret.local_data[6] = l.local_data[6]*r.local_data[0]+l.local_data[7]*r.local_data[1]+l.local_data[8]*r.local_data[2];
		ret.local_data[7] = l.local_data[6]*r.local_data[3]+l.local_data[7]*r.local_data[4]+l.local_data[8]*r.local_data[5];
		ret.local_data[8] = l.local_data[6]*r.local_data[6]+l.local_data[7]*r.local_data[7]+l.local_data[8]*r.local_data[8];
		return ret; }
	/*@}*/

public:
	/** @name Matrix-Vector operators */
	/*@{*/
	/** @brief matrix-vector transformation */
	friend PxVector3 operator *(const PxMatrix3x3 &l, const PxVector3 &r)
	{ PxVector3 ret(l.local_data[0]*r[0]+l.local_data[1]*r[1]+l.local_data[2]*r[2],
					l.local_data[3]*r[0]+l.local_data[4]*r[1]+l.local_data[5]*r[2],
					l.local_data[6]*r[0]+l.local_data[7]*r[1]+l.local_data[8]*r[2]); return ret; }
	/** @brief matrix-vector transformation with transposed matrix */
	friend PxVector3 multiplyTransposed(const PxMatrix3x3 &l, const PxVector3 &r)
	{ PxVector3 ret(l.local_data[0]*r[0]+l.local_data[3]*r[1]+l.local_data[6]*r[2],
					l.local_data[1]*r[0]+l.local_data[4]*r[1]+l.local_data[7]*r[2],
					l.local_data[2]*r[0]+l.local_data[5]*r[1]+l.local_data[8]*r[2]); return ret; }
	/** @brief 3x3-matrix-vector transformation with array input */
	static PxVector3 multiply3x3(const float *mat, const PxVector3 &r)
	{ PxVector3 ret(mat[0]*r[0]+mat[1]*r[1]+mat[2]*r[2],
					mat[3]*r[0]+mat[4]*r[1]+mat[5]*r[2],
					mat[6]*r[0]+mat[7]*r[1]+mat[8]*r[2]); return ret; }
	/*@}*/

public:
	/** @name Matrix-Scalar operators */
	/*@{*/
	/** @brief multiplies all elements with a scalar value */
	friend PxMatrix3x3 operator *(const PxMatrix3x3 &l, const float r) { PxMatrix3x3 ret; for (int i=0; i<9; i++) ret.local_data[i] = l.local_data[i]*r; return ret; }
	/*@}*/

public:
	/** @name Non-altering operators */
	/*@{*/
	/** @brief loads the identity matrix */
	float getDeterminant(void) const { return 0; }//cvDet(this); }
	/*@}*/

public:
	/** @name Altering operators */
	/*@{*/
	/** @brief matrix-matrix assignment */
	void operator =(const PxMatrix3x3 &r) { for (int i=0; i<9; i++) local_data[i] = r.local_data[i]; }
	/** @brief matrix-matrix assignment (copy from cv::Mat) */
//	void operator =(const cv::Mat &r) { assert(r.dims == 2 && r.cols == 3 && r.rows == 3); for (int i=0; i<9; i++) local_data[i] = ((float *)r.data)[i]; }
	/** @brief loads the identity matrix */
	void setIdentity(void) { memset(local_data, 0, 9*sizeof(float)); local_data[0] = 1.f; local_data[4] = 1.f; local_data[8] = 1.f; }
//	/** @brief computes the inverse of the matrix */
//	void inverse(void) { cvInvert(this, this); }
	/** @brief computes the transpose of the matrix */
	void transpose(void) { std::swap(local_data[1], local_data[3]); std::swap(local_data[2], local_data[6]); std::swap(local_data[5], local_data[7]); }
//	/** @brief multiplies the matrix with its inverse (according to order) */
	void multiplySelfTransposed(const int order=0) { PxMatrix3x3 ret; PxMatrix3x3 a(*this); a.transpose(); if (order) ret = (*this)*a; else ret = a*(*this); (*this) = ret; }
	/*@}*/

private:
	float local_data[9];			///< matrix data
};

/**
 * @brief OpenCV matrix wrapper for 3x3 matrices, manages its own data.
 *
 */
class PxMatrix3x3Double
{
public:
	/** @brief standard constructor */
	PxMatrix3x3Double(void) { }
	/** @brief copy constructor */
	PxMatrix3x3Double(const PxMatrix3x3Double &mat) { for(int i=0; i<9; i++) local_data[i] = mat.local_data[i]; }
	/** @brief constructor given a 9-field C-array */
	PxMatrix3x3Double(const double *mat) { for(int i=0; i<9; i++) local_data[i] = mat[i]; }

public:
	/** @name Access */
	/*@{*/
	/** @brief matrix-like access */
	double &operator() (const int row, const int col) { return local_data[col + row*3]; }
	/** @brief const Matrix-like access */
	double operator() (const int row, const int col) const { return local_data[col + row*3]; }
	/** @brief array-like access */
	double &operator[] (const int index) { return local_data[index]; }
	/** @brief const array-like access */
	double operator[] (const int index) const { return local_data[index]; }
	/*@}*/

public:
	/** @name Matrix-Matrix operators */
	/*@{*/
	/** @brief matrix-matrix multiplication */
	friend PxMatrix3x3Double operator *(const PxMatrix3x3Double &l, const PxMatrix3x3Double &r)
	{ PxMatrix3x3Double ret;
		ret.local_data[0] = l.local_data[0]*r.local_data[0]+l.local_data[1]*r.local_data[3]+l.local_data[2]*r.local_data[6];
		ret.local_data[1] = l.local_data[0]*r.local_data[1]+l.local_data[1]*r.local_data[4]+l.local_data[2]*r.local_data[7];
		ret.local_data[2] = l.local_data[0]*r.local_data[2]+l.local_data[1]*r.local_data[5]+l.local_data[2]*r.local_data[8];

		ret.local_data[3] = l.local_data[3]*r.local_data[0]+l.local_data[4]*r.local_data[3]+l.local_data[5]*r.local_data[6];
		ret.local_data[4] = l.local_data[3]*r.local_data[1]+l.local_data[4]*r.local_data[4]+l.local_data[5]*r.local_data[7];
		ret.local_data[5] = l.local_data[3]*r.local_data[2]+l.local_data[4]*r.local_data[5]+l.local_data[5]*r.local_data[8];

		ret.local_data[6] = l.local_data[6]*r.local_data[0]+l.local_data[7]*r.local_data[3]+l.local_data[8]*r.local_data[6];
		ret.local_data[7] = l.local_data[6]*r.local_data[1]+l.local_data[7]*r.local_data[4]+l.local_data[8]*r.local_data[7];
		ret.local_data[8] = l.local_data[6]*r.local_data[2]+l.local_data[7]*r.local_data[5]+l.local_data[8]*r.local_data[8];
		return ret; }
	/** @brief matrix-matrix subtraction */
	friend PxMatrix3x3Double operator -(const PxMatrix3x3Double &l, const PxMatrix3x3Double &r) { PxMatrix3x3Double ret; cvSub(&l, &r, &ret); return ret; }
	/** @brief matrix-matrix multiplication with the possibility to transpose one or both matrices for the operation */
	friend PxMatrix3x3Double multiplyTransposed(const PxMatrix3x3Double &l, const PxMatrix3x3Double &r)
	{ PxMatrix3x3Double ret;
		ret.local_data[0] = l.local_data[0]*r.local_data[0]+l.local_data[1]*r.local_data[1]+l.local_data[2]*r.local_data[2];
		ret.local_data[1] = l.local_data[0]*r.local_data[3]+l.local_data[1]*r.local_data[4]+l.local_data[2]*r.local_data[5];
		ret.local_data[2] = l.local_data[0]*r.local_data[6]+l.local_data[1]*r.local_data[7]+l.local_data[2]*r.local_data[8];

		ret.local_data[3] = l.local_data[3]*r.local_data[0]+l.local_data[4]*r.local_data[1]+l.local_data[5]*r.local_data[2];
		ret.local_data[4] = l.local_data[3]*r.local_data[3]+l.local_data[4]*r.local_data[4]+l.local_data[5]*r.local_data[5];
		ret.local_data[5] = l.local_data[3]*r.local_data[6]+l.local_data[4]*r.local_data[7]+l.local_data[5]*r.local_data[8];

		ret.local_data[6] = l.local_data[6]*r.local_data[0]+l.local_data[7]*r.local_data[1]+l.local_data[8]*r.local_data[2];
		ret.local_data[7] = l.local_data[6]*r.local_data[3]+l.local_data[7]*r.local_data[4]+l.local_data[8]*r.local_data[5];
		ret.local_data[8] = l.local_data[6]*r.local_data[6]+l.local_data[7]*r.local_data[7]+l.local_data[8]*r.local_data[8];
		return ret; }
	/*@}*/

public:
	/** @name Matrix-Vector operators */
	/*@{*/
	/** @brief matrix-vector transformation */
	friend PxVector3 operator *(const PxMatrix3x3Double &l, const PxVector3 &r)
	{ PxVector3 ret(l.local_data[0]*r[0]+l.local_data[1]*r[1]+l.local_data[2]*r[2],
					l.local_data[3]*r[0]+l.local_data[4]*r[1]+l.local_data[5]*r[2],
					l.local_data[6]*r[0]+l.local_data[7]*r[1]+l.local_data[8]*r[2]); return ret; }
	/** @brief 3x3-matrix-vector transformation with array input */
	static PxVector3 multiply3x3(const double *mat, const PxVector3 &r)
	{ PxVector3 ret(mat[0]*r[0]+mat[1]*r[1]+mat[2]*r[2],
					mat[3]*r[0]+mat[4]*r[1]+mat[5]*r[2],
					mat[6]*r[0]+mat[7]*r[1]+mat[8]*r[2]); return ret; }
	/*@}*/

public:
	/** @name Matrix-Scalar operators */
	/*@{*/
	/** @brief multiplies all elements with a scalar value */
	friend PxMatrix3x3Double operator *(const PxMatrix3x3Double &l, const double r) { PxMatrix3x3Double ret; cvConvertScale(&l, &ret, r); return ret; }
	/*@}*/

public:
	/** @name Non-altering operators */
	/*@{*/
	/** @brief loads the identity matrix */
	double getDeterminant(void) const { return (double)(cvDet(this)); }
	/*@}*/

public:
	/** @name Altering operators */
	/*@{*/
	/** @brief matrix-matrix assignment */
	void operator =(const PxMatrix3x3Double &r) { for (int i=0; i<9; i++) local_data[i] = r.local_data[i]; }
	/** @brief loads the identity matrix */
	void setIdentity(void) { memset(local_data, 0, 9*sizeof(double)); local_data[0] = 1.f; local_data[4] = 1.f; local_data[8] = 1.f; }
//	/** @brief computes the inverse of the matrix */
//	void inverse(void) { cvInvert(this, this); }
	/** @brief computes the transpose of the matrix */
	void transpose(void) { std::swap(local_data[1], local_data[3]); std::swap(local_data[2], local_data[6]); std::swap(local_data[5], local_data[7]); }
//	/** @brief multiplies the matrix with its inverse (according to order) */
	void multiplySelfTransposed(const int order=0) { PxMatrix3x3Double ret; PxMatrix3x3Double a(*this); a.transpose(); if (order) ret = (*this)*a; else ret = a*(*this); (*this) = ret; }
	/*@}*/

private:
	double local_data[9];			///< matrix data
};

#endif //_PX_MATRIX_H_

/*@}*/

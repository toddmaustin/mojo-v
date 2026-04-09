/**
 kalman.cpp
 
 @author Tobias Ebsen
 @version 1.0.0 23/4/2018

 MIT License

 Copyright (c) 2020 Tobias Ebsen

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE. A very simple and versatile Kalman filter
 */

#include "utils.h"
// include build configuration defines
#include "../config.h"
#include "kalman.h"

KalmanModel::KalmanModel() {
    reset();
}

void KalmanModel::predict(VIP_ENCFLOAT dt, VIP_ENCFLOAT u, Vec2 Q) {
    
    // State transition model (Sometimes called A)
    Mat2x2 F = Mat2x2(
        1, -dt,
        0, 1
    );
    
    // Control-input model
    Vec2 B = Vec2(
        dt,
        0);

    // Predict current state
    x = F * x + B * u;

    // Transposed transition model
    Mat2x2 Ft = F.transposed();

    // Estimation of (pre) error covariance matrix
    P = F * P * Ft + Q * dt;
}

void KalmanModel::correct(VIP_ENCFLOAT z, VIP_ENCFLOAT R) {
    
    // Observation model
    Mat2x2 H = Mat2x2(
        1, 1,
        0, 0
    );
    
    // Transposed observation model
    Mat2x2 Ht = H.transposed();
    
    // Innovation covariance
    Mat2x2 Sm = H * P * Ht + R;
    VIP_ENCFLOAT S = Sm.m00;
    
    // Kalman gain
    Mat2x2 Km = P * Ht * (1/S);
    Vec2 K(Km.m00, Km.m10);
    
    // Update the state
    VIP_ENCFLOAT y = z - x[0];
    x = x + K * y;
    
    // Identity matrix
    Mat2x2 I = Mat2x2(
        1, 0,
        0, 1);
    
    // Post error covariance matrix
    P = (I - Km * H) * P;
}

void KalmanModel::reset() {
    x.x = 0;
    x.y = 0;
    P.m00 = 1.f;
    P.m01 = -1.f;
    P.m10 = -1.f;
    P.m11 = 1.f;
}

KalmanFilter::KalmanFilter() {
    pmodels = 0;
    nmodels = 0;
    Q.x = 0.01;
    Q.y = 0.01;
    R = 0.001;
}

KalmanFilter::KalmanFilter(unsigned int nmodels) : KalmanFilter() {
    init(nmodels);
}

KalmanFilter::~KalmanFilter() {
    if (pmodels) {
        delete pmodels;
        pmodels = nullptr;
    }
}

void KalmanFilter::predict(VIP_ENCFLOAT dt, VIP_ENCFLOAT * pu, unsigned int nu, unsigned int start) {
    for (unsigned int i=0; i<nu; i++) {
		if (i < nmodels)
        pmodels[start+i].predict(dt, pu ? pu[i] : 0, Q);
    }
}

void KalmanFilter::predict(VIP_ENCFLOAT dt) {
    predict(dt, 0, nmodels);
}

void KalmanFilter::correct(VIP_ENCFLOAT *pz, unsigned int nz, unsigned int start) {
    for (unsigned int i=0; i<nz; i++) {
        if (i < nmodels)
            pmodels[start+i].correct(pz[i], R);
    }
}

void KalmanFilter::correct(VIP_ENCFLOAT z, unsigned int istate) {
    if (istate < nmodels)
        pmodels[istate].correct(z, R);
}

void KalmanFilter::get(VIP_ENCFLOAT *px, unsigned int nx, unsigned int start) {
    for (unsigned int i=0; i<nx; i++) {
        if (i < nmodels)
            px[i] = pmodels[start+i].x[0];
    }
}

VIP_ENCFLOAT KalmanFilter::get(unsigned int istate) {
    return pmodels[istate].x[0];
}

void KalmanFilter::set(VIP_ENCFLOAT *px, unsigned int nx, unsigned int start) {
    for (unsigned int i=0; i<nx; i++) {
        if (i < nmodels)
            pmodels[start+i].x[0] = px[i];
    }
}

void KalmanFilter::reset() {
    for (unsigned int i=0; i<nmodels; i++) {
        pmodels[i].reset();
    }
}

void KalmanFilter::reset(unsigned int n, unsigned int start) {
    for (unsigned int i=start; i<start+n; i++)
        pmodels[i].reset();
}

void KalmanFilter::set(VIP_ENCFLOAT x, unsigned int istate) {
    pmodels[istate].x[0] = x;
}

Vec2::Vec2() {
    this->x = 0;
    this->y = 0;
}

Vec2::Vec2(VIP_ENCFLOAT x, VIP_ENCFLOAT y) {
    this->x = x;
    this->y = y;
}

Vec2 Vec2::operator*(const VIP_ENCFLOAT &scalar) const {
    return Vec2(x * scalar, y * scalar);
}

Vec2 Vec2::operator*(const Vec2 &other) const {
    return Vec2(x * other.x, y * other.y);
}

Vec2 Vec2::operator+(const Vec2 &other) const {
    return Vec2(x + other.x, y + other.y);
}

VIP_ENCFLOAT & Vec2::operator[](int index) {
    return index == 0 ? x : y;
}

Mat2x2::Mat2x2() {
    this->m00 = 0;
    this->m01 = 0;
    this->m10 = 0;
    this->m11 = 0;
}

Mat2x2::Mat2x2(VIP_ENCFLOAT m00, VIP_ENCFLOAT m01, VIP_ENCFLOAT m10, VIP_ENCFLOAT m11) {
    this->m00 = m00;
    this->m01 = m01;
    this->m10 = m10;
    this->m11 = m11;
}

Mat2x2 Mat2x2::transposed() {
    return Mat2x2(m00, m10, m01, m11);
}

Mat2x2 Mat2x2::inversed() {
    VIP_ENCFLOAT det = 1 / (m00 * m11 - m01 * m10);
    return Mat2x2(m11, -m01, -m10, m00) * det;
}

Mat2x2 Mat2x2::operator*(const Mat2x2 &other) const {
    Mat2x2 m;
    m.m00 = this->m00 * other.m00 + this->m01 * other.m10;
    m.m01 = this->m00 * other.m01 + this->m01 * other.m11;
    m.m10 = this->m10 * other.m00 + this->m11 * other.m10;
    m.m11 = this->m10 * other.m01 + this->m11 * other.m11;
    return m;
}

Vec2 Mat2x2::operator*(const Vec2 &other) const {
    Vec2 v;
    v.x = this->m00 * other.x + this->m01 * other.y;
    v.y = this->m10 * other.x + this->m11 * other.y;
    return v;
}

Mat2x2 Mat2x2::operator * (const VIP_ENCFLOAT & scalar) const {
    Mat2x2 m;
    m.m00 = this->m00 * scalar;
    m.m01 = this->m01 * scalar;
    m.m10 = this->m10 * scalar;
    m.m11 = this->m11 * scalar;
    return m;
}

Mat2x2 Mat2x2::operator+(const Vec2 &other) const {
    Mat2x2 m;
    m.m00 = this->m00 + other.x;
    m.m01 = this->m01 + other.x;
    m.m10 = this->m10 + other.y;
    m.m11 = this->m11 + other.y;
    return m;
}

Mat2x2 Mat2x2::operator+(const VIP_ENCFLOAT &scalar) const {
    Mat2x2 m;
    m.m00 = this->m00 + scalar;
    m.m01 = this->m01 + scalar;
    m.m10 = this->m10 + scalar;
    m.m11 = this->m11 + scalar;
    return m;
}

Mat2x2 Mat2x2::operator-(const Mat2x2 &other) const {
    Mat2x2 m;
    m.m00 = this->m00 - other.m00;
    m.m01 = this->m01 - other.m01;
    m.m10 = this->m10 - other.m10;
    m.m11 = this->m11 - other.m11;
    return m;
}


/*        Generated by TAPENADE     (INRIA, Tropics team)
    Tapenade 3.10 (r5498) - 20 Jan 2015 09:48
*/
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "ba.h"
/*  Hint: NBDirsMax should be the maximum number of differentiation directions
*/

/*
  Differentiation of rodrigues_rot in forward (tangent) mode:
   variations   of useful results: *R
   with respect to varying inputs: *rot *R
   Plus diff mem management of: rot:in R:in
*/
// rot 3 rotation parameters
// R 3*3 rotation matrix (column major)
// easy to understand calculation in matlab:
//	theta = sqrt(sum(w. ^ 2));
//	n = w / theta;
//	n_x = au_cross_matrix(n);
//	R = eye(3) + n_x*sin(theta) + n_x*n_x*(1 - cos(theta));
void rodrigues_rot_dv(double *rot, double (*rotd)[NBDirsMax], double *R, 
        double (*Rd)[NBDirsMax], int nbdirs) {
    double w1, w2, w3, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14
    , t15, t17, t23, t32;
    double w1d[NBDirsMax], w2d[NBDirsMax], w3d[NBDirsMax], t2d[NBDirsMax], t3d
    [NBDirsMax], t4d[NBDirsMax], t5d[NBDirsMax], t6d[NBDirsMax], t7d[NBDirsMax
    ], t8d[NBDirsMax], t9d[NBDirsMax], t10d[NBDirsMax], t11d[NBDirsMax], t12d[
    NBDirsMax], t13d[NBDirsMax], t14d[NBDirsMax], t15d[NBDirsMax], t17d[
    NBDirsMax], t23d[NBDirsMax], t32d[NBDirsMax];
    int nd;
    w1 = rot[0];
    w2 = rot[1];
    w3 = rot[2];
    t2 = w2*w2;
    t3 = w1*w1;
    t4 = w3*w3;
    t5 = t2 + t3 + t4 + (double)2.22044604925031308085e-16L;
    t7 = sqrt(t5);
    t8 = cos(t7);
    t10 = sin(t7);
    t9 = t8 - 1.0;
    t11 = 1./t7;
    t6 = 1./t5;
    t12 = t4*t6;
    t15 = t2*t6;
    t17 = t12 + t15;
    for (nd = 0; nd < nbdirs; ++nd) {
        w1d[nd] = rotd[0][nd];
        w2d[nd] = rotd[1][nd];
        w3d[nd] = rotd[2][nd];
        t2d[nd] = w2d[nd]*w2 + w2*w2d[nd];
        t3d[nd] = w1d[nd]*w1 + w1*w1d[nd];
        t4d[nd] = w3d[nd]*w3 + w3*w3d[nd];
        t5d[nd] = t2d[nd] + t3d[nd] + t4d[nd];
        t7d[nd] = (t5 == 0.0 ? 0.0 : t5d[nd]/(2.0*sqrt(t5)));
        t8d[nd] = -(t7d[nd]*sin(t7));
        t10d[nd] = t7d[nd]*cos(t7);
        t9d[nd] = t8d[nd];
        t11d[nd] = -(t7d[nd]/(t7*t7));
        t13d[nd] = (t10d[nd]*t11+t10*t11d[nd])*w2 + t10*t11*w2d[nd];
        t6d[nd] = -(t5d[nd]/(t5*t5));
        t12d[nd] = t4d[nd]*t6 + t4*t6d[nd];
        t14d[nd] = t3d[nd]*t6 + t3*t6d[nd];
        t15d[nd] = t2d[nd]*t6 + t2*t6d[nd];
        t17d[nd] = t12d[nd] + t15d[nd];
        t23d[nd] = t12d[nd] + t14d[nd];
        t32d[nd] = t14d[nd] + t15d[nd];
        // first row
        Rd[0*3 + 0][nd] = t9d[nd]*t17 + t9*t17d[nd];
    }
    t13 = t10*t11*w2;
    t14 = t3*t6;
    t23 = t12 + t14;
    t32 = t14 + t15;
    R[0*3 + 0] = t9*t17 + 1.;
    for (nd = 0; nd < nbdirs; ++nd)
        Rd[1*3 + 0][nd] = -(t10d[nd]*t11*w3) - t10*(t11d[nd]*w3+t11*w3d[nd]) -
            (t6d[nd]*t9+t6*t9d[nd])*w1*w2 - t6*t9*(w1d[nd]*w2+w1*w2d[nd]);
    R[1*3 + 0] = -t10*t11*w3 - t6*t9*w1*w2;
    for (nd = 0; nd < nbdirs; ++nd)
        Rd[2*3 + 0][nd] = t13d[nd] - (t6d[nd]*t9+t6*t9d[nd])*w1*w3 - t6*t9*(
            w1d[nd]*w3+w1*w3d[nd]);
    R[2*3 + 0] = t13 - t6*t9*w1*w3;
    for (nd = 0; nd < nbdirs; ++nd)
        // second row
        Rd[0*3 + 1][nd] = (t10d[nd]*t11+t10*t11d[nd])*w3 + t10*t11*w3d[nd] - (
            t6d[nd]*t9+t6*t9d[nd])*w1*w2 - t6*t9*(w1d[nd]*w2+w1*w2d[nd]);
    R[0*3 + 1] = t10*t11*w3 - t6*t9*w1*w2;
    for (nd = 0; nd < nbdirs; ++nd)
        Rd[1*3 + 1][nd] = t9d[nd]*t23 + t9*t23d[nd];
    R[1*3 + 1] = t9*t23 + 1.0;
    for (nd = 0; nd < nbdirs; ++nd)
        Rd[2*3 + 1][nd] = -(t10d[nd]*t11*w1) - t10*(t11d[nd]*w1+t11*w1d[nd]) -
            (t6d[nd]*t9+t6*t9d[nd])*w2*w3 - t6*t9*(w2d[nd]*w3+w2*w3d[nd]);
    R[2*3 + 1] = -t10*t11*w1 - t6*t9*w2*w3;
    for (nd = 0; nd < nbdirs; ++nd)
        // third row
        Rd[0*3 + 2][nd] = -t13d[nd] - (t6d[nd]*t9+t6*t9d[nd])*w1*w3 - t6*t9*(
            w1d[nd]*w3+w1*w3d[nd]);
    R[0*3 + 2] = -t13 - t6*t9*w1*w3;
    for (nd = 0; nd < nbdirs; ++nd)
        Rd[1*3 + 2][nd] = (t10d[nd]*t11+t10*t11d[nd])*w1 + t10*t11*w1d[nd] - (
            t6d[nd]*t9+t6*t9d[nd])*w2*w3 - t6*t9*(w2d[nd]*w3+w2*w3d[nd]);
    R[1*3 + 2] = t10*t11*w1 - t6*t9*w2*w3;
    for (nd = 0; nd < nbdirs; ++nd)
        Rd[2*3 + 2][nd] = t9d[nd]*t32 + t9*t32d[nd];
    R[2*3 + 2] = t9*t32 + 1.0;
}
/*  Hint: NBDirsMax should be the maximum number of differentiation directions
*/


/*
  Differentiation of radial_distort in forward (tangent) mode:
   variations   of useful results: *proj
   with respect to varying inputs: *rad_params *proj
   Plus diff mem management of: rad_params:in proj:in
*/
// rad_params 2 radial distortion parameters
// proj 2 projection to be distorted
void radial_distort_dv(double *rad_params, double (*rad_paramsd)[NBDirsMax], 
	double *proj, double(*projd)[NBDirsMax], int nbdirs) {
	double L;
    double rsq;
    double rsqd[NBDirsMax];
    rsq = proj[0]*proj[0] + proj[1]*proj[1];
    double Ld[NBDirsMax];
    int nd;
    L = 1 + rad_params[0]*rsq + rad_params[1]*rsq*rsq;
    for (nd = 0; nd < nbdirs; ++nd) {
        rsqd[nd] = projd[0][nd]*proj[0] + proj[0]*projd[0][nd] + projd[1][nd]*
            proj[1] + proj[1]*projd[1][nd];
        Ld[nd] = rad_paramsd[0][nd]*rsq + rad_params[0]*rsqd[nd] + rad_paramsd
            [1][nd]*(rsq*rsq) + rad_params[1]*(rsqd[nd]*rsq+rsq*rsqd[nd]);
        projd[0][nd] = projd[0][nd]*L + proj[0]*Ld[nd];
    }
    proj[0] = proj[0]*L;
    for (nd = 0; nd < nbdirs; ++nd)
        projd[1][nd] = projd[1][nd]*L + proj[1]*Ld[nd];
    proj[1] = proj[1]*L;
}
/*  Hint: NBDirsMax should be the maximum number of differentiation directions
*/


/*
  Differentiation of project in forward (tangent) mode:
   variations   of useful results: *proj
   with respect to varying inputs: *cam *R *X *proj
   Plus diff mem management of: cam:in R:in X:in proj:in-out
*/
// cam 11 cameras in format [r1 r2 r3 C1 C2 C3 f u0 v0 k1 k2]
//            r1, r2, r3 are angle - axis rotation parameters(Rodrigues)
//			  [C1 C2 C3]' is the camera center
//            f is the focal length in pixels
//			  [u0 v0]' is the principal point
//            k1, k2 are radial distortion parameters
// R 3*3 column major rotation matrix
// X 3 point
// proj 2 projection
// projection: 
// Xcam = R * (X - C)
// distorted = radial_distort(projective2euclidean(Xcam), radial_parameters)
// proj = distorted * f + principal_point
// err = sqsum(proj - measurement)
void project_dv(double *cam, double (*camd)[NBDirsMax], double *R, double (*Rd
        )[NBDirsMax], double *X, double (*Xd)[NBDirsMax], double *proj, double
        (*projd)[NBDirsMax], int nbdirs) {
    int i, k, Ridx;
    double *C;
    double (*Cd)[NBDirsMax];
    double Xo[3], Xcam[3];
    double Xod[3][NBDirsMax], Xcamd[3][NBDirsMax];
    int nd;
    int ii1;
    Cd = &camd[3];
    C = &cam[3];
    for (nd = 0; nd < nbdirs; ++nd) {
        for (ii1 = 0; ii1 < 3; ++ii1)
            Xod[ii1][nd] = 0.0;
        Xod[0][nd] = Xd[0][nd] - Cd[0][nd];
    }
    Xo[0] = X[0] - C[0];
    for (nd = 0; nd < nbdirs; ++nd)
        Xod[1][nd] = Xd[1][nd] - Cd[1][nd];
    Xo[1] = X[1] - C[1];
    for (nd = 0; nd < nbdirs; ++nd) {
        Xod[2][nd] = Xd[2][nd] - Cd[2][nd];
        Xcamd[0][nd] = 0.0;
        Xcamd[1][nd] = 0.0;
        Xcamd[2][nd] = 0.0;
    }
    Xo[2] = X[2] - C[2];
    Xcam[0] = 0.;
    Xcam[1] = 0.;
    Xcam[2] = 0.;
    Ridx = 0;
    for (nd = 0; nd < nbdirs; ++nd)
        for (ii1 = 0; ii1 < 3; ++ii1)
            Xcamd[ii1][nd] = 0.0;
    for (i = 0; i < 3; ++i)
        for (k = 0; k < 3; ++k) {
            for (nd = 0; nd < nbdirs; ++nd)
                Xcamd[k][nd] = Xcamd[k][nd] + Rd[Ridx][nd]*Xo[i] + R[Ridx]*Xod
                    [i][nd];
            Xcam[k] = Xcam[k] + R[Ridx]*Xo[i];
            Ridx = Ridx + 1;
        }
    for (nd = 0; nd < nbdirs; ++nd)
        projd[0][nd] = (Xcamd[0][nd]*Xcam[2]-Xcam[0]*Xcamd[2][nd])/(Xcam[2]*
            Xcam[2]);
    proj[0] = Xcam[0]/Xcam[2];
    for (nd = 0; nd < nbdirs; ++nd)
        projd[1][nd] = (Xcamd[1][nd]*Xcam[2]-Xcam[1]*Xcamd[2][nd])/(Xcam[2]*
            Xcam[2]);
    proj[1] = Xcam[1]/Xcam[2];
    radial_distort_dv(&cam[9], &camd[9], proj, projd, nbdirs);
    for (nd = 0; nd < nbdirs; ++nd)
        projd[0][nd] = projd[0][nd]*cam[6] + proj[0]*camd[6][nd] + camd[7][nd]
        ;
    proj[0] = proj[0]*cam[6] + cam[7];
    for (nd = 0; nd < nbdirs; ++nd)
        projd[1][nd] = projd[1][nd]*cam[6] + proj[1]*camd[6][nd] + camd[8][nd]
        ;
    proj[1] = proj[1]*cam[6] + cam[8];
}
/*  Hint: NBDirsMax should be the maximum number of differentiation directions
*/


/*
  Differentiation of ba in forward (tangent) mode:
   variations   of useful results: *err
   with respect to varying inputs: *cams *X
   RW status of diff variables: *err:out *cams:in *X:in
   Plus diff mem management of: err:in cams:in X:in
*/
// n number of cameras
// m number of points
// p number of observations
// cams 11*n cameras in format [r1 r2 r3 C1 C2 C3 f u0 v0 k1 k2]
//            r1, r2, r3 are angle - axis rotation parameters(Rodrigues)
//			  [C1 C2 C3]' is the camera center
//            f is the focal length in pixels
//			  [u0 v0]' is the principal point
//            k1, k2 are radial distortion parameters
// X 3*m points
// obs 2*p observations (pairs cameraIdx, pointIdx)
// feats 2*p features (x,y coordinates corresponding to observations)
// err p squared errors of observations
// projection: 
// Xcam = R * (X - C)
// distorted = radial_distort(projective2euclidean(Xcam), radial_parameters)
// proj = distorted * f + principal_point
// err = sqsum(proj - measurement)
void ba_dv(int n, int m, int p, double *cams, double (*camsd)[NBDirsMax], 
        double *X, double (*Xd)[NBDirsMax], int *obs, double *feats, double *
        err, double (*errd)[NBDirsMax], int nbdirs) {
    int i;
    int nCamParams;
    int nd;
    nCamParams = 11;
    double *R = (double *)malloc(3*3*n*sizeof(double));
	double(*Rd)[NBDirsMax] = (double(*)[NBDirsMax])malloc(3 * 3 * n*sizeof(double)*NBDirsMax);
    double *proj = (double *)malloc(2*sizeof(double));
	double(*projd)[NBDirsMax] = (double(*)[NBDirsMax])malloc(2 * sizeof(double)*NBDirsMax);
    for (i = 0; i < n; ++i)
        rodrigues_rot_dv(&cams[i*nCamParams], &camsd[i*nCamParams], &R[i*3*3]
                         , &Rd[i*3*3], nbdirs);
    for (i = 0; i < p; ++i) {
        int camIdx;
        camIdx = obs[i*2 + 0];
        int ptIdx;
        ptIdx = obs[i*2 + 1];
        project_dv(&cams[camIdx*nCamParams], &camsd[camIdx*nCamParams], &R[
                   camIdx*3*3], &Rd[camIdx*3*3], &X[ptIdx*3], &Xd[ptIdx*3], 
                   proj, projd, nbdirs);
        proj[0] = proj[0] - feats[i*2 + 0];
        proj[1] = proj[1] - feats[i*2 + 1];
        for (nd = 0; nd < nbdirs; ++nd)
            errd[i][nd] = projd[0][nd]*proj[0] + proj[0]*projd[0][nd] + projd[
                1][nd]*proj[1] + proj[1]*projd[1][nd];
        err[i] = proj[0]*proj[0] + proj[1]*proj[1];
    }
    free(Rd);
    free(R);
    free(projd);
    free(proj);
    err[0] = err[0] + (cams[0] - cams[0] + (X[0] - X[0]));
}

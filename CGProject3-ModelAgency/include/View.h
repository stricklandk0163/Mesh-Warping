#ifndef VIEW_H_
#define VIEW_H_

#include "CGCommon.h"

#include <string>

// Data describing camera position and orientation w/ and optional silhouette image
class CameraPose
{
public:
    CameraPose();
    CameraPose(const CameraPose &pCP);
    virtual ~CameraPose();

    // Accessor functions
    void GetPosition(float &pX, float &pY, float &pZ) const;
    const float* GetOrientationMatrix() const;

    void Nudge(const float &pDX, const float &pDY, const float &pDZ);

    // Build a pose from a 'quaternion' coordinate description
    static CameraPose* MakeFromQuaternion(const float &pX, const float &pY, const float &pZ,
                                          const float pQuatI = 0.0, const float pQuatJ = 0.0,
                                          const float pQuatK = 0.0, const float pQuatQ = 1.0);

    // Build a pose from a 'LookAt' coordinate description (a la gluLookat())
    static CameraPose* MakeFromLookAt(const float &pX, const float &pY, const float &pZ,
                                      const float pAtX = 0.0, const float pAtY = 0.0, const float pAtZ = 0.0,
                                      const float pUpX = 0.0, const float pUpY = 1.0, const float pUpZ = 0.0);


protected:
    // Data describing the optics of this view
    // (position, projection and distortion)
    float mX, mY, mZ;			// World position
    float mM[9];				// Orientation Matrix

    // Built in quaternion conversion functions
    static void QuatToMatrix(float *pM, const double &pi, const double &pj, const double &pk, const double &pQr);
    static void MatrixToQuat(const float *pM, double &pi, double &pj, double &pk, double &pQr);
};

// Data for distortion introduced by a typical camera lens
class CameraDistortion
{
public:
    CameraDistortion();
    CameraDistortion(const CameraDistortion &pCD);
    CameraDistortion(const float pFocalLengthMM, const float pSensorWidthMM = 32, const float pImageAspect = 1.0,
                     const float pImageCenterX = 0.0, const float pImageCenterY = 0.0,
                     const float mRadial1 = 0.0, const float mRadial2 = 0.0, const float mRadial3 = 0.0,
                     const float mTangent1 = 0.0, const float mTangent2 = 0.0);

    virtual ~CameraDistortion();

    // Accessor funcitons
    void GetSensorData(float &pCenterX, float &pCenterY, float &pWidth, float &pAspect) const;
    void GetLensData(float &pFocalLengthMM, float &pFOVDeg) const;
    void GetDistortionData(float &pRadial1, float &pRadial2, float &pRadial3, float &pTangent1, float &pTangent2) const;
    void GetFrustumData(float &pAspect, float &pFOVDeg) const;

    // Convert from focal length in mm to field-of-view in degrees (and back)
    static float FLtoFOV(float pFLmm, float pSensorWmm)
    {
        return (360.0f*(float)::atan2(pSensorWmm, 2.0f*pFLmm)/(float)M_PI);
    }

    static float FOVtoFL(float pFOV, float pSensorWmm)
    {
        return (pSensorWmm/(2.0f*(float)::tan(M_PI*pFOV/360.0f)));
    }

protected:
    // Optical center of the lens (in pixels)
    float mImageCenterX, mImageCenterY;

    // Distortion model parameters
    float mRadialDistortion[3], mTangentialDistortion[2];

    // Lens focal length and image sensor/film plane width (in mm)
    float mFocalLengthMM, mSensorWidthMM;

    // Field of view in degrees and image aspect ratio
    float mFOVDeg, mImageAspect;
};

#endif /* VIEW_H_ */

#include "..\include\itm.h"

/*=============================================================================
 |
 |  Description:  Compute the knife-edge diffraction loss
 |
 |        Input:  d__meter          - Distance of interest, in meters
 |                f__mhz            - Frequency, in MHz
 |                a_e__meter        - Effective earth radius, in meters
 |                theta_los         - Angular distance of line-of-sight region
 |                d_hzn__meter[2]   - Horizon distances, in meters
 |
 |      Outputs:  [None]
 |
 |      Returns:  A_k__db        - Knife-edge diffraction loss, in dB
 |
 *===========================================================================*/
double KnifeEdgeDiffraction(const double d__meter, const double f__mhz, const double a_e__meter, const double theta_los, const double d_hzn__meter[2])
{
    const double d_ML__meter = d_hzn__meter[0] + d_hzn__meter[1];        // Maximum line-of-sight distance for actual path
    const double theta_nlos = d__meter / a_e__meter - theta_los;         // Angular distance of diffraction region [Algorithm, Eqn 4.12]

    const double d_nlos__meter = d__meter - d_ML__meter;                 // Diffraction distance, in meters

    // 1 / (4 pi) = 0.0795775
    // [TN101, Eqn I.7]
    const double v_1 = 0.0795775 * (f__mhz / 47.7) * pow(theta_nlos, 2) * d_hzn__meter[0] * d_nlos__meter / (d_nlos__meter + d_hzn__meter[0]);
    const double v_2 = 0.0795775 * (f__mhz / 47.7) * pow(theta_nlos, 2) * d_hzn__meter[1] * d_nlos__meter / (d_nlos__meter + d_hzn__meter[1]);

    const double A_k__db = FresnelIntegral(v_1) + FresnelIntegral(v_2);  // [TN101, Eqn I.1]

    return A_k__db;
}
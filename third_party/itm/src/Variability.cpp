#include "..\include\itm.h"
#include "..\include\Enums.h"
#include "..\include\Warnings.h"

/*=============================================================================
 |
 |  Description:  Curve helper function for TN101v2 Eqn III.69 & III.70
 |
 |        Input:  c1, c2, x1, x2, x3    - Curve fit parameters
 |                d_e__metre            - Effective distance, in meters
 |
 |      Outputs:  [None]
 |
 |      Returns:  Curve value           - in dB
 |
 *===========================================================================*/
double Curve(const double c1, const double c2, const double x1, const double x2, const double x3, const double d_e__meter)
{
    return (c1 + c2 / (1.0 + pow((d_e__meter - x2) / x3, 2))) * (pow(d_e__meter / x1, 2)) / (1.0 + (pow(d_e__meter / x1, 2)));
}

/*=============================================================================
 |
 |  Description:  Compute the variability loss
 |
 |        Input:  time           - Time percentage, 0 < time < 1
 |                location       - Location percentage, 0 < location < 1
 |                situation      - Situation percentage, 0 < situation < 1
 |                h_e__meter[2]  - Effective antenna heights, in meters
 |                delta_h__meter - Terrain irregularity parameter
 |                f__mhz         - Frequency, in MHz
 |                d__meter       - Path distance, in meters
 |                A_ref__db      - Reference attenuation, in dB
 |                climate        - Radio climate enum
 |                                      + 1 : CLIMATE__EQUATORIAL
 |                                      + 2 : CLIMATE__CONTINENTAL_SUBTROPICAL
 |                                      + 3 : CLIMATE__MARITIME_SUBTROPICAL
 |                                      + 4 : CLIMATE__DESERT
 |                                      + 5 : CLIMATE__CONTINENTAL_TEMPERATE
 |                                      + 6 : CLIMATE__MARITIME_TEMPERATE_OVER_LAND
 |                                      + 7 : CLIMATE__MARITIME_TEMPERATE_OVER_SEA
 |                mdvar          - Mode of variability
 |
 |      Outputs:  warnings       - Warning flags
 |
 |      Returns:  F()            - in dB
 |
 *===========================================================================*/
double Variability(const double time, const double location, const double situation, const double h_e__meter[2], const double delta_h__meter, 
    const double f__mhz, const double d__meter, const double A_ref__db, const int climate, const int mdvar, long *warnings)
{
    // Asymptotic values from TN101, Fig 10.13
    // -> approximate to TN101v2 Eqn III.69 & III.70
    // -> to describe the curves for each climate
    constexpr double all_year[5][7] =
    {
        {  -9.67,   -0.62,    1.26,   -9.21,   -0.62,   -0.39,      3.15 },
        {  12.7,     9.19,   15.5,     9.05,    9.19,    2.86,   857.9   },
        { 144.9e3, 228.9e3, 262.6e3,  84.1e3, 228.9e3, 141.7e3, 2222.e3  },
        { 190.3e3, 205.2e3, 185.2e3, 101.1e3, 205.2e3, 315.9e3,  164.8e3 },
        { 133.8e3, 143.6e3,  99.8e3,  98.6e3, 143.6e3, 167.4e3,  116.3e3 }
    };

    constexpr double bsm1[] = { 2.13,      2.66,    6.11,     1.98,   2.68,    6.86,    8.51 };
    constexpr double bsm2[] = { 159.5,     7.67,    6.65,    13.11,   7.16,   10.38,  169.8 };
    constexpr double xsm1[] = { 762.2e3, 100.4e3, 138.2e3, 139.1e3,  93.7e3, 187.8e3, 609.8e3 };
    constexpr double xsm2[] = { 123.6e3, 172.5e3, 242.2e3, 132.7e3, 186.8e3, 169.6e3, 119.9e3 };
    constexpr double xsm3[] = { 94.5e3,  136.4e3, 178.6e3, 193.5e3, 133.5e3, 108.9e3, 106.6e3 };

    constexpr double bsp1[] = { 2.11, 6.87, 10.08, 3.68, 4.75, 8.58, 8.43 };
    constexpr double bsp2[] = { 102.3, 15.53, 9.60, 159.3, 8.12, 13.97, 8.19 };
    constexpr double xsp1[] = { 636.9e3, 138.7e3, 165.3e3, 464.4e3, 93.2e3, 216.0e3, 136.2e3 };
    constexpr double xsp2[] = { 134.8e3, 143.7e3, 225.7e3, 93.1e3, 135.9e3, 152.0e3, 188.5e3 };
    constexpr double xsp3[] = { 95.6e3, 98.6e3, 129.7e3, 94.2e3, 113.4e3, 122.7e3, 122.9e3 };

    constexpr double C_D[] = { 1.224, 0.801, 1.380, 1.000, 1.224, 1.518, 1.518 };	// [Algorithm, Table 5.1], C_d
    constexpr double z_D[] = { 1.282, 2.161, 1.282, 20.0, 1.282, 1.282, 1.282 };	// [Algorithm, Table 5.1], z_d

    constexpr double bfm1[] = { 1.0, 1.0, 1.0, 1.0, 0.92, 1.0, 1.0 };
    constexpr double bfm2[] = { 0.0, 0.0, 0.0, 0.0, 0.25, 0.0, 0.0 };
    constexpr double bfm3[] = { 0.0, 0.0, 0.0, 0.0, 1.77, 0.0, 0.0 };

    constexpr double bfp1[] = { 1.0, 0.93, 1.0, 0.93, 0.93, 1.0, 1.0 };
    constexpr double bfp2[] = { 0.0, 0.31, 0.0, 0.19, 0.31, 0.0, 0.0 };
    constexpr double bfp3[] = { 0.0, 2.00, 0.0, 1.79, 2.00, 0.0, 0.0 };

    double z_T = InverseComplementaryCumulativeDistributionFunction(time / 100);
    double z_L = InverseComplementaryCumulativeDistributionFunction(location / 100);
    const double z_S = InverseComplementaryCumulativeDistributionFunction(situation / 100);

    int climate_idx = climate; // Create an internal copy for modification
    climate_idx--; // 0-based indexes

    const double wn = f__mhz / 47.7;

    // compute the effective distance
    const double d_ex__meter = sqrt(2 * a_9000__meter * h_e__meter[0]) + sqrt(2 * a_9000__meter * h_e__meter[1]) + pow((575.7e12 / wn), THIRD);  // [Algorithm, Eqn 5.3]

    double d_e__meter;
    if (d__meter < d_ex__meter)
        d_e__meter = 130e3 * d__meter / d_ex__meter;
    else
        d_e__meter = 130e3 + d__meter - d_ex__meter;

    //////////////////////////////////
    // situation variability calcs

    // if mdvar >= 20, then "Direct situation variability is to be eliminated as it should when
    //                       considering interference problems.  Note that there may still be a 
    //                       small residual situation variability" [Hufford, 1982]
    int mdvar_internal = mdvar;  // Create an internal copy to modify
    const bool plus20 = mdvar_internal >= 20;
    if (plus20)
        mdvar_internal -= 20;

    double sigma_S;
    if (plus20)
        sigma_S = 0.0;
    else
    {
        double D__meter = 100e3;                                // Scale distance, D = 100 km
        sigma_S = 5.0 + 3.0 * exp(-d_e__meter / D__meter);      // [Algorithm, Eqn 5.10]
    }

    //
    //////////////////////////////////

    
    const bool plus10 = mdvar_internal >= 10;
    if (plus10)
        mdvar_internal -= 10;

    const double V_med__db = Curve(all_year[0][climate_idx], all_year[1][climate_idx], all_year[2][climate_idx], all_year[3][climate_idx], all_year[4][climate_idx], d_e__meter);

    if (mdvar_internal == SINGLE_MESSAGE_MODE)
    {
        z_T = z_S;
        z_L = z_S;
    }
    else if (mdvar_internal == ACCIDENTAL_MODE)
        z_L = z_S;
    else if (mdvar_internal == MOBILE_MODE)
        z_L = z_T;
    // else using Broadcast Mode (no additional operations)

    if (fabs(z_T) > 3.10 || fabs(z_L) > 3.10 || fabs(z_S) > 3.10)
        *warnings |= WARN__EXTREME_VARIABILITIES;

    //////////////////////////////////
    // location variability calcs

    double sigma_L;
    if (plus10)
        sigma_L = 0.0;
    else
    {
        const double delta_h_d__meter = TerrainRoughness(d__meter, delta_h__meter);

        sigma_L = 10.0 * wn * delta_h_d__meter / (wn * delta_h_d__meter + 13.0);    // Context of [Algorithm, Eqn 5.9]
    }
    const double Y_L = sigma_L * z_L;

    //
    //////////////////////////////////

    //////////////////////////////////
    // time variability calcs

    const double q = log(0.133 * wn);
    const double g_minus = bfm1[climate_idx] + bfm2[climate_idx] / (pow(bfm3[climate_idx] * q, 2) + 1.0);
    const double g_plus = bfp1[climate_idx] + bfp2[climate_idx] / (pow(bfp3[climate_idx] * q, 2) + 1.0);

    const double sigma_T_minus = Curve(bsm1[climate_idx], bsm2[climate_idx], xsm1[climate_idx], xsm2[climate_idx], xsm3[climate_idx], d_e__meter) * g_minus;
    const double sigma_T_plus = Curve(bsp1[climate_idx], bsp2[climate_idx], xsp1[climate_idx], xsp2[climate_idx], xsp3[climate_idx], d_e__meter) * g_plus;

    const double sigma_TD = C_D[climate_idx] * sigma_T_plus;
    const double tgtd = (sigma_T_plus - sigma_TD) * z_D[climate_idx];

    double sigma_T;
    if (z_T < 0.0)
        sigma_T = sigma_T_minus;
    else if (z_T <= z_D[climate_idx])
        sigma_T = sigma_T_plus;
    else
        sigma_T = sigma_TD + tgtd / z_T;
    const double Y_T = sigma_T * z_T;

    //
    /////////////////////////////////

    const double Y_S_temp = pow(sigma_S, 2) + pow(Y_T, 2) / (7.8 + pow(z_S, 2)) + pow(Y_L, 2) / (24.0 + pow(z_S, 2));  // Part of [Algorithm, Eqn 5.11]

    double Y_R, Y_S;
    if (mdvar_internal == SINGLE_MESSAGE_MODE)
    {
        Y_R = 0.0;
        Y_S = sqrt(pow(sigma_T, 2) + pow(sigma_L, 2) + Y_S_temp) * z_S;
    }
    else if (mdvar_internal == ACCIDENTAL_MODE)
    {
        Y_R = Y_T;
        Y_S = sqrt(pow(sigma_L, 2) + Y_S_temp) * z_S;
    }
    else if (mdvar_internal == MOBILE_MODE)
    {
        Y_R = sqrt(pow(sigma_T, 2) + pow(sigma_L, 2)) * z_T;
        Y_S = sqrt(Y_S_temp) * z_S;
    }
    else // BROADCAST_MODE
    {
        Y_R = Y_T + Y_L;
        Y_S = sqrt(Y_S_temp) * z_S;
    }

    double result = A_ref__db - V_med__db - Y_R - Y_S;

    // [Algorithm, Eqn 52]
    if (result < 0.0)
        result = result * (29.0 - result) / (29.0 - 10.0 * result);

    return result;
}
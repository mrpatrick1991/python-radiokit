#include "itm.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// Helper function to convert warnings (bitmask) into a Python list of strings
std::vector<std::string> parse_warnings(long warnings) {
  std::vector<std::string> result;
  if (warnings & 0x0001)
    result.push_back("TX terminal height is near its limits");
  if (warnings & 0x0002)
    result.push_back("RX terminal height is near its limits");
  if (warnings & 0x0004)
    result.push_back("Frequency is near its limits");
  if (warnings & 0x0008)
    result.push_back("Path distance is near its upper limit");
  if (warnings & 0x0010)
    result.push_back("Path distance is large - care must be taken with result");
  if (warnings & 0x0020)
    result.push_back("Path distance is near its lower limit");
  if (warnings & 0x0040)
    result.push_back("Path distance is small - care must be taken with result");
  if (warnings & 0x0080)
    result.push_back("TX horizon angle is large - small angle approximations "
                     "could break down");
  if (warnings & 0x0100)
    result.push_back("RX horizon angle is large - small angle approximations "
                     "could break down");
  if (warnings & 0x0200)
    result.push_back("TX horizon distance is less than 1/10 of the smooth "
                     "earth horizon distance");
  if (warnings & 0x0400)
    result.push_back("RX horizon distance is less than 1/10 of the smooth "
                     "earth horizon distance");
  if (warnings & 0x0800)
    result.push_back("TX horizon distance is greater than 3 times the smooth "
                     "earth horizon distance");
  if (warnings & 0x1000)
    result.push_back("RX horizon distance is greater than 3 times the smooth "
                     "earth horizon distance");
  if (warnings & 0x2000)
    result.push_back("One of the provided variabilities is located far in the "
                     "tail of its distribution");
  if (warnings & 0x4000)
    result.push_back("Internally computed surface refractivity value is small "
                     "- care must be taken with result");
  return result;
}

// Wrap the IntermediateValues structure
py::dict wrap_intermediate_values(const IntermediateValues &values) {
  py::dict result;
  result["theta_hzn"] =
      std::vector<double>{values.theta_hzn[0], values.theta_hzn[1]};
  result["d_hzn__meter"] =
      std::vector<double>{values.d_hzn__meter[0], values.d_hzn__meter[1]};
  result["h_e__meter"] =
      std::vector<double>{values.h_e__meter[0], values.h_e__meter[1]};
  result["N_s"] = values.N_s;
  result["delta_h__meter"] = values.delta_h__meter;
  result["A_ref__db"] = values.A_ref__db;
  result["A_fs__db"] = values.A_fs__db;
  result["d__km"] = values.d__km;
  result["mode"] = values.mode;
  return result;
}

PYBIND11_MODULE(itm_bindings, m) {
  m.doc() = "Python bindings for ITM propagation model";

  // ITM_P2P_TLS
  m.def("itm_p2p_tls",
        [](double h_tx, double h_rx, const std::vector<double> &pfl,
           int climate, double N_0, double f_mhz, int pol, double epsilon,
           double sigma, int mdvar, double time, double location,
           double situation) {
          double A_db;
          long warnings;
          int status = ITM_P2P_TLS(h_tx, h_rx, pfl.data(), climate, N_0, f_mhz,
                                   pol, epsilon, sigma, mdvar, time, location,
                                   situation, &A_db, &warnings);
          return py::make_tuple(status, A_db, parse_warnings(warnings));
        });

  // ITM_P2P_TLS_Ex
  m.def(
      "itm_p2p_tls_ex",
      [](double h_tx, double h_rx, const std::vector<double> &pfl, int climate,
         double N_0, double f_mhz, int pol, double epsilon, double sigma,
         int mdvar, double time, double location, double situation) {
        double A_db;
        long warnings;
        IntermediateValues interValues;

        int status = ITM_P2P_TLS_Ex(h_tx, h_rx, pfl.data(), climate, N_0, f_mhz,
                                    pol, epsilon, sigma, mdvar, time, location,
                                    situation, &A_db, &warnings, &interValues);

        return py::make_tuple(
            status,                               // Return status code
            A_db,                                 // Computed attenuation
            parse_warnings(warnings),             // List of warnings
            wrap_intermediate_values(interValues) // Wrapped intermediate values
        );
      },
      "Point-to-point transmission loss calculation with extended values",
      py::arg("h_tx"), py::arg("h_rx"), py::arg("pfl"), py::arg("climate"),
      py::arg("N_0"), py::arg("f_mhz"), py::arg("pol"), py::arg("epsilon"),
      py::arg("sigma"), py::arg("mdvar"), py::arg("time"), py::arg("location"),
      py::arg("situation"));

  // ITM_P2P_CR
  m.def("itm_p2p_cr",
        [](double h_tx, double h_rx, const std::vector<double> &pfl,
           int climate, double N_0, double f_mhz, int pol, double epsilon,
           double sigma, int mdvar, double confidence, double reliability) {
          double A_db;
          long warnings;
          int status = ITM_P2P_CR(h_tx, h_rx, pfl.data(), climate, N_0, f_mhz,
                                  pol, epsilon, sigma, mdvar, confidence,
                                  reliability, &A_db, &warnings);
          return py::make_tuple(status, A_db, parse_warnings(warnings));
        });

  // ITM_P2P_CR_Ex
  m.def(
      "itm_p2p_cr_ex",
      [](double h_tx, double h_rx, const std::vector<double> &pfl, int climate,
         double N_0, double f_mhz, int pol, double epsilon, double sigma,
         int mdvar, double confidence, double reliability) {
        double A_db;
        long warnings;
        IntermediateValues interValues;

        int status = ITM_P2P_CR_Ex(h_tx, h_rx, pfl.data(), climate, N_0, f_mhz,
                                   pol, epsilon, sigma, mdvar, confidence,
                                   reliability, &A_db, &warnings, &interValues);

        return py::make_tuple(
            status,                               // Return status code
            A_db,                                 // Computed attenuation
            parse_warnings(warnings),             // List of warnings
            wrap_intermediate_values(interValues) // Wrapped intermediate values
        );
      },
      "Point-to-point transmission loss calculation with confidence, "
      "reliability, and extended values",
      py::arg("h_tx"), py::arg("h_rx"), py::arg("pfl"), py::arg("climate"),
      py::arg("N_0"), py::arg("f_mhz"), py::arg("pol"), py::arg("epsilon"),
      py::arg("sigma"), py::arg("mdvar"), py::arg("confidence"),
      py::arg("reliability"));

  // ITM_AREA_TLS
  m.def("itm_area_tls",
        [](double h_tx, double h_rx, int tx_site_criteria, int rx_site_criteria,
           double d_km, double delta_h_meter, int climate, double N_0,
           double f_mhz, int pol, double epsilon, double sigma, int mdvar,
           double time, double location, double situation) {
          double A_db;
          long warnings;
          int status = ITM_AREA_TLS(
              h_tx, h_rx, tx_site_criteria, rx_site_criteria, d_km,
              delta_h_meter, climate, N_0, f_mhz, pol, epsilon, sigma, mdvar,
              time, location, situation, &A_db, &warnings);
          return py::make_tuple(status, A_db, parse_warnings(warnings));
        });

  // ITM_AREA_TLS_Ex
  m.def(
      "itm_area_tls_ex",
      [](double h_tx, double h_rx, int tx_site_criteria, int rx_site_criteria,
         double d_km, double delta_h_meter, int climate, double N_0,
         double f_mhz, int pol, double epsilon, double sigma, int mdvar,
         double time, double location, double situation) {
        double A_db;
        long warnings;
        IntermediateValues interValues;

        int status = ITM_AREA_TLS_Ex(
            h_tx, h_rx, tx_site_criteria, rx_site_criteria, d_km, delta_h_meter,
            climate, N_0, f_mhz, pol, epsilon, sigma, mdvar, time, location,
            situation, &A_db, &warnings, &interValues);

        return py::make_tuple(
            status,                               // Return status code
            A_db,                                 // Computed attenuation
            parse_warnings(warnings),             // List of warnings
            wrap_intermediate_values(interValues) // Intermediate values wrapped
                                                  // in a Python dictionary
        );
      },
      "Area transmission loss calculation with extended values",
      py::arg("h_tx"), py::arg("h_rx"), py::arg("tx_site_criteria"),
      py::arg("rx_site_criteria"), py::arg("d_km"), py::arg("delta_h_meter"),
      py::arg("climate"), py::arg("N_0"), py::arg("f_mhz"), py::arg("pol"),
      py::arg("epsilon"), py::arg("sigma"), py::arg("mdvar"), py::arg("time"),
      py::arg("location"), py::arg("situation"));

  // ITM_AREA_CR
  m.def(
      "itm_area_cr",
      [](double h_tx, double h_rx, int tx_site_criteria, int rx_site_criteria,
         double d_km, double delta_h_meter, int climate, double N_0,
         double f_mhz, int pol, double epsilon, double sigma, int mdvar,
         double confidence, double reliability) {
        double A_db;
        long warnings;

        int status =
            ITM_AREA_CR(h_tx, h_rx, tx_site_criteria, rx_site_criteria, d_km,
                        delta_h_meter, climate, N_0, f_mhz, pol, epsilon, sigma,
                        mdvar, confidence, reliability, &A_db, &warnings);

        return py::make_tuple(status,                  // Return status code
                              A_db,                    // Computed attenuation
                              parse_warnings(warnings) // List of warnings
        );
      },
      "Area transmission loss calculation with confidence and reliability",
      py::arg("h_tx"), py::arg("h_rx"), py::arg("tx_site_criteria"),
      py::arg("rx_site_criteria"), py::arg("d_km"), py::arg("delta_h_meter"),
      py::arg("climate"), py::arg("N_0"), py::arg("f_mhz"), py::arg("pol"),
      py::arg("epsilon"), py::arg("sigma"), py::arg("mdvar"),
      py::arg("confidence"), py::arg("reliability"));

  // ITM_AREA_CR_Ex
  m.def(
      "itm_area_cr_ex",
      [](double h_tx, double h_rx, int tx_site_criteria, int rx_site_criteria,
         double d_km, double delta_h_meter, int climate, double N_0,
         double f_mhz, int pol, double epsilon, double sigma, int mdvar,
         double confidence, double reliability) {
        double A_db;
        long warnings;
        IntermediateValues interValues;

        int status = ITM_AREA_CR_Ex(
            h_tx, h_rx, tx_site_criteria, rx_site_criteria, d_km, delta_h_meter,
            climate, N_0, f_mhz, pol, epsilon, sigma, mdvar, confidence,
            reliability, &A_db, &warnings, &interValues);

        return py::make_tuple(
            status,                               // Return status code
            A_db,                                 // Computed attenuation
            parse_warnings(warnings),             // List of warnings
            wrap_intermediate_values(interValues) // Wrapped intermediate values
        );
      },
      "Area transmission loss calculation with confidence, reliability, and "
      "extended values",
      py::arg("h_tx"), py::arg("h_rx"), py::arg("tx_site_criteria"),
      py::arg("rx_site_criteria"), py::arg("d_km"), py::arg("delta_h_meter"),
      py::arg("climate"), py::arg("N_0"), py::arg("f_mhz"), py::arg("pol"),
      py::arg("epsilon"), py::arg("sigma"), py::arg("mdvar"),
      py::arg("confidence"), py::arg("reliability"));
}

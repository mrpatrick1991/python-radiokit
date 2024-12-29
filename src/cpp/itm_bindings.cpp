#include "itm_bindings.h"
#include "itm.h" // ITM library header

namespace py = pybind11;

void init_itm_bindings(py::module_ &m) {
    // Expose ITM library functions to Python
    m.def("ITM_P2P_TLS", &ITM_P2P_TLS, "Point-to-point transmission loss model",
          py::arg("h_tx__meter"), py::arg("h_rx__meter"), py::arg("pfl"),
          py::arg("climate"), py::arg("N_0"), py::arg("f__mhz"), py::arg("pol"),
          py::arg("epsilon"), py::arg("sigma"), py::arg("mdvar"),
          py::arg("time"), py::arg("location"), py::arg("situation"));
}

// Pybind11 module definition
PYBIND11_MODULE(_itm_bindings, m) {
    m.doc() = "Python bindings for ITM library";

    // Call the initialization function for ITM bindings
    init_itm_bindings(m);
}

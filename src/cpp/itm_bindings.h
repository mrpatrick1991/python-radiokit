#ifndef ITM_BINDINGS_H
#define ITM_BINDINGS_H

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

// Forward declare any functions you will expose via Pybind11
void init_itm_bindings(pybind11::module_ &m);

#endif // ITM_BINDINGS_H

from setuptools import setup, find_packages, Extension
import pybind11
import glob

itm_sources = glob.glob("third_party/itm/src/*.cpp")

radiokit_bindings_itm = Extension(
    "radiokit.bindings.itm_bindings",
    sources=[
        "src/radiokit/bindings/itm_bindings.cpp",
        *itm_sources,
    ],
    include_dirs=[
        "third_party/itm/include",
        pybind11.get_include(),
    ],
    extra_compile_args=["-fdeclspec", "-fms-extensions", "-std=c++11"],
    language="c++",
)

setup(
    name="radiokit",
    version="0.1.0",
    description="Radio propagation models for Python",
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    ext_modules=[radiokit_bindings_itm],
    include_package_data=True,
)

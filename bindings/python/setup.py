import os
import sys
import subprocess
import shutil
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        
        # Required for auto-detection of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            "-DBUILD_PYTHON_BINDINGS=ON",
            "-DBUILD_TESTS=OFF",
            "-DBUILD_BENCHMARKS=OFF",
        ]

        cfg = "Debug" if self.debug else "Release"
        build_args = ["--config", cfg]

        if sys.platform.startswith("win"):
            cmake_args += [
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}",
            ]
            build_args += ["--", "/m"]
        else:
            cmake_args += [f"-DCMAKE_BUILD_TYPE={cfg}"]
            build_args += ["--", "-j4"]

        env = os.environ.copy()
        env["CXXFLAGS"] = f'{env.get("CXXFLAGS", "")} -DVERSION_INFO=\\"{self.distribution.get_version()}\\"'
        
        # Build directory
        build_temp = Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)
        
        # Navigate to root of project (two levels up from bindings/python)
        root_dir = Path(ext.sourcedir).resolve().parent.parent
        
        subprocess.check_call(
            ["cmake", str(root_dir)] + cmake_args, 
            cwd=self.build_temp, 
            env=env
        )
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, 
            cwd=self.build_temp
        )
        
        # Copy the unigram_tokeniser shared library alongside the Python extension
        # Search for the library in multiple possible locations
        if sys.platform.startswith("win"):
            lib_name = "unigram_tokeniser.dll"
            search_paths = [
                Path(self.build_temp) / "bin" / cfg / lib_name,
                Path(self.build_temp) / "bin" / lib_name,
                Path(self.build_temp) / "lib" / cfg / lib_name,
            ]
        else:
            lib_name = "libunigram_tokeniser.so"
            search_paths = [
                Path(self.build_temp) / "bin" / lib_name,
                Path(self.build_temp) / "lib" / lib_name,
                Path(self.build_temp) / "src" / lib_name,
            ]
        
        # Find and copy the library
        lib_found = False
        for lib_path in search_paths:
            if lib_path.exists():
                shutil.copy2(str(lib_path), extdir)
                lib_found = True
                print(f"Copied {lib_name} from {lib_path} to {extdir}")
                break
        
        if not lib_found:
            print(f"Warning: Could not find {lib_name} in expected locations:")
            for p in search_paths:
                print(f"  - {p}")
        
        # Copy the Python extension module (the actual pybind11 module)
        # This is critical - it's what Python imports!
        import glob
        
        # Search for the Python extension in the bindings/python directory
        python_ext_patterns = [
            Path(self.build_temp) / "bindings" / "python" / "unigram*.so",
            Path(self.build_temp) / "bindings" / "python" / "unigram*.pyd",
        ]
        
        ext_found = False
        for pattern in python_ext_patterns:
            matching_files = glob.glob(str(pattern))
            for ext_path in matching_files:
                ext_path = Path(ext_path)
                shutil.copy2(str(ext_path), extdir)
                ext_found = True
                print(f"Copied Python extension {ext_path.name} from {ext_path} to {extdir}")
                break
            if ext_found:
                break
        
        if not ext_found:
            print(f"ERROR: Could not find Python extension module!")
            print(f"\nContents of build directory {self.build_temp}:")
            for root, dirs, files in os.walk(self.build_temp):
                level = root.replace(str(self.build_temp), '').count(os.sep)
                indent = ' ' * 2 * level
                print(f'{indent}{os.path.basename(root)}/')
                subindent = ' ' * 2 * (level + 1)
                for file in files[:10]:  # Limit output
                    if file.endswith(('.so', '.dll', '.pyd')):
                        print(f'{subindent}{file}')


setup(
    ext_modules=[CMakeExtension("unigram_tokeniser.unigram")],
    cmdclass={"build_ext": CMakeBuild},
    packages=["unigram_tokeniser"],
    zip_safe=False,
)

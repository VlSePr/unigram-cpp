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
        
        # Search for the Python extension module by walking the entire build tree
        # pybind11 might place it in various locations depending on platform/CMake config
        print(f"\n{'='*60}")
        print(f"Searching for Python extension module in {self.build_temp}...")
        print(f"{'='*60}")
        
        ext_found = False
        ext_name_pattern = "unigram"  # Looking for unigram.*.so or unigram.*.pyd
        
        for root, dirs, files in os.walk(self.build_temp):
            for file in files:
                # Match unigram.*.so (Linux/Mac) or unigram.*.pyd (Windows)
                # but NOT libunigram_tokeniser.so (that's the C++ library)
                if (file.startswith(ext_name_pattern) and 
                    (file.endswith('.so') or file.endswith('.pyd')) and
                    not file.startswith('lib')):
                    ext_path = Path(root) / file
                    dest_path = Path(extdir) / file
                    shutil.copy2(str(ext_path), str(dest_path))
                    ext_found = True
                    print(f"✓ Copied Python extension: {file}")
                    print(f"  From: {ext_path}")
                    print(f"  To:   {dest_path}")
                    break
            if ext_found:
                break
        
        if not ext_found:
            print(f"\n{'='*60}")
            print(f"ERROR: Could not find Python extension module!")
            print(f"{'='*60}")
            print(f"Searched in: {self.build_temp}")
            print(f"Looking for: {ext_name_pattern}.*.so or {ext_name_pattern}.*.pyd")
            print(f"\nAll .so/.pyd/.dll files found in build directory:")
            for root, dirs, files in os.walk(self.build_temp):
                for file in files:
                    if file.endswith(('.so', '.dll', '.pyd')):
                        rel_path = Path(root).relative_to(self.build_temp)
                        print(f"  {rel_path / file}")
            print(f"{'='*60}\n")
            raise RuntimeError("Python extension module not found after build!")


setup(
    ext_modules=[CMakeExtension("unigram_tokeniser.unigram")],
    cmdclass={"build_ext": CMakeBuild},
    packages=["unigram_tokeniser"],
    zip_safe=False,
)

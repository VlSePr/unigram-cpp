# Distribution Guide

This document explains how to build and distribute pre-compiled wheels for easy installation.

## Overview

The project uses **GitHub Actions** to automatically build wheels for:
- **Platforms**: Linux (x86_64), Windows (x64), macOS (x86_64, arm64)
- **Python versions**: 3.8, 3.9, 3.10, 3.11, 3.12
- **Distribution**: PyPI and GitHub Releases

## How It Works

### 1. Building Wheels (`build-wheels.yml`)

The workflow uses [cibuildwheel](https://cibuildwheel.readthedocs.io/) to build wheels across platforms:

```yaml
# Triggered on:
- Push to main branch
- Tagged releases (v*)
- Manual dispatch
```

**What it does:**
1. **Builds wheels** for all platform/Python combinations
2. **Runs tests** to verify each wheel imports correctly
3. **Uploads artifacts** for download
4. **Publishes to PyPI** (on tagged releases only)
5. **Creates GitHub Release** with wheels attached (on tagged releases)

### 2. Testing Wheels (`test-wheels.yml`)

After wheels are built, this workflow:
- Downloads each platform's wheels
- Tests installation on fresh environments
- Verifies imports and basic functionality

## Usage Scenarios

### For Users (After Setup)

Once wheels are published, users simply run:

```bash
pip install unigram-tokeniser
```

**No compilers needed!** The wheel contains pre-compiled binaries.

### For Google Colab

```python
# Before (required compiling):
!apt-get install build-essential cmake  # ❌ Slow
!pip install git+https://github.com/VlSePr/unigram-cpp.git#subdirectory=bindings/python

# After (with wheels):
!pip install unigram-tokeniser  # ✅ Fast, just downloads wheel
```

### For Development

```bash
# Install from GitHub Release (specific version)
pip install https://github.com/VlSePr/unigram-cpp/releases/download/v1.0.0/unigram_tokeniser-1.0.0-cp312-cp312-linux_x86_64.whl

# Or from main branch wheels (latest)
# Download artifacts from latest Actions run
```

## Publishing Releases

### Step 1: Create a Release

```bash
# Tag a new version
git tag v1.0.0
git push origin v1.0.0
```

This automatically:
1. ✅ Builds wheels for all platforms
2. ✅ Runs tests
3. ✅ Uploads to PyPI (if configured)
4. ✅ Creates GitHub Release with wheels

### Step 2: PyPI Setup (First Time Only)

**Option A: Trusted Publishing (Recommended)**

1. Go to [PyPI Settings](https://pypi.org/manage/account/publishing/)
2. Add a new publisher:
   - **PyPI Project Name**: `unigram-tokeniser`
   - **Owner**: `VlSePr` (your GitHub username)
   - **Repository**: `unigram-cpp`
   - **Workflow**: `build-wheels.yml`
   - **Environment**: `pypi`

No secrets needed! GitHub and PyPI handle auth automatically.

**Option B: API Token (Legacy)**

1. Create token at https://pypi.org/manage/account/token/
2. Add to GitHub Secrets as `PYPI_API_TOKEN`
3. Update workflow to use `password: ${{ secrets.PYPI_API_TOKEN }}`

### Step 3: Test PyPI (Optional)

For testing before production:

```yaml
# In build-wheels.yml, change:
uses: pypa/gh-action-pypi-publish@release/v1
with:
  repository-url: https://test.pypi.org/legacy/
```

Then install from Test PyPI:
```bash
pip install --index-url https://test.pypi.org/simple/ unigram-tokeniser
```

## Manual Wheel Building

If you need to build wheels locally:

```bash
# Install cibuildwheel
pip install cibuildwheel

# Build wheels for your platform
cd bindings/python
cibuildwheel --platform linux  # or windows, macos

# Wheels appear in ./wheelhouse/
```

## Workflow Architecture

```
┌─────────────────────┐
│   Push tag (v1.0)   │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────────────────────────┐
│  build_wheels (parallel)                │
│  ├─ ubuntu-latest  (Linux x86_64)       │
│  ├─ windows-latest (Windows x64)        │
│  └─ macos-latest   (macOS x86_64+arm64) │
└──────────┬──────────────────────────────┘
           │
           ▼
┌─────────────────────┐
│   build_sdist       │  (source distribution)
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  Upload artifacts   │
│  (wheels + sdist)   │
└──────────┬──────────┘
           │
           ├──────────────────┬─────────────────┐
           ▼                  ▼                 ▼
   ┌─────────────┐   ┌──────────────┐  ┌──────────────┐
   │ PyPI Upload │   │ GitHub       │  │ Test Wheels  │
   │ (on tags)   │   │ Release      │  │ (all platforms)
   └─────────────┘   └──────────────┘  └──────────────┘
```

## Troubleshooting

### Build Fails on Linux

```yaml
# May need to install dependencies
CIBW_BEFORE_ALL_LINUX: |
  yum install -y cmake3 gcc gcc-c++ || \
  apt-get update && apt-get install -y cmake g++
```

### Build Fails on macOS

```yaml
# May need specific architecture
CIBW_ARCHS_MACOS: "x86_64 arm64 universal2"
```

### Import Fails After Install

Check that both files are in the wheel:
```bash
# Extract wheel
unzip unigram_tokeniser-*.whl -d extracted/

# Should contain:
# ✓ unigram_tokeniser/__init__.py
# ✓ unigram_tokeniser/unigram.*.so (or .pyd on Windows)
# ✓ unigram_tokeniser/libunigram_tokeniser.so (or .dll on Windows)
```

## Cost Considerations

GitHub Actions provides:
- **Free tier**: 2,000 minutes/month (public repos get unlimited)
- **Build time**: ~10-15 minutes per platform × 3 platforms = 30-45 minutes per release

For most projects, this is well within free limits.

## Benefits

✅ **Users don't need compilers** - just `pip install`  
✅ **Fast installation** - no build step, just download  
✅ **Works in Colab** - no apt-get install needed  
✅ **Consistent builds** - same environment every time  
✅ **Automated testing** - catches platform-specific issues  
✅ **Easy releases** - just push a tag  

## Next Steps

1. **Push the workflows** to GitHub
2. **Test with manual dispatch** (no tag needed)
3. **Create a test tag**: `git tag v0.1.0-test`
4. **Configure PyPI** trusted publishing
5. **Create official release**: `git tag v1.0.0`

After setup, future releases are just:
```bash
git tag v1.0.1
git push origin v1.0.1
```

Everything else is automatic! 🚀

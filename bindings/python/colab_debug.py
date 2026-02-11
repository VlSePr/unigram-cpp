"""
Diagnostic script to debug Colab installation issues
Run this after: pip install .
"""
import sys
import os
from pathlib import Path

print("=" * 60)
print("UNIGRAM TOKENISER INSTALLATION DEBUG")
print("=" * 60)

# 1. Python version and platform
print(f"\n1. Environment Info:")
print(f"   Python: {sys.version}")
print(f"   Platform: {sys.platform}")
print(f"   Executable: {sys.executable}")

# 2. Check if package is installed
print(f"\n2. Package Installation:")
try:
    import unigram_tokeniser
    print(f"   ✓ unigram_tokeniser package found")
    print(f"   Location: {unigram_tokeniser.__file__}")
    print(f"   Version: {getattr(unigram_tokeniser, '__version__', 'unknown')}")
except ImportError as e:
    print(f"   ✗ Cannot import unigram_tokeniser: {e}")
    sys.exit(1)

# 3. Check package contents
print(f"\n3. Package Contents:")
package_dir = Path(unigram_tokeniser.__file__).parent
print(f"   Package directory: {package_dir}")
if package_dir.exists():
    for item in sorted(package_dir.iterdir()):
        size = item.stat().st_size if item.is_file() else 0
        marker = "📁" if item.is_dir() else "📄"
        print(f"   {marker} {item.name:40s} {size:>10,} bytes" if size else f"   {marker} {item.name}")
else:
    print(f"   ✗ Package directory not found!")

# 4. Try importing the extension module
print(f"\n4. Extension Module Import:")
try:
    from unigram_tokeniser import unigram
    print(f"   ✓ Extension module imported successfully")
    print(f"   Module: {unigram}")
except ImportError as e:
    print(f"   ✗ Cannot import extension module: {e}")
    print(f"\n   This usually means the .so/.pyd file is missing or")
    print(f"   the shared library dependency is not found.")
    
    # Try to find .so or .pyd files
    print(f"\n   Looking for binary files in package:")
    for root, dirs, files in os.walk(package_dir):
        for f in files:
            if f.endswith(('.so', '.pyd', '.dll')):
                full_path = Path(root) / f
                print(f"   Found: {full_path}")
                print(f"          Size: {full_path.stat().st_size:,} bytes")
    sys.exit(1)

# 5. Check available classes and functions
print(f"\n5. Available API:")
members = [x for x in dir(unigram_tokeniser) if not x.startswith('_')]
print(f"   Found {len(members)} public members:")
for member in members:
    obj = getattr(unigram_tokeniser, member)
    print(f"   - {member:30s} {type(obj).__name__}")

# 6. Try basic functionality
print(f"\n6. Functionality Test:")
try:
    vocab = unigram_tokeniser.Vocabulary()
    print(f"   ✓ Created Vocabulary: {type(vocab)}")
    
    config = unigram_tokeniser.TokenizerConfig()
    print(f"   ✓ Created TokenizerConfig")
    print(f"     - vocab_size: {config.vocab_size}")
    
    tokenizer = unigram_tokeniser.Tokenizer(config)
    print(f"   ✓ Created Tokenizer: {type(tokenizer)}")
    
    print(f"\n   Special tokens:")
    print(f"   - UNK: {unigram_tokeniser.UNK_TOKEN} (ID: {unigram_tokeniser.UNK_TOKEN_ID})")
    print(f"   - BOS: {unigram_tokeniser.BOS_TOKEN} (ID: {unigram_tokeniser.BOS_TOKEN_ID})")
    print(f"   - EOS: {unigram_tokeniser.EOS_TOKEN} (ID: {unigram_tokeniser.EOS_TOKEN_ID})")
    print(f"   - PAD: {unigram_tokeniser.PAD_TOKEN} (ID: {unigram_tokeniser.PAD_TOKEN_ID})")
    
except Exception as e:
    print(f"   ✗ Functionality test failed: {e}")
    import traceback
    traceback.print_exc()
    sys.exit(1)

print(f"\n" + "=" * 60)
print("✓ ALL CHECKS PASSED - Installation is working correctly!")
print("=" * 60)

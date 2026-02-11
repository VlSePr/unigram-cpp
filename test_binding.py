import sys
import os

# Add both lib and bin directories to path
sys.path.insert(0, r'build\lib\Release')
os.add_dll_directory(r'C:\Users\Volodymyr_Prudnikov\source\repos\BigLLM\UnigramTokeniser\build\bin\Release')

try:
    import unigram
    print('✓ Python binding imported successfully!')
    print(f'\nModule: {unigram}')
    
    # List available functions and classes
    members = [x for x in dir(unigram) if not x.startswith('_')]
    print(f'\nAvailable members ({len(members)}):')
    for member in members:
        obj = getattr(unigram, member)
        print(f'  - {member}: {type(obj).__name__}')
    
    print('\n✓ Build successful! Python bindings are working.')
    
except ImportError as e:
    print(f'✗ Failed to import unigram: {e}')
    sys.exit(1)

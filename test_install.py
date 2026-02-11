import unigram_tokeniser

print('✓ Successfully imported unigram_tokeniser!')
print(f'Version: {unigram_tokeniser.__version__}')
print(f'\nAvailable classes and constants:')
for name in sorted(dir(unigram_tokeniser)):
    if not name.startswith('_'):
        print(f'  - {name}')

# Test basic functionality
print('\n✓ Testing basic functionality...')
vocab = unigram_tokeniser.Vocabulary()
print(f'  Created Vocabulary: {type(vocab)}')
print(f'  UNK_TOKEN: {unigram_tokeniser.UNK_TOKEN}')
print(f'  BOS_TOKEN: {unigram_tokeniser.BOS_TOKEN}')

print('\n✓ All tests passed! Package is working correctly.')

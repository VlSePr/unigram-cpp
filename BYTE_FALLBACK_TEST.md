# Byte-Level Fallback Test

This script tests the new tokenizer with byte-level fallback support.

## Expected Results

With byte-level fallback, the tokenizer should:
- Never produce `<unk>` tokens
- Handle any input text, even with rare characters
- Degrade gracefully to single-byte tokens when needed

## Test Commands

```bash
# Test on standard text
.\build\bin\Release\debug_tokenizer.exe model\tokenizer_32k_bytebpe.json

# Compare old vs new model
.\build\bin\Release\unigram-cli.exe compare \
  --models model\tokenizer_32000.json,model\tokenizer_32k_bytebpe.json \
  --corpus data\treasure-island \
  --samples "The quick brown fox jumps over the lazy dog|treasure island adventure|rare words: xyzzy quux" \
  --max-lines 1000

# Expected improvements:
# - Old model: ~90% unknown rate
# - New model: <5% unknown rate (ideally 0%)
# - Byte fallback ensures all characters can be encoded
```

## Training Progress

The training is currently running with:
- **263,306 lines** from 14 classic literature files
- **32,000 vocabulary target** size
- **10 EM iterations**
- **Byte-level fallback**: All 256 byte values included as base tokens

This should take approximately 5-7 minutes to complete.

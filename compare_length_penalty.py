#!/usr/bin/env python3
"""
Compare tokenizers trained with and without length penalty.
Shows how length penalty affects token quality.
"""

import json
import sys
from collections import Counter

def load_tokenizer(filepath):
    """Load tokenizer vocabulary from JSON file."""
    with open(filepath, 'r', encoding='utf-8') as f:
        data = json.load(f)
    return data['vocabulary']

def analyze_tokens(vocab, name="Model"):
    """Analyze token characteristics."""
    print(f"\n{'='*60}")
    print(f"{name} Analysis")
    print(f"{'='*60}")
    
    lengths = [len(token['token']) for token in vocab]
    scores = [token['score'] for token in vocab]
    
    print(f"Total tokens: {len(vocab)}")
    print(f"\nToken Length Statistics:")
    print(f"  Average length: {sum(lengths)/len(lengths):.2f} chars")
    print(f"  Min length: {min(lengths)}")
    print(f"  Max length: {max(lengths)}")
    
    # Length distribution
    length_dist = Counter(lengths)
    print(f"\nLength Distribution:")
    for length in sorted(length_dist.keys())[:15]:  # Show first 15
        count = length_dist[length]
        pct = 100 * count / len(vocab)
        bar = '█' * int(pct / 2)
        print(f"  {length:2d} chars: {count:5d} ({pct:5.1f}%) {bar}")
    
    # Score statistics
    print(f"\nScore Statistics:")
    print(f"  Average score: {sum(scores)/len(scores):.4f}")
    print(f"  Min score (worst): {min(scores):.4f}")
    print(f"  Max score (best): {max(scores):.4f}")
    
    # Show tokens at different quality levels
    sorted_by_score = sorted(vocab, key=lambda x: x['score'], reverse=True)
    
    print(f"\nTop 10 Best Tokens (highest score):")
    for token in sorted_by_score[:10]:
        print(f"  '{token['token']}' (len={len(token['token'])}, score={token['score']:.4f})")
    
    print(f"\nBottom 10 Worst Tokens (lowest score):")
    for token in sorted_by_score[-10:]:
        print(f"  '{token['token']}' (len={len(token['token'])}, score={token['score']:.4f})")
    
    # Find problematic tokens (very short or fragmented)
    short_tokens = [t for t in vocab if len(t['token']) <= 2]
    print(f"\nVery Short Tokens (≤2 chars): {len(short_tokens)} ({100*len(short_tokens)/len(vocab):.1f}%)")
    if len(short_tokens) > 0:
        print(f"  Examples: {[t['token'] for t in short_tokens[:20]]}")
    
    return {
        'avg_length': sum(lengths)/len(lengths),
        'avg_score': sum(scores)/len(scores),
        'short_token_pct': 100*len(short_tokens)/len(vocab)
    }

def compare_models(model1_path, model2_path):
    """Compare two models."""
    print(f"\nLoading models...")
    print(f"Model 1: {model1_path}")
    print(f"Model 2: {model2_path}")
    
    vocab1 = load_tokenizer(model1_path)
    vocab2 = load_tokenizer(model2_path)
    
    stats1 = analyze_tokens(vocab1, "Model 1 (No Length Penalty)")
    stats2 = analyze_tokens(vocab2, "Model 2 (With Length Penalty)")
    
    print(f"\n{'='*60}")
    print("COMPARISON SUMMARY")
    print(f"{'='*60}")
    print(f"\nAverage Token Length:")
    print(f"  Model 1: {stats1['avg_length']:.2f} chars")
    print(f"  Model 2: {stats2['avg_length']:.2f} chars")
    print(f"  Δ Change: {stats2['avg_length'] - stats1['avg_length']:+.2f} chars")
    
    print(f"\nShort Tokens (≤2 chars) %:")
    print(f"  Model 1: {stats1['short_token_pct']:.1f}%")
    print(f"  Model 2: {stats2['short_token_pct']:.1f}%")
    print(f"  Δ Change: {stats2['short_token_pct'] - stats1['short_token_pct']:+.1f}%")
    
    if stats2['short_token_pct'] < stats1['short_token_pct']:
        print(f"  ✓ Length penalty reduced short tokens!")
    elif stats2['short_token_pct'] > stats1['short_token_pct']:
        print(f"  ⚠ Length penalty increased short tokens (unexpected)")
    else:
        print(f"  = No change in short token percentage")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python compare_length_penalty.py <model1.json> <model2.json>")
        print("\nExample:")
        print("  python compare_length_penalty.py model/tokenizer_24000.json model/tokenizer_24000_penalty.json")
        sys.exit(1)
    
    compare_models(sys.argv[1], sys.argv[2])

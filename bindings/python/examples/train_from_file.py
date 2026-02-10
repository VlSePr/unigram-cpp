"""
Example: Training tokenizer from a file
"""
import unigram
import sys
import os


def main():
    if len(sys.argv) < 2:
        print("Usage: python train_from_file.py <input_file> [output_model]")
        print("Example: python train_from_file.py corpus.txt model.json")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "trained_model.json"
    
    if not os.path.exists(input_file):
        print(f"Error: Input file not found: {input_file}")
        sys.exit(1)
    
    print("=" * 60)
    print("Training Unigram Tokenizer from File")
    print("=" * 60)
    print(f"Input file: {input_file}")
    print(f"Output model: {output_file}")
    print()
    
    # Configuration
    config = unigram.TokenizerConfig()
    config.vocab_size = 5000
    config.num_iterations = 5
    config.byte_fallback = True
    
    print(f"Vocabulary size: {config.vocab_size}")
    print(f"Training iterations: {config.num_iterations}")
    print()
    
    # Create trainer
    trainer = unigram.Trainer(config)
    
    # Progress callback
    def progress_callback(iteration, message):
        print(f"[Iteration {iteration}] {message}")
    
    trainer.set_progress_callback(progress_callback)
    
    # Train
    print("Training...")
    vocab = trainer.train_from_file(input_file)
    print(f"\nTraining complete!")
    print(f"Final vocabulary size: {vocab.size()}")
    print()
    
    # Create tokenizer
    tokenizer = unigram.Tokenizer(config)
    tokenizer.set_vocabulary(vocab)
    
    # Test on sample text
    print("Testing tokenizer...")
    sample_texts = [
        "Hello, world!",
        "This is a test sentence.",
        "The quick brown fox jumps over the lazy dog.",
    ]
    
    for text in sample_texts:
        tokens = tokenizer.encode(text)
        token_ids = tokenizer.encode_as_ids(text)
        decoded = tokenizer.decode(tokens)
        
        print(f"\nInput: {text}")
        print(f"Tokens ({len(tokens)}): {tokens[:10]}{'...' if len(tokens) > 10 else ''}")
        print(f"IDs ({len(token_ids)}): {token_ids[:10]}{'...' if len(token_ids) > 10 else ''}")
        print(f"Decoded: {decoded}")
    
    # Save model
    print(f"\nSaving model to: {output_file}")
    tokenizer.save(output_file)
    print("Done!")
    print("=" * 60)


if __name__ == "__main__":
    main()

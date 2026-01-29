package com.unigram;

/**
 * Unigram Tokenizer for Java
 * 
 * This class provides JNI bindings to the native C++ Unigram tokenizer.
 */
public class Tokenizer {
    // Load native library
    static {
        System.loadLibrary("unigram_jni");
    }

    private long nativeHandle;

    /**
     * Create a new Tokenizer with default configuration
     */
    public Tokenizer() {
        this.nativeHandle = createNative();
    }

    /**
     * Load tokenizer from file
     * 
     * @param filepath Path to model file
     * @return true if successful
     */
    public native boolean load(String filepath);

    /**
     * Save tokenizer to file
     * 
     * @param filepath Path to model file
     * @return true if successful
     */
    public native boolean save(String filepath);

    /**
     * Encode text into tokens
     * 
     * @param text Input text
     * @return Array of token strings
     */
    public native String[] encode(String text);

    /**
     * Encode text into token IDs
     * 
     * @param text Input text
     * @return Array of token IDs
     */
    public native int[] encodeAsIds(String text);

    /**
     * Decode tokens back to text
     * 
     * @param tokens Array of token strings
     * @return Decoded text
     */
    public native String decode(String[] tokens);

    /**
     * Decode token IDs back to text
     * 
     * @param tokenIds Array of token IDs
     * @return Decoded text
     */
    public native String decodeIds(int[] tokenIds);

    /**
     * Load tokenizer from file (factory method)
     * 
     * @param filepath Path to model file
     * @return Loaded tokenizer
     */
    public static Tokenizer fromFile(String filepath) {
        Tokenizer tokenizer = new Tokenizer();
        if (!tokenizer.load(filepath)) {
            throw new RuntimeException("Failed to load tokenizer from: " + filepath);
        }
        return tokenizer;
    }

    // Native methods
    private native long createNative();

    private native void destroyNative(long handle);

    @Override
    protected void finalize() throws Throwable {
        try {
            if (nativeHandle != 0) {
                destroyNative(nativeHandle);
                nativeHandle = 0;
            }
        } finally {
            super.finalize();
        }
    }
}

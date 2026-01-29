# Unigram Tokeniser Java Bindings

Java bindings for the Unigram Tokeniser C++ library using JNI.

## Building

```bash
# Build the JNI library
cd build
cmake .. -DBUILD_JAVA_BINDINGS=ON
cmake --build . --config Release

# Compile Java classes
javac -d build/java bindings/java/src/main/java/com/unigram/*.java
```

## Usage

```java
import com.unigram.Tokenizer;

public class Example {
    public static void main(String[] args) {
        // Load tokenizer from file
        Tokenizer tokenizer = Tokenizer.fromFile("model.json");
        
        // Encode text
        String[] tokens = tokenizer.encode("Hello, world!");
        int[] tokenIds = tokenizer.encodeAsIds("Hello, world!");
        
        // Decode tokens
        String text = tokenizer.decode(tokens);
        
        System.out.println("Tokens: " + String.join(", ", tokens));
        System.out.println("Decoded: " + text);
    }
}
```

## Requirements

- JDK 11 or higher
- Native library (unigram_jni.dll on Windows, libunigram_jni.so on Linux)

## Note

Make sure the native library is in your Java library path:
```bash
java -Djava.library.path=/path/to/lib Example
```

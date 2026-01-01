#include <fstream>
#include <piper.h>

// Building:
//
// g++ test-1.cpp -I/home/bruce/git/piper1-gpl/libpiper/install/include -L/home/bruce/git/piper1-gpl/libpiper/install -L/home/bruce/git/piper1-gpl/libpiper/install/lib -lpiper -lonnxruntime -o test-1
// export LD_LIBRARY_PATH=/home/bruce/git/piper1-gpl/libpiper/install/lib:/home/bruce/git/piper1-gpl/libpiper/install
//
int main(int,const char**) {

    piper_synthesizer *synth = piper_create("en_US-amy-low.onnx",
                                            "en_US-amy-low.onnx.json",
                                            "libpiper-aarch64/espeak-ng-data");

    // aplay -r 22050 -c 1 -f FLOAT_LE -t raw output.raw
    // aplay -r 16000 -c 1 -f FLOAT_LE -t raw output.raw
    std::ofstream audio_stream("/tmp/output.raw", std::ios::binary);

    piper_synthesize_options options = piper_default_synthesize_options(synth);
    // Change options here:
    // options.length_scale = 2;
    // options.speaker_id = 5;

    piper_synthesize_start(synth, "Hello, parrot connected, CODEC is 16k linear. Ready to record!    Peak -12dB, average -35dB.",
                           &options /* NULL for defaults */);

    piper_audio_chunk chunk;
    while (piper_synthesize_next(synth, &chunk) != PIPER_DONE) {
        audio_stream.write(reinterpret_cast<const char *>(chunk.samples),
                           chunk.num_samples * sizeof(float));
    }

    piper_free(synth);

    return 0;
}

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <unigram/tokenizer.h>
#include <unigram/vocabulary.h>
#include <unigram/trainer.h>

namespace py = pybind11;

PYBIND11_MODULE(unigram, m) {
    m.doc() = "Unigram Tokeniser Python bindings";

    // TokenizerConfig
    py::class_<unigram::TokenizerConfig>(m, "TokenizerConfig")
        .def(py::init<>())
        .def_readwrite("add_bos_token", &unigram::TokenizerConfig::add_bos_token)
        .def_readwrite("add_eos_token", &unigram::TokenizerConfig::add_eos_token)
        .def_readwrite("byte_fallback", &unigram::TokenizerConfig::byte_fallback)
        .def_readwrite("vocab_size", &unigram::TokenizerConfig::vocab_size)
        .def_readwrite("num_iterations", &unigram::TokenizerConfig::num_iterations)
        .def_readwrite("shrinking_factor", &unigram::TokenizerConfig::shrinking_factor)
        .def_readwrite("min_frequency", &unigram::TokenizerConfig::min_frequency);

    // Vocabulary
    py::class_<unigram::Vocabulary>(m, "Vocabulary")
        .def(py::init<>())
        .def("add_token", &unigram::Vocabulary::add_token,
             py::arg("token"), py::arg("score"))
        .def("get_id", &unigram::Vocabulary::get_id,
             py::arg("token"))
        .def("get_token", &unigram::Vocabulary::get_token,
             py::arg("id"))
        .def("get_score", &unigram::Vocabulary::get_score,
             py::arg("id"))
        .def("size", &unigram::Vocabulary::size)
        .def("contains", &unigram::Vocabulary::contains,
             py::arg("token"))
        .def("clear", &unigram::Vocabulary::clear)
        .def("load", &unigram::Vocabulary::load,
             py::arg("filepath"))
        .def("save", &unigram::Vocabulary::save,
             py::arg("filepath"))
        .def("__len__", &unigram::Vocabulary::size);

    // Tokenizer
    py::class_<unigram::Tokenizer>(m, "Tokenizer")
        .def(py::init<>())
        .def(py::init<const unigram::TokenizerConfig&>(),
             py::arg("config"))
        .def("set_vocabulary", &unigram::Tokenizer::set_vocabulary,
             py::arg("vocab"))
        .def("encode", &unigram::Tokenizer::encode,
             py::arg("text"))
        .def("encode_as_ids", &unigram::Tokenizer::encode_as_ids,
             py::arg("text"))
        .def("decode", py::overload_cast<const unigram::Tokens&>(&unigram::Tokenizer::decode, py::const_),
             py::arg("tokens"))
        .def("decode", py::overload_cast<const unigram::TokenIds&>(&unigram::Tokenizer::decode, py::const_),
             py::arg("token_ids"))
        .def("load", &unigram::Tokenizer::load,
             py::arg("filepath"))
        .def("save", &unigram::Tokenizer::save,
             py::arg("filepath"))
        .def("config", &unigram::Tokenizer::config)
        .def("vocabulary", &unigram::Tokenizer::vocabulary)
        .def_static("from_file", [](const std::string& filepath) {
            unigram::Tokenizer tokenizer;
            if (!tokenizer.load(filepath)) {
                throw std::runtime_error("Failed to load tokenizer from: " + filepath);
            }
            return tokenizer;
        }, py::arg("filepath"));

    // Trainer
    py::class_<unigram::Trainer>(m, "Trainer")
        .def(py::init<>())
        .def(py::init<const unigram::TokenizerConfig&>(),
             py::arg("config"))
        .def("train", &unigram::Trainer::train,
             py::arg("corpus"))
        .def("train_from_file", &unigram::Trainer::train_from_file,
             py::arg("filepath"))
        .def("set_progress_callback", &unigram::Trainer::set_progress_callback,
             py::arg("callback"))
        .def("config", &unigram::Trainer::config);

    // Constants
    m.attr("UNK_TOKEN") = unigram::UNK_TOKEN;
    m.attr("BOS_TOKEN") = unigram::BOS_TOKEN;
    m.attr("EOS_TOKEN") = unigram::EOS_TOKEN;
    m.attr("PAD_TOKEN") = unigram::PAD_TOKEN;
    m.attr("UNK_TOKEN_ID") = unigram::UNK_TOKEN_ID;
    m.attr("BOS_TOKEN_ID") = unigram::BOS_TOKEN_ID;
    m.attr("EOS_TOKEN_ID") = unigram::EOS_TOKEN_ID;
    m.attr("PAD_TOKEN_ID") = unigram::PAD_TOKEN_ID;

    // Version info
    m.attr("__version__") = "1.0.0";
}

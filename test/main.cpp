/**
 * @file main.cpp
 * @author karurochari
 * @brief Tester program for vs.templ
 * Use a single file containing data, template and expected to test if the library is doing a good job.
 * @copyright Copyright (c) 2024
 * 
 */

#include <cassert>
#include <iostream>
#include <pugixml.hpp>
#include <vs-templ.hpp>

using namespace vs::templ;

int main(int argc, const char** argv){
    assert(argc>1);
    pugi::xml_document doc;
    pugi::xml_parse_result ret = doc.load_file(argv[1]);
    std::cout << "Load result: " << ret.description()
              << ", mesh name: " << doc.child("mesh").attribute("name").value()
              << std::endl;

    auto data = doc.root().child("data");
    auto tmpl = doc.root().child("template");
    auto expects = doc.root().child("expects");
    int seed = doc.root().attribute("seed").as_int(0);

    preprocessor pdoc(data, tmpl, "s");
    auto &result = pdoc.parse();

    for (auto &log : pdoc.logs()) {
      if (log.type() == log_t::values::ERROR) {
        std::cerr << log.description() << "\n";
      }
    }
}
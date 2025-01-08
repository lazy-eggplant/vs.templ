/**
 * @file main.cpp
 * @author karurochari
 * @brief Tester program for vs.templ
 * Use a single file containing data, template and expected to test if the
 * library is doing a good job.
 * @copyright Copyright (c) 2024
 *
 */

#include "logging.hpp"
#include <cassert>
#include <iostream>
#include <pugixml.hpp>
#include <sstream>
#include <vs-templ.hpp>

using namespace vs::templ;

int main(int argc, const char **argv) {
  assert(argc > 1);
  pugi::xml_document doc;
  pugi::xml_parse_result ret = doc.load_file(argv[1]);

  if (ret.status != 0) {
    std::cerr << "Load result: " << ret.description() << ret.status
              << std::endl;
    return 1;
  }

  auto data = doc.child("test").child("data");
  auto tmpl = doc.child("test").child("template");
  auto expects = doc.child("test").child("expects").first_child();
  uint64_t seed = doc.child("test").attribute("seed").as_int(0);

  // tmpl.print(std::cout);
  // data.print(std::cout);
  // expects.print(std::cout);

  preprocessor pdoc(data, tmpl, "s:", +[](log_t::values,const char*,const logctx_t&){}, nullptr, nullptr, seed);
  auto &result = pdoc.parse();

  /*for (auto &log : pdoc.logs()) {
    if (log.type() == log_t::values::ERROR) {
      std::cerr << log.description() << "\n";
    }
  }*/
  // if (!pdoc.result){return 3;}

  std::stringstream serial_result;
  result.print(serial_result);
  std::stringstream serial_expects;

  expects.print(serial_expects);

  if (serial_result.str() == serial_expects.str()) {
    return 0;
  } else {
    std::cerr << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    result.print(std::cerr);
    std::cerr << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    expects.print(std::cerr);
    std::cerr << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    return 2;
  }
}

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0167 OLD)
project(TradingBotCore LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.70 REQUIRED COMPONENTS system thread)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(spdlog REQUIRED)

find_package(nlohmann_json REQUIRED)

find_package(CURL REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
  ${spdlog_INCLUDE_DIRS}
  ${nlohmann_json_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
)

add_subdirectory(extern/pybind11)

add_library(trading_core
  src/limit_order_book.cpp
  src/market_data_connector.cpp
  src/execution_engine.cpp
  src/risk_manager.cpp
  src/config.cpp
  src/telemetry.cpp
  src/symbol_worker.cpp
)

target_link_libraries(trading_core
  PRIVATE
    Boost::system
    Boost::thread
    ${OPENSSL_LIBRARIES}
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    ${CURL_LIBRARIES}
)

add_executable(trading_bot src/main.cpp)
target_link_libraries(trading_bot trading_core)

pybind11_add_module(trading_core_py bindings/python_bindings.cpp)
set_target_properties(trading_core_py PROPERTIES OUTPUT_NAME "trading_core")
target_link_libraries(trading_core_py PRIVATE trading_core)

enable_testing()

add_executable(test_lob    tests/test_limit_order_book.cpp)
add_executable(test_mdc    tests/test_market_data_connector.cpp)
add_executable(test_ee     tests/test_execution_engine.cpp)
add_executable(test_risk   tests/test_risk_manager.cpp)
add_executable(test_cfg    tests/test_config.cpp)
add_executable(test_tel    tests/test_telemetry.cpp)

foreach(tgt IN ITEMS test_lob test_mdc test_ee test_risk test_cfg test_tel)
  target_link_libraries(${tgt} PRIVATE trading_core)
endforeach()

add_test(NAME LOBTest        COMMAND test_lob)
add_test(NAME MDCTest        COMMAND test_mdc)
add_test(NAME EETest         COMMAND test_ee)
add_test(NAME RiskTest       COMMAND test_risk)
add_test(NAME ConfigTest     COMMAND test_cfg)
add_test(NAME TelemetryTest  COMMAND test_tel)
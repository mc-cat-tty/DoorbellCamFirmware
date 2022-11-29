#include "spinner.hpp"
#include <common.hpp>
#include <log/log.hpp>

using namespace app::animation;

constexpr static const auto mod = wrapper::log::Module::ANIMATION;

void SpinnerLogMock::animate() {
  static const auto logger = wrapper::log::Logger::getInstance();

  const size_t selectionCount = pow(2ULL, demux.getSelCount());
  logger.log(mod, ESP_LOG_DEBUG, "selCount: %d", selectionCount);
  
  if (currentSelection < selectionCount) {
    demux.select(currentSelection);
    logger.log(mod, ESP_LOG_DEBUG, "Selecting bit number: %d", currentSelection);
    currentSelection++;
    return;
  }

  running = false;
  logger.log(mod, ESP_LOG_DEBUG, "Stop running");
}
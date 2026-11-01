#pragma once

#include "bridge.h"
#include "webview.h"

#include <memory>
#include <string>

class App {
public:
  bool init();
  int run();

private:
  static std::string app_directory();
  static std::string to_file_uri(const std::string &path);

  std::unique_ptr<webview::webview> m_webview;
  Bridge m_bridge;
};

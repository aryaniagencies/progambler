#include "app.h"

#include <windows.h>

#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

std::string App::app_directory() {
  char path[MAX_PATH]{};
  const DWORD len = GetModuleFileNameA(nullptr, path, MAX_PATH);
  if (len == 0 || len == MAX_PATH) {
    return ".";
  }
  return fs::path(path).parent_path().string();
}

std::string App::to_file_uri(const std::string &path) {
  std::ostringstream uri;
  uri << "file:///";
  for (char ch : fs::path(path).lexically_normal().string()) {
    uri << (ch == '\\' ? '/' : ch);
  }
  return uri.str();
}

bool App::init() {
  m_webview = std::make_unique<webview::webview>(true, nullptr);
  if (!m_webview) {
    std::cerr << "Failed to create WebView2 window.\n";
    return false;
  }

  m_bridge.set_webview(m_webview.get());
  m_bridge.set_ui_updater([this](const std::string &id, const std::string &value) {
    m_bridge.send_to_frontend(id, value);
  });

  m_webview->set_title("ProGambler");
  m_webview->set_size(1280, 720, WEBVIEW_HINT_NONE);

  m_webview->bind("postToCpp", [this](const std::string &request) -> std::string {
    return m_bridge.handle_message(request);
  });

  const fs::path html_path = fs::path(app_directory()) / "main.html";
  if (!fs::exists(html_path)) {
    std::cerr << "main.html not found at " << html_path << '\n';
    return false;
  }

  m_webview->navigate(to_file_uri(html_path.string()));
  return true;
}

int App::run() {
  if (!m_webview) {
    return 1;
  }
  m_webview->run();
  return 0;
}

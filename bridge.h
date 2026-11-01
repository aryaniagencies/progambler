#pragma once

#include <functional>
#include <string>

namespace webview {
class webview;
}

// Routes JSON commands from the frontend and pushes UI updates back to JavaScript.
class Bridge {
public:
  using UiUpdater = std::function<void(const std::string &element_id,
                                       const std::string &value)>;

  void set_webview(webview::webview *webview);
  void set_ui_updater(UiUpdater updater);

  // Called when the frontend invokes window.postToCpp(requestJson).
  std::string handle_message(const std::string &request_json);

  // Push a value to a DOM element (dispatches the cpp-update event).
  void send_to_frontend(const std::string &element_id,
                        const std::string &value);

private:
  webview::webview *m_webview = nullptr;
  UiUpdater m_ui_updater;
};

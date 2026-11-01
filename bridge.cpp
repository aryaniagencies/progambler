#include "bridge.h"

#include "json.hpp"
#include "webview.h"

#include <stdexcept>

using json = nlohmann::json;

void Bridge::set_webview(webview::webview *webview) { m_webview = webview; }

void Bridge::set_ui_updater(UiUpdater updater) { m_ui_updater = std::move(updater); }

std::string Bridge::handle_message(const std::string &request_json) {
  json response;
  response["status"] = "error";

  try {
    const json request = json::parse(request_json);
    const std::string command =
        request.value("command", request.value("method", ""));

    if (command == "ping") {
      response["status"] = "ok";
      response["message"] = "pong";
      response["echo"] = request;
    } else if (command == "gamble") {
      response["status"] = "ok";
      response["image"] =
          "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIxMDAiIHZpZXdCb3g9IjAgMCAyNCAyNCIgc3R5bGU9ImZpbGw6IHJnYmEoMCwwLDAsMC4xNSk7IHRyYW5zZm9ybTogc2NhbGUoMC43NSkiPgogICAgICAgIDxwYXRoIGQ9Ik04LjUgMTMuNWwyLjUgMyAzLjUtNC41IDQuNSA2SDVtMTYgMVY1YTIgMiAwIDAgMC0yLTJINWMtMS4xIDAtMiAuOS0yIDJ2MTRjMCAxLjEuOSAyIDIgMmgxNGMxLjEgMCAyLS45IDItMnoiPjwvcGF0aD4KICAgICAgPC9zdmc+";

      if (m_ui_updater) {
        m_ui_updater("ik91", response["image"].get<std::string>());
      }
    } else if (command == "set_status") {
      const std::string id = request.value("id", "status");
      const std::string value = request.value("value", "");
      if (m_ui_updater) {
        m_ui_updater(id, value);
      }
      response["status"] = "ok";
    } else if (command == "log") {
      response["status"] = "ok";
      response["logged"] = request.value("message", "");
    } else {
      response["status"] = "error";
      response["message"] = "Unknown command: " + command;
    }
  } catch (const json::exception &ex) {
    response["status"] = "error";
    response["message"] = std::string("Invalid JSON: ") + ex.what();
  } catch (const std::exception &ex) {
    response["status"] = "error";
    response["message"] = ex.what();
  }

  return response.dump();
}

void Bridge::send_to_frontend(const std::string &element_id,
                              const std::string &value) {
  if (!m_webview) {
    return;
  }

  json detail;
  detail["id"] = element_id;
  detail["value"] = value;

  const std::string js =
      "window.dispatchEvent(new CustomEvent('cpp-update', {detail: " +
      detail.dump() + "}));";
  m_webview->eval(js);
}

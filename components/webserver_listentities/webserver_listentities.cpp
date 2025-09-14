#include "webserver_listentities.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/component_iterator.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/web_server_idf/web_server_idf.h"
#include <ArduinoJson.h>

// Per-entity includes guarded by USE_* like ESPHome does
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_LIGHT
#include "esphome/components/light/light_state.h"
#endif
#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_FAN
#include "esphome/components/fan/fan.h"
#endif
#ifdef USE_COVER
#include "esphome/components/cover/cover.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_LOCK
#include "esphome/components/lock/lock.h"
#endif
#ifdef USE_VALVE
#include "esphome/components/valve/valve.h"
#endif
#ifdef USE_MEDIA_PLAYER
#include "esphome/components/media_player/media_player.h"
#endif
#ifdef USE_ALARM_CONTROL_PANEL
#include "esphome/components/alarm_control_panel/alarm_control_panel.h"
#endif
#ifdef USE_EVENT
#include "esphome/components/event/event.h"
#endif
#ifdef USE_UPDATE
#include "esphome/components/update/update.h"
#endif
#ifdef USE_DATETIME_DATE
#include "esphome/components/datetime/date_entity.h"
#endif
#ifdef USE_DATETIME_TIME
#include "esphome/components/datetime/time_entity.h"
#endif
#ifdef USE_DATETIME_DATETIME
#include "esphome/components/datetime/datetime_entity.h"
#endif
#ifdef USE_TEXT
#include "esphome/components/text/text.h"
#endif

namespace esphome {
namespace webserver_listentities {

static const char *const TAG = "webserver_listentities";

// JSON-building iterator over all components
class ListEntitiesJsonIterator : public esphome::ComponentIterator {
 public:
  explicit ListEntitiesJsonIterator(ArduinoJson::JsonArray &out) : out_(out) {}

#ifdef USE_SENSOR
  bool on_sensor(sensor::Sensor *e) override { add_(e, "sensor"); return true; }
#endif
#ifdef USE_BINARY_SENSOR
  bool on_binary_sensor(binary_sensor::BinarySensor *e) override { add_(e, "binary_sensor"); return true; }
#endif
#ifdef USE_SWITCH
  bool on_switch(switch_::Switch *e) override { add_(e, "switch"); return true; }
#endif
#ifdef USE_NUMBER
  bool on_number(number::Number *e) override { add_(e, "number"); return true; }
#endif
#ifdef USE_LIGHT
  bool on_light(light::LightState *e) override { add_(e, "light"); return true; }
#endif
#ifdef USE_CLIMATE
  bool on_climate(climate::Climate *e) override { add_(e, "climate"); return true; }
#endif
#ifdef USE_TEXT_SENSOR
  bool on_text_sensor(text_sensor::TextSensor *e) override { add_(e, "text_sensor"); return true; }
#endif
#ifdef USE_FAN
  bool on_fan(fan::Fan *e) override { add_(e, "fan"); return true; }
#endif
#ifdef USE_COVER
  bool on_cover(cover::Cover *e) override { add_(e, "cover"); return true; }
#endif
#ifdef USE_SELECT
  bool on_select(select::Select *e) override { add_(e, "select"); return true; }
#endif
#ifdef USE_BUTTON
  bool on_button(button::Button *e) override { add_(e, "button"); return true; }
#endif
#ifdef USE_LOCK
  bool on_lock(lock::Lock *e) override { add_(e, "lock"); return true; }
#endif
#ifdef USE_VALVE
  bool on_valve(valve::Valve *e) override { add_(e, "valve"); return true; }
#endif
#ifdef USE_MEDIA_PLAYER
  bool on_media_player(media_player::MediaPlayer *e) override { add_(e, "media_player"); return true; }
#endif
#ifdef USE_ALARM_CONTROL_PANEL
  bool on_alarm_control_panel(alarm_control_panel::AlarmControlPanel *e) override { add_(e, "alarm_control_panel"); return true; }
#endif
#ifdef USE_EVENT
  bool on_event(event::Event *e) override { add_(e, "event"); return true; }
#endif
#ifdef USE_UPDATE
  bool on_update(update::UpdateEntity *e) override { add_(e, "update"); return true; }
#endif
#ifdef USE_DATETIME_DATE
  bool on_date(datetime::DateEntity *e) override { add_(e, "date"); return true; }
#endif
#ifdef USE_DATETIME_TIME
  bool on_time(datetime::TimeEntity *e) override { add_(e, "time"); return true; }
#endif
#ifdef USE_DATETIME_DATETIME
  bool on_datetime(datetime::DateTimeEntity *e) override { add_(e, "datetime"); return true; }
#endif
#ifdef USE_TEXT
  bool on_text(text::Text *e) override { add_(e, "text"); return true; }
#endif

  bool on_end() override { return true; }

 private:
  template <typename T>
  void add_(T *entity, const char *type) {
    auto obj = out_.add<ArduinoJson::JsonObject>();
    obj["type"] = type;
    obj["object_id"] = entity->get_object_id();
    obj["name"] = entity->get_name();
  }

  ArduinoJson::JsonArray &out_;
};

// ESP-IDF web server handler (snake_case virtuals; note const on is_request_handler)
class ListEntitiesHandler : public esphome::web_server_idf::AsyncWebHandler {
 public:
  bool canHandle(esphome::web_server_idf::AsyncWebServerRequest *request) const override {
    const auto url = request->url();
    const bool match = (url == "/entities" || url == "/entities/");
    ESP_LOGD(TAG, "can_handle url=%s match=%d", url.c_str(), match);
    return match;
  }

  void handleRequest(esphome::web_server_idf::AsyncWebServerRequest *request) override {
    ESP_LOGD(TAG, "handle_request /entities");

    ArduinoJson::JsonDocument doc;  // ArduinoJson v8
    auto root = doc.to<ArduinoJson::JsonObject>();
    auto arr = root["entities"].to<ArduinoJson::JsonArray>();

    // Iterate all known components/entities
    ListEntitiesJsonIterator it(arr);
    it.begin();
    //::esphome::iterate(&it);

    std::string json;
    ArduinoJson::serializeJson(doc, json);
    request->send(200, "application/json", json.c_str());
  }
};

float WebServerListEntities::get_setup_priority() const {
  // After WiFi; server attaches handlers when ready.
  return setup_priority::AFTER_WIFI;
}

void WebServerListEntities::setup() {
  ESP_LOGI(TAG, "Registering /entities endpoint (ESP-IDF)");
  auto *ws = esphome::web_server_base::global_web_server_base;
  if (!ws) {
    ESP_LOGE(TAG, "Web server not available; cannot register /entities");
    return;
  }
  ws->add_handler(new ListEntitiesHandler());
  ESP_LOGI(TAG, "Registered /entities endpoint");
}

void WebServerListEntities::dump_config() { ESP_LOGI(TAG, "Component loaded. Route: /entities"); }

}  // namespace webserver_listentities
}  // namespace esphome
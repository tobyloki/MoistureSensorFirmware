/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <app_priv.h>
#include <app_reset.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

#include "Arduino.h"
#include "SHTSensor.h"
#include "LPS.h"
#include "I2CSoilMoistureSensor.h"
#include "SparkFun_VEML7700_Arduino_Library.h"
#include "Wire.h"
#include "esp32-hal.h"
#include "esp_matter_attribute_utils.h"

SHTSensor sht(SHTSensor::SHT3X);
LPS ps;
I2CSoilMoistureSensor soil_sensor;
VEML7700 ls;

static const char *TAG = "app_main";
uint16_t temperature_endpoint_id;
uint16_t pressure_endpoint_id;
uint16_t humidity_endpoint_id;

// begin timer stuff
typedef struct
{
    esp_timer_handle_t oneshot_timer;
    void (*cb)();
} _timer;
void startTimer(uint64_t time_us, _timer *timer);
void cancelTimer(_timer *timer);
void measureCb(void);
_timer measureTimer = {
    .oneshot_timer = NULL,
    .cb = measureCb
};
// end timer stuff

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        {
            ESP_LOGI(TAG, "Fabric removed successfully");
            if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
            {
                chip::CommissioningWindowManager & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
                constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
                if (!commissionMgr.IsCommissioningWindowOpen())
                {
                    /* After removing last fabric, this example does not remove the Wi-Fi credentials
                     * and still has IP connectivity so, only advertising on DNS-SD.
                     */
                    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                    chip::CommissioningWindowAdvertisement::kDnssdOnly);
                    if (err != CHIP_NO_ERROR)
                    {
                        ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                    }
                }
            }
        break;
        }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;
    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    if (type == PRE_UPDATE) {
        /* Driver update */
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }

    return err;
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    /* Initialize the ESP NVS layer */
    nvs_flash_init();

    /* Initialize driver */
    app_driver_handle_t temperature_handle = app_driver_temperature_init();
    app_driver_handle_t pressure_handle = app_driver_pressure_init();
    app_driver_handle_t humidity_handle = app_driver_humidity_init();
    app_driver_handle_t button_handle = app_driver_button_init();
    app_reset_button_register(button_handle);

    /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);

    // color_temperature_light::config_t light_config;
    // light_config.on_off.on_off = DEFAULT_POWER;
    // light_config.on_off.lighting.start_up_on_off = nullptr;
    // light_config.level_control.current_level = DEFAULT_BRIGHTNESS;
    // light_config.level_control.lighting.start_up_current_level = DEFAULT_BRIGHTNESS;
    // light_config.color_control.color_mode = EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE;
    // light_config.color_control.enhanced_color_mode = EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE;
    // light_config.color_control.color_temperature.startup_color_temperature_mireds = nullptr;
    // endpoint_t *endpoint = color_temperature_light::create(node, &light_config, ENDPOINT_FLAG_NONE, light_handle);

    // endpoint (temperature device type)
    temperature_sensor::config_t temperature_config;
    nullable<int16_t> temperature = DEFAULT_TEMPERATURE;
    temperature_config.temperature_measurement.measured_value = temperature;
    endpoint_t *temperature_endpoint = temperature_sensor::create(node, &temperature_config, ENDPOINT_FLAG_NONE, temperature_handle);
    // endpoint (pressure device type)
    pressure_sensor::config_t pressure_config;
    nullable<int16_t> pressure = DEFAULT_PRESSURE;
    pressure_config.pressure_measurement.pressure_measured_value = pressure;
    endpoint_t *pressure_endpoint = pressure_sensor::create(node, &pressure_config, ENDPOINT_FLAG_NONE, pressure_handle);
    // endpoint (humidity device type)
    humidity_sensor::config_t humidity_config;
    nullable<uint16_t> humidity = DEFAULT_HUMIDITY;
    humidity_config.humidity_measurement.measured_value = humidity;
    nullable<uint16_t> deviceId = DEfAULT_THING_NAME;
    humidity_config.humidity_measurement.min_measured_value = deviceId; // will be using this as the device id
    nullable<uint16_t> battery = DEFAULT_BATTERY;
    humidity_config.humidity_measurement.max_measured_value = battery; // will be usings this as the battery percentage
    endpoint_t *humidity_endpoint = humidity_sensor::create(node, &humidity_config, ENDPOINT_FLAG_NONE, humidity_handle);

    /* These node and endpoint handles can be used to create/add other endpoints and clusters. */
    if (!node || !temperature_endpoint || !pressure_endpoint || !humidity_endpoint) {
        ESP_LOGE(TAG, "Matter node creation failed");
    }

    // light_endpoint_id = endpoint::get_id(endpoint);
    // ESP_LOGI(TAG, "Light created with endpoint_id %d", light_endpoint_id);
    temperature_endpoint_id = endpoint::get_id(temperature_endpoint);
    pressure_endpoint_id = endpoint::get_id(pressure_endpoint);
    humidity_endpoint_id = endpoint::get_id(humidity_endpoint);
    ESP_LOGI(TAG, "Temperature created with endpoint_id %d", temperature_endpoint_id);
    ESP_LOGI(TAG, "Pressure created with endpoint_id %d", pressure_endpoint_id);
    ESP_LOGI(TAG, "Humidity created with endpoint_id %d", humidity_endpoint_id);

    /* Add additional features to the node */
    // cluster_t *cluster = cluster::get(endpoint, ColorControl::Id);
    // cluster::color_control::feature::hue_saturation::config_t hue_saturation_config;
    // hue_saturation_config.current_hue = DEFAULT_HUE;
    // hue_saturation_config.current_saturation = DEFAULT_SATURATION;
    // cluster::color_control::feature::hue_saturation::add(cluster, &hue_saturation_config);

    /* Matter start */
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Matter start failed: %d", err);
    }

    /* Starting driver with default values */
    // app_driver_light_set_defaults(light_endpoint_id);
    app_driver_temperature_set_defaults(temperature_endpoint_id);
    app_driver_pressure_set_defaults(pressure_endpoint_id);
    app_driver_humidity_set_defaults(humidity_endpoint_id);

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::init();
#endif

    initArduino();
    Wire.begin();
    ps.init();
    ps.enableDefault();
    soil_sensor.begin();
    ls.begin();
    startTimer(5 * 1000 * 1000, &measureTimer);
}

void measureCb() {
    ESP_LOGI(TAG, "Measure callback");

    // NOTE: code goes here to measure stuff and report to matter
    sht.init();
    float sht_temp = sht.getTemperature();
    float sht_humidity = sht.getHumidity();
    ESP_LOGI("SHT", "temp = %0.1f C, humidity = %0.1f \%", sht_temp, sht_humidity);
    
    float pressure = ps.readPressureInchesHg();
    ESP_LOGI("LPS", "pressure = %f inchesHg", pressure);

    uint16_t soil_capacitance = soil_sensor.getCapacitance();
    ESP_LOGI("SOIL", "capacitance = %d", soil_capacitance);

    float lux = ls.getLux();
    ESP_LOGI("VEML", "light = %0.1f lux", lux);

    // temp
    uint16_t endpoint_id = temperature_endpoint_id;
    uint32_t cluster_id = TemperatureMeasurement::Id;
    uint32_t attribute_id = TemperatureMeasurement::Attributes::MeasuredValue::Id;

    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = cluster::get(endpoint, cluster_id);
    attribute_t *attribute = attribute::get(cluster, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.i16 = sht_temp;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);

    // humidity
    endpoint_id = humidity_endpoint_id;
    cluster_id = RelativeHumidityMeasurement::Id;
    attribute_id = RelativeHumidityMeasurement::Attributes::MeasuredValue::Id;

    node = node::get();
    endpoint = endpoint::get(node, endpoint_id);
    cluster = cluster::get(endpoint, cluster_id);
    attribute = attribute::get(cluster, attribute_id);

    val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.u16 = sht_humidity;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);

    // pressure
    endpoint_id = pressure_endpoint_id;
    cluster_id = PressureMeasurement::Id;
    attribute_id = PressureMeasurement::Attributes::MeasuredValue::Id;

    node = node::get();
    endpoint = endpoint::get(node, endpoint_id);
    cluster = cluster::get(endpoint, cluster_id);
    attribute = attribute::get(cluster, attribute_id);

    val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.u16 = pressure;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);

    startTimer(5 * 1000 * 1000, &measureTimer);
}

void timerCb(void *arg)
{
    ESP_LOGI(TAG, "Timer callback");
    ESP_ERROR_CHECK(esp_timer_delete(((_timer *)arg)->oneshot_timer));
    ((_timer *)arg)->oneshot_timer = NULL;

    if (((_timer *)arg)->cb != NULL)
        ((_timer *)arg)->cb();
}

void cancelTimer(_timer *timer)
{
    if (timer->oneshot_timer != NULL)
    {
        ESP_ERROR_CHECK(esp_timer_stop(timer->oneshot_timer));
        ESP_ERROR_CHECK(esp_timer_delete(timer->oneshot_timer));
        timer->oneshot_timer = NULL;
        ESP_LOGI(TAG, "Timer canceled");
    }
    else
    {
        // ESP_LOGI(TAG, "Timer already canceled");
    }
}

void startTimer(uint64_t time_us, _timer *timer)
{
    if (timer->oneshot_timer != NULL)
        cancelTimer(timer);

    const esp_timer_create_args_t oneshot_timer_args = {
        .callback = &timerCb,
        .arg = (void *)timer};
    // Create timer
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &(timer->oneshot_timer)));
    // Start timer
    ESP_ERROR_CHECK(esp_timer_start_once(timer->oneshot_timer, time_us));
    ESP_LOGI(TAG, "Started timer for: %" PRIu64 " us", time_us);
}
